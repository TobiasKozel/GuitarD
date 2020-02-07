#pragma once

/**
 * Little helper class IControls can derive from if they need additional
 * functionality as a child in a ScrollViewControl
 */
namespace guitard {
  class ScrollViewChild {
  protected:
    bool mIsInView = false;
  public:
    virtual ~ScrollViewChild() = default;

    /**
     * Called from the scroll view on each scroll event
     */
    void onScroll(const bool inView) {
      if (mIsInView && !inView) { onScrollOutView(); }
      if (!mIsInView && inView) { onScrollInView(); }
      mIsInView = inView;
    }

    /**
     * called when the element is scrolled into view
     */
    virtual void onScrollInView() {

    }

    /**
     * Called when the element is scrolled out of view
     */
    virtual void onScrollOutView() {

    }
  };
}