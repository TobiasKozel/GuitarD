#pragma once

namespace guitard {

  template <class T, int N>
  class PointerStack {
    T* mStack[N] = { nullptr };
    int mIndex = 0;
  public:
    bool push(T* i) {
      if (mIndex < N) {
        mStack[mIndex] = i;
        mIndex++;
        return true;
      }
      return false;
    }

    T* pop() {
      if (mIndex > 0) {
        mIndex--;
        return mStack[mIndex];
      }
      return nullptr;
    }
  };
}
