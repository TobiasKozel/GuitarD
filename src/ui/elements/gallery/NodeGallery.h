#pragma once
#include "../scroll/ScrollViewControl.h"
#include "../../bus/MessageBus.h"
#include "./NodeGalleryCategory.h"

namespace guitard {
  class NodeGallery : public ScrollViewControl {
    MessageBus::Bus* mBus = nullptr;
  public:

    NodeGallery(MessageBus::Bus* pBus) : ScrollViewControl(IRECT()) {
      mBus = pBus;
    }

    void OnInit() override {
      /*
       * This will get deleted when the ui gets rid of it, not pretty but there's
       * no onDetach event to use for doing cleanup
       */
      ScrollViewControl::OnInit();
      setFullWidthChildren(true);
      setDoDragScroll(false);
      init();
      OnResize();
    }

  private:
    /**
     * This will create all the categories and add the nodes to it
     */
    void init() {
      std::map<String, GalleryCategory*> uniqueCat;
      for (auto& i : NodeList::nodeList) {
        if (i.second.hidden) { continue; }
        if (uniqueCat.find(i.second.categoryName) == uniqueCat.end()) {
          GalleryCategory* cat = new GalleryCategory(mBus);
          uniqueCat.insert(std::pair<String, GalleryCategory*>(i.second.categoryName, cat));
          appendChild(cat);
        }
      }
      for (auto& i : NodeList::nodeList) {
        if (i.second.hidden) { continue; }
        uniqueCat.at(i.second.categoryName)->addNode(&i.second);
      }
    }
  };
}
