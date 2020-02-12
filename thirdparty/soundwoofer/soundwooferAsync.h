#pragma once
#include <mutex>
#include <functional>
#include <thread>
#include "./soundwooferTypes.h"

namespace soundwoofer {
  /**
   * Allows async operations wrapping the SW singleton
   */
  namespace async {
    namespace _ {
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
        void* parent = nullptr; // This can be used by plugin instances for identification
      };

      /**
       * Tasks will be queued up here and processed from back to front by mThread
       */
      std::vector<TaskBundle> mQueue;

      /**
       * This will mutex the mQueue
       */
      std::mutex mMutex;
      std::thread mThread;
      bool mThreadRunning = false;

      /**
       * This will add a TaskBundle to the queue and set off
       * the thread to work on the queue if it's not already running
       */
      void startAsync(Task task, Callback callback, void* invocedBy = nullptr) {
        mMutex.lock();
        mQueue.push_back({
          callback, task, invocedBy
        });
        if (!mThreadRunning) {
          if (mThread.joinable()) {
            mThread.join();
          }
          mThreadRunning = true;
          mMutex.unlock();
          mThread = std::thread([&]() {
            while (mThreadRunning) {
              mMutex.lock(); // Mutex to make sure the queue doesn't get corrupted
              TaskBundle t = *mQueue.begin();
              mQueue.erase(mQueue.begin());
              mMutex.unlock();
              const Status status = t.task();
              if (mThreadRunning) { // We might want to terminate here
                t.callback(status);
                mMutex.lock();
                mThreadRunning = !mQueue.empty();
                mMutex.unlock();
              }
            }
          });
        }
        else {
          mMutex.unlock();
        }
      }
    }

    Status listIRs(const Callback callback, void* invocedBy = nullptr) {
      _::startAsync([&]() {
        return instance().listIRs();
      }, callback, invocedBy);
      return ASYNC;
    }

    Status listPresets(const Callback callback, void* invocedBy = nullptr) {
      _::startAsync([&]() {
        return instance().listPresets();
      }, callback, invocedBy);
      return ASYNC;
    }

    Status sendPreset(const std::string name, const char* data, const size_t length, Callback callback, void* invocedBy = nullptr) {
      _::startAsync([&, name, data, length]() {
        return instance().sendPreset(name, data, length);
      }, callback, invocedBy);
      return ASYNC;
    }

    Status loadIR(const SWImpulseShared ir, Callback callback, void* invocedBy = nullptr) {
      _::startAsync([&, ir]() {
        return instance().loadIR(ir);
      }, callback, invocedBy);
      return ASYNC;
    }

    Status loadIR(std::string fileId, Callback callback, void* invocedBy = nullptr) {
      //for (auto& i : mIRlist) {
      //  if (fileId == i->file) {
      //    return loadIR(i, callback);
      //  }
      //}
      return GENERIC_ERROR;
    }

    Status loadPreset(SWPresetsShared preset, Callback callback, void* invocedBy = nullptr) {
      _::startAsync([&, preset]() {
        return instance().loadPreset(preset);
      }, callback, invocedBy);
      return ASYNC;
    }

    /**
     * Clears the async queue, but doesn't terminate a running task
     * Call this if for example the UI gets destroyed to be sure there are no callbacks
     * lingering which might be attached to destroyed UI elements
     * @param invocedBy if nullptr will clear the whole queue
     */
    void clearAsyncQueue(void* invocedBy = nullptr, const bool doJoin = false) {
      _::mMutex.lock();
      if (invocedBy == nullptr) {
        _::mThreadRunning = false;
        _::mQueue.clear();
      }
      else {
        auto it = _::mQueue.begin();
        while (it != _::mQueue.end()) {
          if (it->parent == invocedBy) {
            it = _::mQueue.erase(it);
          }
          else ++it;
        }
      }
      if (_::mThread.joinable() && doJoin) {
        _::mThread.join();
      }
      _::mMutex.unlock();
    }
  }
}
