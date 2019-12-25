#pragma once
#include "IControl.h"
#include "src/ui/ScrollViewControl.h"
#include "src/misc/MessageBus.h"
#include "NodeGalleryCategory.h"
#include "src/ui/theme.h"

using namespace iplug;
using namespace igraphics;


class NodeGallery : public IControl {
  MessageBus::Bus* mBus = nullptr;
public:
  ScrollViewControl* mScrollview;
  bool mIsOpen = false;
  IText mStats;
  long long avgExecutionTime;
  MessageBus::Subscription<bool> mOpenGalleryEvent;

  NodeGallery(MessageBus::Bus* pBus, IGraphics* g) :
    IControl(IRECT(), kNoParameter)
  {
    mBus = pBus;
    mOpenGalleryEvent.subscribe(mBus, MessageBus::OpenGallery, [&](bool open) {
      this->openGallery(open);
    });
    avgExecutionTime = 0;
    mStats = DEBUG_FONT;
    setRenderPriority(11);
  }

  void OnInit() override {
    /*
     * This will get deleted when the ui gets rid of it, not pretty but there's
     * no onDetach event to use for doing cleanup
     */
    mScrollview = new ScrollViewControl();
    mScrollview->setRenderPriority(12);
    mScrollview->setFullWidthChildren(true);
    GetUI()->AttachControl(mScrollview);
    init();
    OnResize();
  }

  ~NodeGallery() {
  }

  void openGallery(const bool open = true) {
    if (open == mIsOpen) { return; }
    mIsOpen = open;
    OnResize();
    mScrollview->Hide(!open);
    if (!mIsOpen) {
      GetUI()->SetAllControlsDirty();
    }
  }

  void Draw(IGraphics& g) override {
    if (mIsOpen) {
      g.FillRect(Theme::Gallery::BACKGROUND, mRECT);
    }
    else {
      drawButton(g);
    }
  }

  inline void drawButton(IGraphics& g) {
    g.FillCircle(Theme::Colors::ACCENT, mRECT);
    float x1 = mRECT.L + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_SIZE / 2);
    float y1 = mRECT.T + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_ICON_SIZE / 2);
    g.FillRect(COLOR_WHITE, IRECT(
      x1, y1, x1 + Theme::Gallery::BUTTON_SIZE, y1 + Theme::Gallery::BUTTON_ICON_SIZE
    ));
    x1 = mRECT.L + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_ICON_SIZE / 2);
    y1 = mRECT.T + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_SIZE / 2);
    g.FillRect(COLOR_WHITE, IRECT(
      x1, y1, x1 + Theme::Gallery::BUTTON_ICON_SIZE, y1 + Theme::Gallery::BUTTON_SIZE
    ));
    GraphStats* stats;
    MessageBus::fireEvent<GraphStats**>(mBus, MessageBus::GetGraphStats, &stats);
    avgExecutionTime = static_cast<long long> ((59 * avgExecutionTime + stats->executionTime) / 60.0);
    const std::string time = std::to_string(avgExecutionTime);
    g.DrawText(mStats, time.c_str(), mRECT);
    mDirty = true;
  }

  void OnResize() override {
    IRECT bounds = GetUI()->GetBounds();
    if (mIsOpen) {
      bounds.Pad(-Theme::Gallery::PADDING);
      // Only take up half the screen
      bounds.L = bounds.R * 0.5f;
      mRECT = bounds;
      mTargetRECT = bounds;
      mScrollview->SetTargetAndDrawRECTs(bounds.GetPadded(-Theme::Gallery::PADDING));
    }
    else {
      bounds.Pad(-Theme::Gallery::PADDING);
      bounds.L = bounds.R - Theme::Gallery::ADD_CIRCLE_DIAMETER;
      bounds.B = bounds.T + Theme::Gallery::ADD_CIRCLE_DIAMETER;
      mRECT = bounds;
      mTargetRECT = bounds;
    }
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    if (!mIsOpen) {
      openGallery();
    }
  }

private:
  /**
   * This will create all the categories and add the nodes to it
   */
  void init() const {
    std::map<std::string, GalleryCategory*> uniqueCat;
    for (auto i : NodeList::nodelist) {
      if (uniqueCat.find(i.second.categoryName) == uniqueCat.end()) {
        GalleryCategory* cat = new GalleryCategory(mBus);
        uniqueCat.insert(std::pair<std::string, GalleryCategory*>(i.second.categoryName, cat));
        mScrollview->appendChild(cat);
      }
    }
    for (auto i : NodeList::nodelist) {
      uniqueCat.at(i.second.categoryName)->addNode(i.second);
    }
  }
};