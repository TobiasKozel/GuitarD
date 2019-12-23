#pragma once
#include "IControl.h"
#include "src/node/Node.h"
#include "src/misc/MessageBus.h"
#include "NodeGalleryCategory.h"
#include "src/ui/theme.h"

using namespace iplug;
using namespace igraphics;


class NodeGallery : public IControl {
  MessageBus::Bus* mBus;
public:
  bool mIsOpen;
  int mDistanceDragged = -1;
  float mPadding;
  IGraphics* mGraphics;
  WDL_PtrList<GalleryCategory> mCategories;
  IRECT mViewPort;
  IRECT mViewPortBounds;
  IText mStats;
  long long avgExecutiontime;
  MessageBus::Subscription<bool> mOpenGalleryEvent;

  NodeGallery(MessageBus::Bus* pBus, IGraphics* g) :
    IControl(IRECT(), kNoParameter)
  {
    mBus = pBus;
    mPadding = 10;
    mGraphics = g;
    init();
    mIsOpen = false;
    OnResize();
    mOpenGalleryEvent.subscribe(mBus, MessageBus::OpenGallery, [&](bool open) {
      this->openGallery(open);
    });
    avgExecutiontime = 0;
    mStats = DEBUG_FONT;
  }

  ~NodeGallery() {
    mCategories.Empty(true);
  }

  void openGallery(bool open = true) {
    if (open == mIsOpen) { return; }
    mIsOpen = open;
    OnResize();
    if (mIsOpen) {
      // ensure it's on top of the drawing stack
      mGraphics->RemoveControl(this);
      mGraphics->AttachControl(this);
    }
    else {
      // redraw the whole screen
      mGraphics->SetAllControlsDirty();
    }
  }

  void Draw(IGraphics& g) override {
    if (mIsOpen) {
      g.FillRect(Theme::Gallery::BACKGROUND, mRECT);
      for (int i = 0; i < mCategories.GetSize(); i++) {
        mCategories.Get(i)->Draw(g);
      }
      /** Some trickery to give the scroll list padding*/
      g.FillRect(Theme::Gallery::BACKGROUND, IRECT(
        mRECT.L, mRECT.T - 1, mRECT.R, mRECT.T + Theme::Gallery::PADDING
      ));
      g.FillRect(Theme::Gallery::BACKGROUND, IRECT(
        mRECT.L, mRECT.B - Theme::Gallery::PADDING, mRECT.R, mRECT.B + 1
      ));
    }
    else {
      g.FillCircle(Theme::Colors::ACCENT, mRECT);
      float x1 = mRECT.L + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_SIZE / 2);
      float y1 = mRECT.T + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_ICON_SIZE / 2);
      g.FillRect(Theme::Gallery::BACKGROUND, IRECT(
        x1, y1, x1 + Theme::Gallery::BUTTON_SIZE, y1 + Theme::Gallery::BUTTON_ICON_SIZE
      ));
      x1 = mRECT.L + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_ICON_SIZE / 2);
      y1 = mRECT.T + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_SIZE / 2);
      g.FillRect(Theme::Gallery::BACKGROUND, IRECT(
        x1, y1, x1 + Theme::Gallery::BUTTON_ICON_SIZE, y1 + Theme::Gallery::BUTTON_SIZE
      ));
      GraphStats* stats;
      MessageBus::fireEvent<GraphStats**>(mBus, MessageBus::GetGraphStats, &stats);
      avgExecutiontime = static_cast<long long> ((59 * avgExecutiontime + stats->executionTime) / 60.0);
      const std::string time = std::to_string(avgExecutiontime);
      g.DrawText(mStats, time.c_str(), mRECT);
      mDirty = true;
    }
  }

  void OnResize() override {
    if (mGraphics != nullptr) {
      IRECT bounds = mGraphics->GetBounds();
      if (mIsOpen) {
        bounds.Pad(-Theme::Gallery::PADDING);
        bounds.L = bounds.R * 0.5f;
        mRECT = bounds;
        mTargetRECT = bounds;
        float top = mViewPort.T;
        mViewPort = bounds;
        mViewPort.Pad(-Theme::Gallery::PADDING);
        mViewPort.T = top;
      }
      else {
        bounds.Pad(-Theme::Gallery::PADDING);
        bounds.L = bounds.R - Theme::Gallery::ADD_CIRCLE_DIAMETER;
        bounds.B = bounds.T + Theme::Gallery::ADD_CIRCLE_DIAMETER;
        mRECT = bounds;
        mTargetRECT = bounds;
        mViewPort = bounds;
      }
    }
  }

  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override {
    if (!mIsOpen) { return; }
    scroll(d * 20);
  }

  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    scroll(dY);
    mDistanceDragged += static_cast<int>(abs(dX) + abs(dY));
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    if (mIsOpen && mDistanceDragged < 4) {
      GalleryCategory* cat;
      for (int i = 0; i < mCategories.GetSize(); i++) {
        cat = mCategories.Get(i);
        if (cat->mRECT.Contains(IRECT(x, y, x, y))) {
          NodeList::NodeInfo* ret = cat->OnMouseDown(x, y, mod);
          if (ret != nullptr) {
            MessageBus::fireEvent<NodeList::NodeInfo>(mBus, MessageBus::NodeAdd, *ret);
          }
          mDirty = true;
        }
      }
    }
    else {
      openGallery();
    }
    mDistanceDragged = -1;
  }

private:
  /**
   * This will create all the categories and add the nodes to it
   */
  void init() {
    std::map<std::string, GalleryCategory*> uniqueCat;
    GalleryCategory* prevCat = nullptr;
    for (auto i : NodeList::nodelist) {
      if (uniqueCat.find(i.second.categoryName) == uniqueCat.end()) {
        GalleryCategory* cat = new GalleryCategory(prevCat, &mViewPort);
        prevCat = cat;
        uniqueCat.insert(std::pair<std::string, GalleryCategory*>(i.second.categoryName, cat));
        mCategories.Add(cat);
      }
    }
    for (auto i : NodeList::nodelist) {
      uniqueCat.at(i.second.categoryName)->addNode(i.second);
    }
  }

  void scroll(float d) {
    mViewPort.Translate(0, d);
    if (mViewPort.T > Theme::Gallery::PADDING * 2) {
      mViewPort.T = Theme::Gallery::PADDING * 2;
      //mViewPort.Translate(0, -mViewPort.T);
    }
    else {
    }
    mDirty = true;
  }


};