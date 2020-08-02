#ifndef MULTIPLE_THRADS_THREADPOOL_H_
#define MULTIPLE_THRADS_THREADPOOL_H_
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <unordered_map>

#include "threadpool_interface.h"

namespace thread{
    class StdThread{
        public:
            // thread_options is ignored.
            StdThread(std::function<void()> fn)
                : thread_(fn) {}

            ~StdThread(){
                // std::thread::id thread_id = thread_.get_id();
                thread_.join();
            }
        private:
            std::thread thread_;
    };

    struct Task{};
    class Environment{
        public:
            Environment();

            StdThread* CreateThread(std::function<void()> f){
                return new StdThread(f);
            }

            Task CreateTask(std::function<void()> f) {
                return Task();
            }
    };

    template <typename Environment>
        class ThreadPoolTempl:public ThreadPoolInterface{
            public:
                typedef std::queue<Task> Queue;
                ThreadPoolTempl(int num_threads, Environment env = Environment())
                    : ThreadPoolTempl(num_threads, true, env) {}

                ThreadPoolTempl(int num_threads, bool allow_spinning,
                        Environment env = Environment())
                    : env_(env),
                    num_threads_(num_threads),
                    allow_spinning_(allow_spinning),
                    thread_data_(num_threads),
                    // waiters_(num_threads),
                    blocked_(0),
                    spinning_(0),
                    done_(false),
                    cancelled_(false){
                        for (int i = 0; i < num_threads_; i++) {
                            thread_data_[i].thread.reset(
                                    env_.CreateThread([this, i]() { WorkerLoop(i); }));
                        }
                    }

                ~ThreadPoolTempl() {
                    done_ = true;

                    // Now if all threads block without work, they will start exiting.
                    // But note that threads can continue to work arbitrary long,
                    // block, submit new work, unblock and otherwise live full life.
                    if (!cancelled_) {
                        // ec_.Notify(true);
                    } else {
                        // Since we were cancelled, there might be entries in the queues.
                        // Empty them to prevent their destructor from asserting.
                        for (size_t i = 0; i < thread_data_.size(); i++) {
                            thread_data_[i].queue.Flush();
                        }
                    }
                    // Join threads explicitly (by destroying) to avoid destruction order within
                    // this class.
                    for (size_t i = 0; i < thread_data_.size(); ++i)
                        thread_data_[i].thread.reset();
                }

                void Schedule(std::function<void()> fn) override {
                    ScheduleWithHint(std::move(fn), 0, num_threads_);
                }

                void Cancel() override {
                    cancelled_ = true;
                    done_ = true;

                    // Let each thread know it's been cancelled.
                    for (size_t i = 0; i < thread_data_.size(); i++) {
                        thread_data_[i].thread->OnCancel();
                    }

                    // Wake up the threads without work to let them exit on their own.
                    // ec_.Notify(true);
                }

                int NumThreads() const final { return num_threads_; }

                int CurrentThreadId() const final {
                    const PerThread* pt = const_cast<ThreadPoolTempl*>(this)->GetPerThread();
                    if (pt->pool == this) {
                        return pt->thread_id;
                    } else {
                        return -1;
                    }
                }

            private:

                struct PerThread {
                    constexpr PerThread() : pool(NULL), rand(0), thread_id(-1) {}
                    ThreadPoolTempl* pool;  // Parent pool, or null for normal threads.
                    uint64_t rand;          // Random generator state.
                    int thread_id;          // Worker thread index in pool.

                    // Prevent false sharing.
                    char pad_[128];
                };

                struct ThreadData {
                    constexpr ThreadData() : thread(), steal_partition(0), queue() {}
                    std::unique_ptr<StdThread> thread;
                    std::atomic<unsigned> steal_partition;
                    Queue queue;
                };

                Environment env_;
                const int num_threads_;
                const bool allow_spinning_;
                std::vector<ThreadData> thread_data_;
                // std::vector<MaxSizeVector<unsigned>> all_coprimes_;
                // std::vector<EventCount::Waiter> waiters_;
                unsigned global_steal_partition_;
                std::atomic<unsigned> blocked_;
                std::atomic<bool> spinning_;
                std::atomic<bool> done_;
                std::atomic<bool> cancelled_;
                // EventCount ec_;
                // std::unique_ptr<Barrier> init_barrier_;
                std::mutex per_thread_map_mutex_;  // Protects per_thread_map_.
                std::unordered_map<uint64_t, std::unique_ptr<PerThread>> per_thread_map_;

                // Main worker thread loop.
                void WorkerLoop(int thread_id) {
                    std::unique_ptr<PerThread> new_pt(new PerThread());
                    per_thread_map_mutex_.lock();
                    per_thread_map_.emplace(GlobalThreadIdHash(), std::move(new_pt)).second;
                    per_thread_map_mutex_.unlock();

                    // init_barrier_->Notify();
                    // init_barrier_->Wait();

                    PerThread* pt = GetPerThread();
                    pt->pool = this;
                    pt->rand = GlobalThreadIdHash();
                    pt->thread_id = thread_id;
                    Queue& q = thread_data_[thread_id].queue;
                    // EventCount::Waiter* waiter = &waiters_[thread_id];
                }

                static inline uint64_t GlobalThreadIdHash() {
                    return std::hash<std::thread::id>()(std::this_thread::get_id());
                }

                inline PerThread* GetPerThread() {
                    static PerThread dummy;
                    auto it = per_thread_map_.find(GlobalThreadIdHash());
                    if (it == per_thread_map_.end()) {
                        return &dummy;
                    } else {
                        return it->second.get();
                    }
                }

                // WaitForWork blocks until new work is available (returns true), or if it is
                // time to exit (returns false). Can optionally return a task to execute in t
                // (in such case t.f != nullptr on return).
                bool WaitForWork(Task* t) {
                }
        };

} // namespace thread


#endif
