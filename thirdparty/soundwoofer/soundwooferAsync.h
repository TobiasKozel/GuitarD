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
		/**
		 * Clears the async queue, but doesn't terminate a running task
		 * Call this if for example the UI gets destroyed to be sure there are no callbacks
		 * lingering which might be attached to destroyed UI elements
		 */
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
			void startAsync(Task task, Callback callback);
		}

		namespace ir {
			Status list(const Callback callback);

			Status load(SWImpulseShared ir, Callback callback, size_t sampleRate = 0, bool normalize = true);

			Status loadUnknown(SWImpulseShared* ir, Callback callback, size_t sampleRate = 0, bool normalize = true);
		}

		namespace preset {
			Status list(const Callback callback);

#ifndef SOUNDWOOFER_NO_API
			Status send(const SWPresetShared preset, Callback callback);
#endif

			Status load(SWPresetShared preset, Callback callback);
		}
	}
}


#include "./soundwooferAsyncImpl.h"