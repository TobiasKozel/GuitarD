#pragma once

// #define GUITARD_HEADLESS
#ifdef GUITARD_HEADLESS
#include <vector>
#else
#include "ptrlist.h"
#endif

namespace guitard {
  template <class T>
  class PointerList {
#ifdef GUITARD_HEADLESS
    std::vector<T*> mList;
#else
    WDL_PtrList<T> mList;
#endif

  public:
    T* get(const size_t index) const {
#ifdef GUITARD_HEADLESS
      return mList.at(index);
#else
      return mList.Get(index);
#endif
    }

    T* operator[](const size_t index) const {
      return get(index);
    }

    void add(T* element) {
#ifdef GUITARD_HEADLESS
      mList.push_back(element);
#else
      mList.Add(element);
#endif
    }

    void remove(const T* element, bool const destroy = false) {
#ifdef GUITARD_HEADLESS
      auto position = std::find(mList.begin(), mList.end(), element);
      if (position != mList.end()) {
        mList.erase(position);
      }
#else
      mList.DeletePtr(element);
#endif
      if (destroy) {
        delete element;
      }
    }

    void remove(const size_t index, bool const destroy = false) {
      remove(get(index), destroy);
    }

    size_t size() const {
#ifdef GUITARD_HEADLESS
      return mList.size();
#else
      return mList.GetSize();
#endif
    }

    size_t find(const T* element) const {
#ifdef GUITARD_HEADLESS
      auto position = std::find(mList.begin(), mList.end(), element);
      if (position != mList.end()) {
        return std::distance(mList.begin(), position);
      }
      return -1;
#else
      return mList.Find(element);
#endif
    }

    void clear(const bool destroy = false) {
      if (destroy) {
        for (int i = 0; i < size(); i++) {
          delete get(i);
        }
      }
#ifdef GUITARD_HEADLESS
      mList.clear();
#else
      mList.Empty();
#endif
    }
  };
}