#pragma once
#include <map>
#include <functional>
#include <string>
#include <vector>
#include "mutex.h"
#include "EventList.h"
#include "GStructs.h"

using namespace std;

/**
 * This allows for easy communication between classes which don't know each other
 * A list of events can be found in EventList.h
 */
namespace MessageBus {
  // Base class to take advantage of polymorphism
  class BaseSubscription {
  public:
    virtual ~BaseSubscription() { };
  protected:
    bool subscribed = false;
    MESSAGE_ID mEventId = TOTAL_MESSAGE_IDS;
  };

  typedef WDL_PtrList<BaseSubscription> SubsVector;

  // The bus object knows about all the subscribers and relays the events
  class Bus {
  public:
    SubsVector mSubscriptions[TOTAL_MESSAGE_IDS];
    WDL_Mutex mMutex;
    int mSubCount = 0;
    Bus() {}
    ~Bus() {
      /**
       * If the destructor is called, the plugin was probably destroyed
       * The bus is not responible for the lifetimes of it's subsriptions
       * so just clean out all the references and call it a day
       */

      for (int i = 0; i < TOTAL_MESSAGE_IDS; i++) {
        if (mSubscriptions[i].GetSize() > 0) {
          mSubscriptions[i].Empty(false);
        }
      }
      mSubCount = 0;
    }

    void addSubscriber(BaseSubscription* sub, const MESSAGE_ID pEventId) {
      WDL_MutexLock lock(&mMutex);
      mSubscriptions[pEventId].Add(sub);
      mSubCount++;
      if (mSubCount > 1000) {
        // This probably means there's a leak
        WDBGMSG("Subcount %i\n", mSubCount);
      }
    }

    void removeSubscriber(BaseSubscription* sub, const MESSAGE_ID pEventId) {
      if (mSubCount > 0) {
        WDL_MutexLock lock(&mMutex);
        mSubscriptions[pEventId].DeletePtr(sub);
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
    function<void(T param)> mCallback;

    Subscription(Bus* pBus, const MESSAGE_ID pEventId, function<void(T param)> callback) {
      subscribed = false;
      subscribe(pBus, pEventId, callback);
    }

    Subscription() {
      subscribed = false;
    }

    ~Subscription() {
      mBus->removeSubscriber(this, mEventId);
    }

    void subscribe(Bus* pBus, const MESSAGE_ID pEventId, function<void(T param)> callback) {
      if (subscribed) {
        WDBGMSG("Trying to subscribe twice on the same Subscription!\n");
        return;
      }
      mBus = pBus;
      mBus->addSubscriber(this, pEventId);
      subscribed = true;
      mEventId = pEventId;
      mCallback = callback;
    }
  };

  template <class T>
  void fireEvent(Bus* b, const MESSAGE_ID pEventId, T param) {
    if (b->mSubscriptions[pEventId].GetSize() == 0) {
      WDBGMSG("Fired a event with not subscribers!\n");
      return;
    }
    SubsVector& subs = b->mSubscriptions[pEventId];
    WDL_MutexLock lock(&b->mMutex);
    for (int i = 0; i < subs.GetSize(); i++) {
      Subscription<T>* sub = dynamic_cast<Subscription<T>*>(subs.Get(i));
      if (sub != nullptr) {
        sub->mCallback(param);
      }
    }
  }
}
