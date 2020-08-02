#include "multi_threads/simple_threadpool.h"


namespace thread{
    SimpleThread::SimpleThread(size_t num_threads)
        :num_threads_(num_threads),
        blocked_(0),
        done_(false),
        workers_(num_threads){
            // init threads
            for (int i = 0; i < num_threads_; i++) {
                workers_[i].thread.reset(new std::thread(
                            [this, i](){WorkerLoop(i);}));
            }
        }

    void SimpleThread::WorkerLoop(int thread_id) {
        // init per thread first
        PerThread* pt = GetPerThread();
        pt->pool = this;
        pt->thread_id = thread_id;

        while(true){
            // infinity loop
            std::function<void()> task;
            {
                // fetch task from queue
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this]{return done_ ||!tasks_.empty();});
                if(done_&&tasks_.empty()){
                    return;
                }
                task = std::move(tasks_.front());
                tasks_.pop();
            }

            // execute task
            task();
        }
    }

    SimpleThread::~SimpleThread(){
        done_ = true;

        condition_.notify_all();
        for(auto &worker: workers_)
            worker.thread->join();
    }

    void SimpleThread::Schedule(std::function<void()> fn){
        {
            // submit task to queue
            std::unique_lock<std::mutex> lock(queue_mutex_);
            tasks_.push(std::move(fn));
        }
        condition_.notify_one();
    }

    int SimpleThread::CurrentThreadId() const {
        const PerThread* pt = const_cast<SimpleThread*>(this)->GetPerThread();
        if (pt->pool == this) {
            return pt->thread_id;
        } else {
            return -1;
        }
    }

}//namespace threa
