#include <iostream>
#include <chrono>
#include <glog/logging.h>
#include "multi_threads/simple_threadpool.h"


using thread::SimpleThread;
using thread::ThreadPoolInterface;

int main(int argc, char* argv[]){
    google::InitGoogleLogging(argv[0]);
    auto threadpool = std::unique_ptr<ThreadPoolInterface>(new SimpleThread(2));
    const int num_tasks=3;
    for(int i=0;i<num_tasks;++i){
        threadpool->Schedule([threadpool_ptr=threadpool.get()](){
                LOG(INFO)<<"Thread id: "
                <<threadpool_ptr->CurrentThreadId();
                });
    }

    int a=0;
    std::condition_variable cond;
    std::mutex mutex_a;

    threadpool->Schedule([&](){a=10*10; cond.notify_all();});
    {
        std::unique_lock<std::mutex> lock(mutex_a);
        cond.wait(lock);
    }
    LOG(INFO)<<"result 10 * 10="<<a;
    return 0;
}
