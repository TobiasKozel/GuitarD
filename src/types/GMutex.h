#pragma once

#include <mutex>

#include "./GTypes.h"

namespace guitard {
	class Mutex {
		std::mutex mMutex;
	public:
		Mutex() {}

		void lock() {
			mMutex.lock();
		}

		void unlock() {
			mMutex.unlock();
		}

		GUITARD_NO_COPY(Mutex)
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

		GUITARD_NO_COPY(LockGuard)
	};
}
