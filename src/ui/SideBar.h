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
    ScrollViewControl mScrollView;
    bool mIsOpen = false;
    IText mStats;
    long long avgExecutionTime;
    MessageBus::Subscription<bool> mOpenGalleryEvent;
    NodeGallery mNodeGallery;
    PresetBrowser mPresetBrowser;
    int mOpenTab = 0;

    static const int mTabCount = 2;

    const IRECT mHeaderTextBox = { 0, 0, Theme::SideBar::HEADER_HEIGHT, Theme::SideBar::HEADER_WITH };

    struct Tab {
      IRECT header;
      ILayerPtr layer;
      IControl* tab = nullptr;
      String name;
    };

    bool mBgCached = false;

    Tab mTabs[mTabCount];

    SideBar(MessageBus::Bus* pBus) :
      IControl(IRECT(), kNoParameter), mNodeGallery(pBus), mPresetBrowser(pBus)
    {
      mBus = pBus;
      mOpenGalleryEvent.subscribe(mBus, MessageBus::OpenGallery, [&](bool open) {
        this->openGallery(open);
      });
      avgExecutionTime = 0;
      mStats = DEBUG_FONT;
      SetRenderPriority(11);
      mTabs[0].tab = &mNodeGallery;
      mTabs[0].name = "Nodes";
      mTabs[1].tab = &mPresetBrowser;
      mTabs[1].name = "Presets";
    }

    void OnAttached() override {
      mScrollView.SetRenderPriority(12);
      mScrollView.setDoDragScroll(false);
      mScrollView.setDoScroll(false);
      mScrollView.setScrollBarEnable(false);
      mScrollView.setCleanUpEnabled(false); // All the children of the scroll view are on the stack of this object
      GetUI()->AttachControl(&mScrollView);
      mScrollView.appendChild(&mNodeGallery);
      mScrollView.appendChild(&mPresetBrowser);
    }

    void OnDetached() override {
      GetUI()->DetachControl(&mScrollView);
      IControl::OnDetached();
    }

    void openGallery(const bool open = true) {
      if (open == mIsOpen) { return; }
      mIsOpen = open;
      OnResize();
      mScrollView.Hide(!open);
      if (!mIsOpen) {
        mScrollView.OnMouseOut();
      }
      GetUI()->SetAllControlsDirty();
    }

    void Draw(IGraphics& g) override {
      if (mIsOpen) {
        g.FillRect(Theme::Gallery::BACKGROUND, mRECT);
        drawTabHeaders(g);
        // small gap between the tabs and content
        g.FillRect(Theme::Gallery::BACKGROUND,
          mRECT.GetReducedFromRight(Theme::SideBar::HEADER_WITH + Theme::Gallery::PADDING - 2).GetFromRight(2)
        );
      }
      else {
        drawButton(g);
      }
    }

    void drawTabHeaders(IGraphics& g) {
      for (int i = 0; i < mTabCount; i++) {
        g.FillRect(i == mOpenTab ?
          Theme::Gallery::CATEGORY_TITLE_BG_OPEN :
          Theme::Gallery::CATEGORY_TITLE_BG, mTabs[i].header
        );
        if (!mBgCached) {
          IRECT layerPost = mTabs[i].header;
          layerPost.Translate(
            -mHeaderTextBox.W() * 0.5 + mHeaderTextBox.H() * 0.5,
            mHeaderTextBox.W() * 0.5 - mHeaderTextBox.H() * 0.5
          );
          layerPost.R = layerPost.L + mHeaderTextBox.W();
          layerPost.B = layerPost.T + mHeaderTextBox.H();
          g.StartLayer(this, layerPost);
          // g.FillRect(Theme::Colors::ACCENT, layerPost);
          g.DrawText(Theme::Gallery::CATEGORY_TITLE, mTabs[i].name.c_str(), layerPost);
          mTabs[i].layer = g.EndLayer();
        }
        g.DrawRotatedLayer(mTabs[i].layer, 90);
      }
      mBgCached = true;
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
      //GraphStats* stats;
      //MessageBus::fireEvent<GraphStats**>(mBus, MessageBus::GetGraphStats, &stats);
      //avgExecutionTime = static_cast<long long> ((59 * avgExecutionTime + stats->executionTime) / 60.0);
      //const std::string time = std::to_string(avgExecutionTime);
      //g.DrawText(mStats, time.c_str(), mRECT);
      //mDirty = true;
    }

    void OnResize() override {
      mBgCached = false;
      IRECT bounds = GetUI()->GetBounds();
      if (mIsOpen) {
        bounds.Pad(-Theme::Gallery::PADDING);
        // Only take up half the screen
        bounds.L = bounds.R * 0.5f;
        mRECT = bounds;
        mTargetRECT = bounds;
        IRECT main = bounds.GetPadded(-Theme::Gallery::PADDING);
        main.R -= Theme::SideBar::HEADER_WITH;
        for (int i = 0; i < mTabCount; i++) {
          if (i == mOpenTab) {
            mTabs[i].tab->Hide(false);
            mTabs[i].tab->SetTargetAndDrawRECTs(main);
          }
          else {
            mTabs[i].tab->Hide(true);
          }
          mTabs[i].header.L = main.R;
          mTabs[i].header.R = main.R + Theme::SideBar::HEADER_WITH;
          mTabs[i].header.T = main.T + i * (Theme::SideBar::HEADER_HEIGHT + Theme::SideBar::HEADER_PADDING);
          mTabs[i].header.B = mTabs[i].header.T + Theme::SideBar::HEADER_HEIGHT;
        }
        mScrollView.SetTargetAndDrawRECTs(main);
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
          IRECT mouse = { x, y, x, y };
          for (int i = 0; i < mTabCount; i++) {
            if (mTabs[i].header.Contains(mouse)) {
              mOpenTab = i;
              mDirty = true;
              OnResize();
              break;
            }
          }
        }
      }
    }

  };
}
#endif