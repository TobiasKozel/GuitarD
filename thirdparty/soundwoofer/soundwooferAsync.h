#pragma once
#include <mutex>
#include <functional>
#include <thread>
#include <memory>
#include "./soundwooferTypes.h"

namespace soundwoofer {
  /**
   * Allows async operations wrapping the SW singleton
   */
  namespace async {
    void cancelAll(const bool doJoin = false);

    typedef std::function<void(Status)> CallbackFunc;

    /**
     * Callback for async operations which provides a status code
     */
    typedef std::shared_ptr<std::function<void(Status)>> Callback;

    /**
     * There's no reason to touch anything in here
     */
    namespace _ {
      int maxQueueLength =
#ifndef SOUNDWOOFER_MAX_ASYNC_QUEUE
        3;
#else
        SOUNDWOOFER_MAX_ASYNC_QUEUE;
#endif

      /**
       * A function to generalize most tasks used in here
       * Will provide a status code
       */
      typedef std::function<Status()> Task;

      /**
       * Bundles together a task and a callback to call after a task has been finished
       */
      struct TaskBundle {
        Callback callback;
        Task task;
      };

      typedef std::shared_ptr<TaskBundle> TaskBundleShared;

      /**
       * Tasks will be queued up here and processed from back to front by mThread
       */
      std::vector<TaskBundleShared> mQueue;

      /**
       * This will mutex the mQueue
       */
      std::mutex mutex;
      std::thread thread;
      bool threadRunning = false;

      struct LifeCycleHook {
        static LifeCycleHook& instance() {
          static LifeCycleHook _instance; return _instance;
        }
        ~LifeCycleHook() { cancelAll(true); }
      };

      /**
       * This will add a TaskBundle to the queue and set off
       * the thread to work on the queue if it's not already running
       */
      void startAsync(Task task, Callback callback) {
        if (maxQueueLength < _::mQueue.size()) {
          cancelAll(true);
        }
        LifeCycleHook::instance(); // Create the stack singleton to clean up on program exit
        mutex.lock();
        mQueue.push_back(std::make_shared<TaskBundle>(TaskBundle {
          callback, task
        }));
        if (!threadRunning) {
          if (thread.joinable()) {
            thread.join();
          }
          threadRunning = true;
          mutex.unlock();
          thread = std::thread([&]() {
            while (threadRunning) {
              mutex.lock(); // Mutex to make sure the queue doesn't get corrupted
              if (mQueue.empty()) { // Could be empty by now
                mutex.unlock();
                continue;
              }
              TaskBundleShared t = *mQueue.begin(); // A copy to ensure the object keeps on living
              mQueue.erase(mQueue.begin());
              mutex.unlock();
              const Status status = t->task(); // Do the main task
              if (threadRunning) { // We might want to terminate here if the whole queue was cleared
                const int count = t->callback.use_count();
                if (1 < count) { // More than one owner (this one) means it's still valid
                  (*t->callback)(status); // Do the callback if it's still valid
                }
                mutex.lock();
                threadRunning = !mQueue.empty();
                mutex.unlock();
              }
            }
          });
        }
        else {
          mutex.unlock();
        }
      }
    }

    namespace ir {
      Status list(const Callback callback) {
        _::startAsync([&]() {
          return soundwoofer::ir::list();
        }, callback);
        return ASYNC;
      }

      Status load(SWImpulseShared ir, Callback callback, size_t sampleRate = 0, bool normalize = true) {
        _::startAsync([&, ir, sampleRate, normalize]() {
          return soundwoofer::ir::load(ir, sampleRate, normalize);
        }, callback);
        return ASYNC;
      }

      Status loadUnknown(SWImpulseShared* ir, Callback callback, size_t sampleRate = 0, bool normalize = true) {
        _::startAsync([&, ir, sampleRate, normalize]() {
          return soundwoofer::ir::loadUnknown(ir, sampleRate, normalize);
        }, callback);
        return ASYNC;
      }
    }

    namespace preset {
      Status list(const Callback callback) {
        _::startAsync([&]() {
          return soundwoofer::preset::list();
        }, callback);
        return ASYNC;
      }

#ifndef SOUNDWOOFER_NO_API
      Status send(const SWPreset preset, Callback callback) {
        _::startAsync([&, preset]() {
          return soundwoofer::preset::send(preset);
        }, callback);
        return ASYNC;
      }
#endif

      Status load(SWPresetsShared preset, Callback callback) {
        _::startAsync([&, preset]() {
          return soundwoofer::preset::load(preset);
        }, callback);
        return ASYNC;
      }
    }



    /**
     * Clears the async queue, but doesn't terminate a running task
     * Call this if for example the UI gets destroyed to be sure there are no callbacks
     * lingering which might be attached to destroyed UI elements
     */
    void cancelAll(const bool doJoin) {
      _::mutex.lock();
      _::threadRunning = false;
      _::mQueue.clear();

      if (_::thread.joinable() && doJoin) {
        _::thread.join();
      }
      _::mutex.unlock();
    }
  }
}
