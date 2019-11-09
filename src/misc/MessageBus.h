#pragma once
#include <map>
#include <functional>
#include <string>
#include <vector>
#include "mutex.h"

using namespace std;

namespace MessageBus {

  class BaseSubscription {
  public:
    virtual ~BaseSubscription() { };
  protected:
    bool subscribed;
    string mEventName;
  };

  typedef WDL_PtrList<BaseSubscription> SubsVector;

  map<string, SubsVector> subscriptions;

  WDL_Mutex unsubscribeLock;

  int globalSubs = 0;

  template <class T>
  class Subscription : public BaseSubscription {
  public:
    function<void(T param)> mCallback;

    Subscription(string eventName, function<void(T param)> callback) {
      subscribed = false;
      subscribe(eventName, callback);
    }

    Subscription() {
      subscribed = false;
    }

    ~Subscription() {
      globalSubs--;
      WDL_MutexLock lock(&unsubscribeLock);
      if (subscriptions.find(mEventName) != subscriptions.end()) {
        subscriptions.find(mEventName)->second.DeletePtr(this);
      }
    }

    void subscribe(string eventName, function<void(T param)> callback) {
      if (subscribed) {
        WDBGMSG("Trying to subscribe twice on the same Subscription!\n");
        return;
      }
      globalSubs++;
      if (globalSubs > 1000) {
        // This probably means there's a leak
        WDBGMSG("Subcount %i\n", globalSubs);
      }
      WDL_MutexLock lock(&unsubscribeLock);
      subscribed = true;
      mEventName = eventName;
      mCallback = callback;
      if (subscriptions.find(eventName) == subscriptions.end()) {
        SubsVector temp;
        temp.Add(this);
        subscriptions.insert(pair<string, SubsVector>(eventName, temp));
      }
      else {
        subscriptions.find(eventName)->second.Add(this);
      }
    }
  };

  template <class T>
  void fireEvent(string eventName, T param) {
    if (subscriptions.find(eventName) == subscriptions.end()) {
      WDBGMSG("Fired a event with not subscribers!\n");
      return;
    }
    WDL_MutexLock lock(&unsubscribeLock);
    SubsVector &subs = subscriptions.find(eventName)->second;
    for (int i = 0; i < subs.GetSize(); i++) {
      Subscription<T>* sub = dynamic_cast<Subscription<T>*>(subs.Get(i));
      if (sub != nullptr) {
        sub->mCallback(param);
      }
    }
  }
}