#pragma once
#include <functional>
#include "./EventList.h"
#include "../../types/GPointerList.h"
#include "../../types/GMutex.h"
namespace guitard {
  /**
   * This allows for easy communication between classes which don't know each other
   * A list of events can be found in EventList.h
   */
  namespace MessageBus {
    /**
     * Base class to take advantage of polymorphism
     */
    class BaseSubscription {
    public:
      virtual ~BaseSubscription() { };
    protected:
      bool subscribed = false;
      MESSAGE_ID mEventId = TOTAL_MESSAGE_IDS;
    };

    typedef PointerList<BaseSubscription> SubsVector;

    /**
     * The bus object knows about all the subscribers and relays the events
     * There's only one per Plugin instance
     */
    struct Bus {
      SubsVector mSubscriptions[TOTAL_MESSAGE_IDS];
      Mutex mMutex;
      int mSubCount = 0;
      /**
       * If the destructor is called, the plugin was probably destroyed
       * The bus is not responsible for the lifetimes of it's subscriptions
       * so just clean out all the references and call it a day
       */
      ~Bus() {
        for (int i = 0; i < TOTAL_MESSAGE_IDS; i++) {
          if (mSubscriptions[i].size() > 0) {
            mSubscriptions[i].clear();
          }
        }
        mSubCount = 0;
      }

      void addSubscriber(BaseSubscription* sub, const MESSAGE_ID pEventId) {
        LockGuard lock(mMutex);
        mSubscriptions[pEventId].add(sub);
        mSubCount++;
        if (mSubCount > 1000) {
          // This probably means there's a leak
          WDBGMSG("Subcount %i\n", mSubCount);
        }
      }

      void removeSubscriber(BaseSubscription* sub, const MESSAGE_ID pEventId) {
        if (mSubCount > 0) {
          LockGuard lock(mMutex);
          mSubscriptions[pEventId].remove(sub);
          mSubCount--;
        }
      }
    };

    // Actual implementation based on the data type to pass over the Bus
    template <class T>
    class Subscription : public BaseSubscription {
      // The bus it is subscribed to
      Bus* mBus = nullptr;
    public:
      std::function<void(T param)> mCallback;

      Subscription() { }

      Subscription(Bus* pBus, const MESSAGE_ID pEventId, std::function<void(T param)> callback) {
        subscribe(pBus, pEventId, callback);
      }

      ~Subscription() {
        if (mBus == nullptr || mEventId >= TOTAL_MESSAGE_IDS || mEventId < 0) {
          // This shouldn't happen
          return;
          assert(false);
        }
        mBus->removeSubscriber(this, mEventId);
      }

      void subscribe(Bus* pBus, const MESSAGE_ID pEventId, std::function<void(T param)> callback) {
        if (subscribed) {
          WDBGMSG("Trying to subscribe twice on the same Subscription!\n");
          return;
        }
        if (pBus == nullptr || pEventId >= TOTAL_MESSAGE_IDS || pEventId < 0) {
          assert(false);
        }
        mBus = pBus;
        mEventId = pEventId;
        mCallback = callback;
        mBus->addSubscriber(this, pEventId);
        subscribed = true;
      }
    };

    /**
     * Fires an event on the bus provided with the template type as the transmitted message
     * @param b The bus to target
     * @param pEventId The enumeration which identifies the event
     * @param param The message to transmit
     */
    template <class T>
    void fireEvent(Bus* b, const MESSAGE_ID pEventId, T param) {
      if (b == nullptr) { return; }
      if (b->mSubscriptions[pEventId].size() == 0) {
        WDBGMSG("Fired a event with no subscribers!\n");
        return;
      }
      SubsVector& subs = b->mSubscriptions[pEventId];
      LockGuard lock(b->mMutex);
      for (int i = 0; i < subs.size(); i++) {
        Subscription<T>* sub = dynamic_cast<Subscription<T>*>(subs[i]);
        if (sub != nullptr) {
          sub->mCallback(param);
        }
      }
    }
  }
}