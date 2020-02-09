#pragma once

#ifndef GUITARD_HEADLESS
#include "IControl.h"
#include "../ui/ScrollViewControl.h"
#include "../misc/MessageBus.h"
#include "../ui/theme.h"
#include "./gallery/NodeGallery.h"
#include "./preset/PresetBrowser.h"

namespace guitard {
  class SideBar : public IControl {
    MessageBus::Bus* mBus = nullptr;
  public:
    ScrollViewControl mScrollview;
    bool mIsOpen = false;
    IText mStats;
    long long avgExecutionTime;
    MessageBus::Subscription<bool> mOpenGalleryEvent;
    NodeGallery mNodeGallery;
    PresetBrowser mPresetBrowser;
    int mOpenTab = 0;
    static const int mTabCount = 2;
    IControl* mTabs[mTabCount] = { nullptr };

    SideBar(MessageBus::Bus* pBus, IGraphics* g) :
      IControl(IRECT(), kNoParameter), mNodeGallery(pBus, g), mPresetBrowser(pBus, g)
    {
      mBus = pBus;
      mOpenGalleryEvent.subscribe(mBus, MessageBus::OpenGallery, [&](bool open) {
        this->openGallery(open);
      });
      avgExecutionTime = 0;
      mStats = DEBUG_FONT;
      SetRenderPriority(11);
      mTabs[0] = &mNodeGallery;
      mTabs[1] = &mPresetBrowser;
    }

    void OnInit() override {
      mScrollview.SetRenderPriority(12);
      mScrollview.setChildPadding(1.f); // Should be 0 but this makes sure there's no trace of the other tabs
      mScrollview.setFullWidthChildren(true);
      mScrollview.setDoDragScroll(false);
      mScrollview.setDoScroll(false);
      mScrollview.setScrollBarEnable(false);
      mScrollview.setCleanUpEnabled(false); // All the children of the scrollview are on the stack of this object
      GetUI()->AttachControl(&mScrollview);
      mScrollview.appendChild(&mNodeGallery);
      mScrollview.appendChild(&mPresetBrowser);
      OnResize();
    }

    void OnDetached() override {
      GetUI()->DetachControl(&mScrollview);
      IControl::OnDetached();
    }

    void openGallery(const bool open = true) {
      if (open == mIsOpen) { return; }
      mIsOpen = open;
      OnResize();
      mScrollview.Hide(!open);
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

    void drawButton(IGraphics& g) {
      g.FillCircle(Theme::Colors::ACCENT, mRECT);
      float x1 = mRECT.L + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_SIZE / 2);
      float y1 = mRECT.T + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_ICON_SIZE / 2);
      g.FillRect(iplug::igraphics::COLOR_WHITE, IRECT(
                   x1, y1, x1 + Theme::Gallery::BUTTON_SIZE, y1 + Theme::Gallery::BUTTON_ICON_SIZE
                 ));
      x1 = mRECT.L + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_ICON_SIZE / 2);
      y1 = mRECT.T + Theme::Gallery::ADD_CIRCLE_RADIUS - (Theme::Gallery::BUTTON_SIZE / 2);
      g.FillRect(iplug::igraphics::COLOR_WHITE, IRECT(
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
        IRECT main = bounds.GetPadded(-Theme::Gallery::PADDING);
        main.R -= 20;
        for (int i = 0; i < mTabCount; i++) {
          mTabs[i]->SetTargetAndDrawRECTs(main);
        }
        // mTabs[0]->SetTargetAndDrawRECTs(main);
        mScrollview.SetTargetAndDrawRECTs(main /** .GetVSliced(40) */);
        mScrollview.scrollTo(mOpenTab);
        // mSearch->SetTargetAndDrawRECTs(main.GetFromTop(30));
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
      else {
        if (mod.R || true) {
          mOpenTab++;
          if (mOpenTab >= mTabCount) {
            mOpenTab = 0;
          }
          OnResize();
        }
      }
    }

  };
}
#endif