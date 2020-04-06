#pragma once

#include "./soundwooferAsync.h"
#include "./soundwoofer.h"

namespace soundwoofer {
  namespace async {
    void cancelAll(const bool doJoin) {
      _::mutex.lock();
      _::threadRunning = false;
      _::mQueue.clear();

      if (_::thread.joinable() && doJoin) {
        _::thread.join();
      }
      _::mutex.unlock();
    }

    namespace _ {
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

      Status load(SWImpulseShared ir, Callback callback, size_t sampleRate, bool normalize) {
        _::startAsync([&, ir, sampleRate, normalize]() {
          return soundwoofer::ir::load(ir, sampleRate, normalize);
        }, callback);
        return ASYNC;
      }

      Status loadUnknown(SWImpulseShared* ir, Callback callback, size_t sampleRate, bool normalize) {
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
      Status send(const SWPresetShared preset, Callback callback) {
        _::startAsync([&, preset]() {
          return soundwoofer::preset::send(preset);
        }, callback);
        return ASYNC;
      }
#endif

      Status load(SWPresetShared preset, Callback callback) {
        _::startAsync([&, preset]() {
          return soundwoofer::preset::load(preset);
        }, callback);
        return ASYNC;
      }
    }
  }
}