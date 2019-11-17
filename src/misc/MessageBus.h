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
    bool subscribed;
    EVENTID mEventId;
  };

  typedef WDL_PtrList<BaseSubscription> SubsVector;

  // The bus object knows about all the subscribers and relays the events
  class Bus {
    SubsVector subscriptions[TOTALEVENTS];
    WDL_Mutex eventLock;
    int globalSubs = 0;
  public:
    template <class T>
    void fireEvent(EVENTID pEventId, T param) {
      if (subscriptions[pEventId].GetSize() == 0) {
        WDBGMSG("Fired a event with not subscribers!\n");
        return;
      }
      SubsVector& subs = subscriptions[pEventId];
      WDL_MutexLock lock(&eventLock);
      for (int i = 0; i < subs.GetSize(); i++) {
        Subscription<T>* sub = dynamic_cast<Subscription<T>*>(subs.Get(i));
        if (sub != nullptr) {
          sub->mCallback(param);
        }
      }
    }

    void addSubscriber(BaseSubscription* sub, EVENTID pEventId) {
      WDL_MutexLock lock(&eventLock);
      subscriptions[pEventId].Add(sub);
      globalSubs++;
      if (globalSubs > 1000) {
        // This probably means there's a leak
        WDBGMSG("Subcount %i\n", globalSubs);
      }
    }

    void removeSubscriber(BaseSubscription* sub, EVENTID pEventId) {
      WDL_MutexLock lock(&eventLock);
      subscriptions[pEventId].DeletePtr(sub);
      globalSubs--;
    }
  };

  // Actual implementation based on the data type to pass over the Bus
  template <class T>
  class Subscription : public BaseSubscription {
    // The bus it is subscribed to
    Bus* mBus;
  public:
    function<void(T param)> mCallback;

    Subscription(Bus* pBus, EVENTID pEventId, function<void(T param)> callback) {
      subscribed = false;
      subscribe(pBus, pEventId, callback);
    }

    Subscription() {
      subscribed = false;
    }

    ~Subscription() {
      mBus->removeSubscriber(this, mEventId);
    }

    void subscribe(Bus* pBus, EVENTID pEventId, function<void(T param)> callback) {
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
}