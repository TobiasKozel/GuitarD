#pragma once

#ifndef GUITARD_HEADLESS
#include "IControl.h"
#include "../../../thirdparty/soundwoofer/soundwoofer.h"
#include "../../ui/ScrollViewControl.h"
#include "../../ui/theme.h"
#endif
#include <functional>
#include "../../types/gstructs.h"

namespace guitard {

  struct CabLibNodeSharedData {
    std::function<void(soundwoofer::SWImpulseShared)> callback;
    soundwoofer::SWImpulseShared loadedIr;
  };
#ifndef GUITARD_HEADLESS
  class MicPosition : public IControl {
  public:
    typedef std::function<void(MicPosition * c)> MicPositionCallback;
    bool mSelected = false;
    MicPositionCallback mCallback;
    MicPosition(MicPositionCallback& callback) : IControl({ 0, 0, 0, 20 }) {
      mCallback = callback;
    }

    void Draw(IGraphics& g) override {
      if (mSelected) {
        g.FillRect(Theme::IRBrowser::IR_TITLE_BG_ACTIVE, mRECT);
        g.DrawText(Theme::IRBrowser::IR_TITLE_ACTIVE, name.get(), mRECT.GetHPadded(-8));
      }
      else {
        if (mMouseIsOver) {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG_HOVER, mRECT);
        }
        else {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG, mRECT);
        }
        g.DrawText(Theme::IRBrowser::IR_TITLE, name.get(), mRECT.GetHPadded(-8));
      }
    }

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
      mCallback(this);
    }

    String name;
    String path;
  };

  class Microphone : public IControl {
  public:
    typedef std::function<void(Microphone * c)> MicrophoneCallback;
    MicrophoneCallback mCallback;
    String name;
    String path;
    PointerList<MicPosition> mPositions;
    MicPosition::MicPositionCallback mPosCallback;
    bool mSelected = false;
    Microphone(MicrophoneCallback& mc, MicPosition::MicPositionCallback& pc) : IControl({ 0, 0, 0, 20 }) {
      mCallback = mc;
      mPosCallback = pc;
    }
    ~Microphone() {
      mPositions.clear(true);
    }

    void Draw(IGraphics& g) override {
      if (mSelected) {
        g.FillRect(Theme::IRBrowser::IR_TITLE_BG_ACTIVE, mRECT);
        g.DrawText(Theme::IRBrowser::IR_TITLE_ACTIVE, name.get(), mRECT.GetHPadded(-8));
      }
      else {
        if (mMouseIsOver) {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG_HOVER, mRECT);
        }
        else {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG, mRECT);
        }
        g.DrawText(Theme::IRBrowser::IR_TITLE, name.get(), mRECT.GetHPadded(-8));
      }
    }

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
      mCallback(this);
    }

    void scanPositions() {
      //ScanDir dir(path.get());
      //for (int i = 0; i < dir.size(); i++) {
      //  if (!dir[i]->isFolder) {
      //    // We're only interested in files, each corresponds to a IR
      //    MicPosition* pos = new MicPosition(mPosCallback);
      //    pos->name = dir[i]->name.get();
      //    if (strncmp(".wav", pos->name.getExt(), 5) == 0 ||
      //      strncmp(".WAV", pos->name.getExt(), 5) == 0) {
      //      pos->path = dir[i]->relative.get();
      //      // dir.GetCurrentFullFN(&pos->path);
      //      mPositions.add(pos);
      //    }
      //    else {
      //      delete pos;
      //    }
      //  }
      //}
    }
  };

  class Cabinet : public IControl {
  public:
    typedef std::function<void(Cabinet * c)> CabinetCallback;
    std::string name;
    std::string path;
    CabinetCallback mCallback;
    Microphone::MicrophoneCallback mMicCallback;
    MicPosition::MicPositionCallback mPosCallback;
    PointerList<Microphone> mMics;
    bool mSelected = false;

    Cabinet(const CabinetCallback cc, const Microphone::MicrophoneCallback mc, const MicPosition::MicPositionCallback pc)
      : IControl({ 0, 0, 0, 20 })
    {
      mCallback = cc;
      mMicCallback = mc;
      mPosCallback = pc;
    }

    ~Cabinet() {
      mMics.clear(true);
    }

    void Draw(IGraphics& g) override {
      if (mSelected) {
        g.FillRect(Theme::IRBrowser::IR_TITLE_BG_ACTIVE, mRECT);
        g.DrawText(Theme::IRBrowser::IR_TITLE_ACTIVE, name.c_str(), mRECT.GetHPadded(-8));
      }
      else {
        if (mMouseIsOver) {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG_HOVER, mRECT);
        }
        else {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG, mRECT);
        }
        g.DrawText(Theme::IRBrowser::IR_TITLE, name.c_str(), mRECT.GetHPadded(-8));
      }
    }

    void scanMics() {
      //ScanDir dir(path.get());
      //for (int i = 0; i < dir.size(); i++) {
      //  if (dir[i]->isFolder) {
      //    // We're only interested in folders, each corresponds to a mic
      //    Microphone* mic = new Microphone(mMicCallback, mPosCallback);
      //    mMics.add(mic);
      //    mic->name = dir[i]->name.get();
      //    mic->path = dir[i]->relative.get();
      //    // dir.GetCurrentFullFN(&mic->path);
      //    mic->scanPositions();
      //  }
      //}
    }

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
      mCallback(this);
    }
  };



  class CabLibPopUp : public IControl {
    CabLibNodeSharedData* mCabShared = nullptr;
    IRECT mCloseButton;
    IRECT mPathTitle;
    ScrollViewControl* mScrollView[3] = { nullptr };
    PointerList<Cabinet> mCabinets;
    Cabinet* mSelectedCab = nullptr;
    Microphone* mSelectedMic = nullptr;
    MicPosition* mSelectedPosition = nullptr;
    String mPath;
  public:
    CabLibPopUp(CabLibNodeSharedData* shared) : IControl({}) {
      mRenderPriority = 15;
      mCabShared = shared;
    }

    void OnInit() override {
      for (int i = 0; i < 3; i++) {
        mScrollView[i] = new ScrollViewControl();
        mScrollView[i]->SetRenderPriority(16);
        mScrollView[i]->setFullWidthChildren(true);
        mScrollView[i]->setChildPadding(2);
        mScrollView[i]->setCleanUpEnabled(false);
        GetUI()->AttachControl(mScrollView[i]);
      }
      soundwoofer::async::listIRs([&](soundwoofer::Status status) {
        if (status != soundwoofer::SUCCESS) { return; }
        auto rigs = soundwoofer::instance().getRigs();
        for (auto& i : rigs) {
          Cabinet* cab = new Cabinet(
          [&](Cabinet* cab) { this->onCabChanged(cab); },
          [&](Microphone* mic) { this->onMicChanged(mic); },
          [&](MicPosition* pos) { this->onPositionChanged(pos); }
          );
          mCabinets.add(cab);
          cab->name = i->name;
          cab->path = i->name;
        }
      });
      //String path = soundfoo;
      //path.appendPath("impulses");
      //ScanDir dir(path.get());
      //for (int i = 0; i < dir.size(); i++) {
      //  if (dir[i]->isFolder) {
      //    // We're only interested in folders, at the top level each corresponds to a cab
      //    Cabinet* cab = new Cabinet(
      //      [&](Cabinet* cab) { this->onCabChanged(cab); },
      //      [&](Microphone* mic) { this->onMicChanged(mic); },
      //      [&](MicPosition* pos) { this->onPositionChanged(pos); }
      //    );
      //    mCabinets.add(cab);
      //    cab->name = dir[i]->name.get();
      //    cab->path = dir[i]->relative.get();
      //    // dir.GetCurrentFullFN(&cab->path);
      //    cab->scanMics();
      //    mScrollView[0]->appendChild(cab);

      //  }
      //}
      //setFromIRBundle();
    }

    void onCabChanged(Cabinet* newCab) {
      if (newCab == mSelectedCab) { return; }
      for (int i = 0; i < mCabinets.size(); i++) {
        mCabinets[i]->mSelected = false;
      }
      if (newCab != nullptr) {
        newCab->mSelected = true;
        const int micIndex = mSelectedCab->mMics.find(mSelectedMic);
        mSelectedCab = newCab;
        mScrollView[1]->clearChildren();
        for (int i = 0; i < mSelectedCab->mMics.size(); i++) {
          mScrollView[1]->appendChild(mSelectedCab->mMics[i]);
        }
        if (newCab->mMics.size() > micIndex) {
          onMicChanged(newCab->mMics[micIndex]);
        }
        else {
          onMicChanged(newCab->mMics[0]);
        }

      }
      mScrollView[0]->SetDirty(false);
    }

    void onMicChanged(Microphone* mic) {
      if (mSelectedCab == nullptr || mic == mSelectedMic) { return; }
      for (int i = 0; i < mSelectedCab->mMics.size(); i++) {
        mSelectedCab->mMics[i]->mSelected = false;
      }
      int positionIndex = 0;
      if (mSelectedMic != nullptr) {
        positionIndex = mSelectedMic->mPositions.find(mSelectedPosition);
      }
      mSelectedMic = mic;
      mScrollView[2]->clearChildren();
      if (mic != nullptr) {
        mic->mSelected = true;
        for (int i = 0; i < mSelectedMic->mPositions.size(); i++) {
          mScrollView[2]->appendChild(mSelectedMic->mPositions[i]);
        }
        if (mic->mPositions.size() > positionIndex) {
          onPositionChanged(mic->mPositions[positionIndex]);
        }
        else {
          onPositionChanged(mic->mPositions[0]);
        }
      }
      mScrollView[1]->SetDirty(false);
    }

    void onPositionChanged(MicPosition* pos) {
      if (mSelectedMic == nullptr || pos == mSelectedPosition) { return; }
      mSelectedPosition = pos;
      if (pos != nullptr) {
        for (int i = 0; i < mSelectedMic->mPositions.size(); i++) {
          mSelectedMic->mPositions[i]->mSelected = false;
        }
        //pos->mSelected = true;
        //IRBundle load;
        //load.path.set(pos->path.get());
        //load.name.set(pos->name.get());
        //mCabShared->callback(load);
      }
      mScrollView[2]->SetDirty(false);
    }

    void setFromIRBundle() {
      for (int i = 0; i < mCabinets.size(); i++) {
        Cabinet* c = mCabinets[i];
        for (int j = 0; j < c->mMics.size(); j++) {
          Microphone* m = c->mMics[j];
          for (int k = 0; k < m->mPositions.size(); k++) {
            //MicPosition* p = m->mPositions[k];
            //String& path = mCabShared->loadedIr.path;
            //if (strncmp(p->path.get(), path.get(), path.getLength()) == 0) {
            //  onCabChanged(c);
            //  onMicChanged(m);
            //  onPositionChanged(p);
            //  return;
            //}
          }
        }
      }
    }

    void OnDetached() override {
      for (int i = 0; i < 3; i++) {
        GetUI()->RemoveControl(mScrollView[i]);
      }
      mCabinets.clear(true);
    }

    void OnResize() override {
      const IRECT dimensions = GetUI()->GetBounds().GetPadded(-10);
      mRECT = mTargetRECT = dimensions;
      mCloseButton = dimensions.GetFromRight(20).GetFromTop(20).GetTranslated(-20, 20);
      mPathTitle = dimensions.GetFromTop(60);
      mPathTitle.Pad(-20);
      IRECT list = dimensions.GetPadded(-10);
      list.T += 30;
      for (int i = 0; i < 3; i++) {
        mScrollView[i]->SetTargetAndDrawRECTs(list.SubRectHorizontal(3, i).GetPadded(-10));
      }
    }

    void Draw(IGraphics& g) override {
      g.FillRect(Theme::IRBrowser::BACKGROUND, mRECT);
      g.FillRect(Theme::Colors::ACCENT, mCloseButton);
      g.DrawText(Theme::IRBrowser::PATH, mPath.get(), mPathTitle);
    }

    void OnMouseDown(float x, float y, const IMouseMod& mod) override {
      const IRECT click(x, y, x, y);
      if (mCloseButton.Contains(click)) {
        GetUI()->RemoveControl(this);
      }
    }
  };
#endif
}
