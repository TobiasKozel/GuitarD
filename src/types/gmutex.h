#pragma once

#ifdef GUITARD_HEADLESS
#include <mutex>
#else
#include "mutex.h"
#endif

namespace guitard {
  class Mutex {
#ifdef GUITARD_HEADLESS
    std::mutex mMutex;
#else
    WDL_Mutex mMutex;
#endif
  public:
    Mutex() {}

    void lock() {
#ifdef GUITARD_HEADLESS
      mMutex.lock();
#else
      mMutex.Enter();
#endif
    }

    void unlock() {
#ifdef GUITARD_HEADLESS
      mMutex.unlock();
#else
      mMutex.Leave();
#endif
    }

    Mutex(Mutex&) = delete;
    Mutex(Mutex*) = delete;
    Mutex& operator=(const Mutex&) = delete;
  };

  class LockGuard {
    Mutex* mMutex = nullptr;
  public:
    LockGuard(Mutex& mutex) {
      mMutex = &mutex;
      mutex.lock();
    }

    ~LockGuard() {
      mMutex->unlock();
    }

    LockGuard(LockGuard&) = delete;
    LockGuard(LockGuard*) = delete;
    LockGuard& operator=(const LockGuard&) = delete;
  };
}