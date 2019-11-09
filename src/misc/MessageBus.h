#pragma once
#include <map>
#include <functional>
#include <string>
#include <vector>
#include "mutex.h"
#include "EventList.h"

using namespace std;

/**
 * This allows for easy communication between classes which don't know each other
 * A list of events can be found in EventList.h
 */
namespace MessageBus {
  class BaseSubscription {
  public:
    virtual ~BaseSubscription() { };
  protected:
    bool subscribed;
    EVENTID mEventId;
  };

  typedef WDL_PtrList<BaseSubscription> SubsVector;

  SubsVector subscriptions[TOTALEVENTS];

  WDL_Mutex eventLock;

  int globalSubs = 0;

  template <class T>
  class Subscription : public BaseSubscription {
  public:
    function<void(T param)> mCallback;

    Subscription(EVENTID pEventId, function<void(T param)> callback) {
      subscribed = false;
      subscribe(pEventId, callback);
    }

    Subscription() {
      subscribed = false;
    }

    ~Subscription() {
      globalSubs--;
      WDL_MutexLock lock(&eventLock);
      subscriptions[mEventId].DeletePtr(this);
    }

    void subscribe(EVENTID pEventId, function<void(T param)> callback) {
      if (subscribed) {
        WDBGMSG("Trying to subscribe twice on the same Subscription!\n");
        return;
      }
      globalSubs++;
      if (globalSubs > 1000) {
        // This probably means there's a leak
        WDBGMSG("Subcount %i\n", globalSubs);
      }
      subscribed = true;
      mEventId = pEventId;
      mCallback = callback;
      WDL_MutexLock lock(&eventLock);
      subscriptions[mEventId].Add(this);
    }
  };

  template <class T>
  void fireEvent(EVENTID pEventId, T param) {
    if (subscriptions[pEventId].GetSize() == 0) {
      WDBGMSG("Fired a event with not subscribers!\n");
      return;
    }
    SubsVector &subs = subscriptions[pEventId];
    WDL_MutexLock lock(&eventLock);
    for (int i = 0; i < subs.GetSize(); i++) {
      Subscription<T>* sub = dynamic_cast<Subscription<T>*>(subs.Get(i));
      if (sub != nullptr) {
        sub->mCallback(param);
      }
    }
  }
}