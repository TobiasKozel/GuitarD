#pragma once
#ifndef GUITARD_HEADLESS
#include "../../ui/ScrollViewControl.h"
#include "../../misc/MessageBus.h"
#include "./NodeGalleryCategory.h"

namespace guitard {
  class NodeGallery : public ScrollViewControl {
    MessageBus::Bus* mBus = nullptr;
  public:

    NodeGallery(MessageBus::Bus* pBus, IGraphics* g) :
      ScrollViewControl(IRECT())
    {
      mBus = pBus;
    }

    void OnInit() override {
      /*
       * This will get deleted when the ui gets rid of it, not pretty but there's
       * no onDetach event to use for doing cleanup
       */
      ScrollViewControl::OnInit();
      setFullWidthChildren(true);
      // setDoDragScroll(false);
      init();
      OnResize();
    }

  private:
    /**
     * This will create all the categories and add the nodes to it
     */
    void init() {
      std::map<std::string, GalleryCategory*> uniqueCat;
      for (auto& i : NodeList::nodelist) {
        if (uniqueCat.find(i.second.categoryName) == uniqueCat.end()) {
          GalleryCategory* cat = new GalleryCategory(mBus);
          uniqueCat.insert(std::pair<std::string, GalleryCategory*>(i.second.categoryName, cat));
          appendChild(cat);
        }
      }
      for (auto& i : NodeList::nodelist) {
        uniqueCat.at(i.second.categoryName)->addNode(&i.second);
      }
    }
  };
}
#endif