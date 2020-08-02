#ifndef MULTIPLE_THRADS_SIMPLE_THREADPOOL_H_
#define MULTIPLE_THRADS_SIMPLE_THREADPOOL_H_
#include "multi_threads/threadpool_interface.h"

#include <vector>
#include <atomic>
#include <thread>
#include <queue>
#include <condition_variable>
#include <mutex>

namespace thread{
    class SimpleThread:public ThreadPoolInterface{
        public:
            SimpleThread(size_t num_threads);
            virtual ~SimpleThread();


            virtual void Schedule(std::function<void()> fn)override;

            // Returns the number of threads in the pool.
            virtual int NumThreads() const override{
                return num_threads_;
            }

            // Returns a logical thread index between 0 and NumThreads() - 1 if called
            // from one of the threads in the pool. Returns -1 otherwise.
            virtual int CurrentThreadId() const override;

        private:
            struct Thread{
                std::unique_ptr<std::thread> thread;
            };

            struct PerThread {
                constexpr PerThread() : pool(NULL), rand(0), thread_id(-1) {}
                SimpleThread* pool;  // Parent pool, or null for normal threads.
                uint64_t rand;          // Random generator state.
                int thread_id;          // Worker thread index in pool.

                // Prevent false sharing.
                // char pad_[128];
            };

            void WorkerLoop(int thread_id) ;
            // Thread* CreateThread(std::function<void()>);

            inline PerThread* GetPerThread(){
                thread_local PerThread per_thread_;
                PerThread* pt = &per_thread_;
                return pt;
            }

            const int num_threads_;
            std::vector<Thread> workers_;
            std::queue<std::function<void()>> tasks_;
            std::atomic<unsigned> blocked_;
            std::atomic<bool> spinning_;
            std::atomic<bool> done_;
            std::atomic<bool> cancelled_;

            // synchronization
            std::mutex queue_mutex_;
            std::condition_variable condition_;
    };
}



#endif
