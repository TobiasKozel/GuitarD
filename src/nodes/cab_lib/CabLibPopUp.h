#pragma once

#include "../../types/GPointerList.h"
#include "IControl.h"
#include "../../../thirdparty/soundwoofer/soundwoofer.h"
#include "../../ui/elements/scroll/ScrollViewControl.h"
#include "../../ui/GUIConfig.h"
#include <functional>

namespace guitard {

  class LibIr : public IControl {
  public:
    typedef std::function<void(LibIr * c)> IrCallback;
    bool mSelected = false;
    IrCallback mCallback;
    soundwoofer::SWImpulseShared mIr;
    String mName;
    LibIr(soundwoofer::SWImpulseShared ir, IrCallback callback) : IControl({ 0, 0, 0, 20 }) {
      mCallback = callback;
      mIr = ir;
      mName = ir->name;
    }

    void Draw(IGraphics& g) override {
      if (mSelected) {
        g.FillRect(Theme::IRBrowser::IR_TITLE_BG_ACTIVE, mRECT);
        g.DrawText(Theme::IRBrowser::IR_TITLE_ACTIVE, mName.c_str(), mRECT.GetHPadded(-8));
      }
      else {
        if (mMouseIsOver) {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG_HOVER, mRECT);
        }
        else {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG, mRECT);
        }
        g.DrawText(Theme::IRBrowser::IR_TITLE, mName.c_str(), mRECT.GetHPadded(-8));
      }
    }

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
      mCallback(this);
    }

    
  };

  class LibMic : public IControl {
  public:
    typedef std::function<void(LibMic * c)> MicCallback;
    MicCallback mCallback;
    soundwoofer::SWComponentShared mMic;
    String mName;
    bool mSelected = false;
    LibMic(soundwoofer::SWComponentShared mic,  MicCallback mc) : IControl({ 0, 0, 0, 20 }) {
      mCallback = mc;
      mMic = mic;
      mName = mMic->name;
    }
    ~LibMic() { }

    void Draw(IGraphics& g) override {
      if (mSelected) {
        g.FillRect(Theme::IRBrowser::IR_TITLE_BG_ACTIVE, mRECT);
        g.DrawText(Theme::IRBrowser::IR_TITLE_ACTIVE, mName.c_str(), mRECT.GetHPadded(-8));
      }
      else {
        if (mMouseIsOver) {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG_HOVER, mRECT);
        }
        else {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG, mRECT);
        }
        g.DrawText(Theme::IRBrowser::IR_TITLE, mName.c_str(), mRECT.GetHPadded(-8));
      }
    }

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
      mCallback(this);
    }
  };

  class LibRig : public IControl {
  public:
    typedef std::function<void(LibRig* c)> RigCallback;
    RigCallback mCallback;
    soundwoofer::SWRigShared mRig;
    bool mSelected = false;
    String mName;
    PointerList<LibMic> mMics;
    PointerList<LibIr> mIrs;
    

    LibRig(soundwoofer::SWRigShared rig, const RigCallback cc) : IControl({ 0, 0, 0, 20 }) {
      mCallback = cc;
      mRig = rig;
      mName = rig->name;
    }

    ~LibRig() { }

    void Draw(IGraphics& g) override {
      if (mSelected) {
        g.FillRect(Theme::IRBrowser::IR_TITLE_BG_ACTIVE, mRECT);
        g.DrawText(Theme::IRBrowser::IR_TITLE_ACTIVE, mName.c_str(), mRECT.GetHPadded(-8));
      }
      else {
        if (mMouseIsOver) {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG_HOVER, mRECT);
        }
        else {
          g.FillRect(Theme::IRBrowser::IR_TITLE_BG, mRECT);
        }
        g.DrawText(Theme::IRBrowser::IR_TITLE, mName.c_str(), mRECT.GetHPadded(-8));
      }
    }

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
      mCallback(this);
    }
  };


  class CabLibPopUp : public IControl {
    CabLibNode* mNode = nullptr;
    IRECT mCloseButton;
    IRECT mPathTitle;
    ScrollViewControl* mScrollView[3] = { nullptr };
    PointerList<LibRig> mRigs;
    PointerList<LibMic> mMics;
    PointerList<LibIr> mIrs;

    LibRig* mSelectedRig = nullptr;
    LibMic* mSelectedMic = nullptr;
    LibIr* mSelectedIr = nullptr;
    String mPath;
    bool mLoading = false;

    void changeIr(LibIr* newIr) {
      LibIr* prevIr = mSelectedIr;
      mSelectedIr = nullptr;

      if (newIr == nullptr) {
        LibIr* first = nullptr;
        for (int i = 0; i < mSelectedRig->mIrs.size(); i++) {
          LibIr* c = mSelectedRig->mIrs[i];
          if (mSelectedMic->mMic->id == c->mIr->micId) { // Look if it matches the mic
            if (first == nullptr) { first = c; } // save the first one in case we don't find a match
            if (prevIr != nullptr && prevIr->mIr->name == c->mIr->name) { // look if it matches the last ir
              mSelectedIr = c;
            }
          }
        }

        // Use the first one if no match is found
        if (mSelectedIr == nullptr && first != nullptr) {
          mSelectedIr = first;
        }
      }
      else {
        mSelectedIr = newIr;
      }


      if (prevIr != nullptr) {
        prevIr->mSelected = false;
      }

      if (mSelectedIr == nullptr) {
        mScrollView[2]->SetDirty();
        return;
      }

      mSelectedIr->mSelected = true;
      mScrollView[2]->SetDirty();
      mNode->loadIr(mSelectedIr->mIr);
    }

    void changeMic(LibMic* newMic) {
      mScrollView[2]->clearChildren(); // Clear out old irs
      LibMic* prevMic = mSelectedMic;
      mSelectedMic = nullptr;

      if (newMic == nullptr) {
        // Try to match the old mic
        if (prevMic != nullptr) {
          
          for (int i = 0; i < mSelectedRig->mMics.size(); i++) {
            if (prevMic->mMic->id == mSelectedRig->mMics[i]->mMic->id) {
              mSelectedMic = mSelectedRig->mMics[i]; // Found a match
              break;
            }
          }
        }

        // Use the first one if there's no match
        if (mSelectedMic == nullptr && mSelectedRig->mMics.size()) {
          mSelectedMic = mSelectedRig->mMics[0];
        }
      }
      else {
        mSelectedMic = newMic;
      }

      if (prevMic != nullptr ) {
        prevMic->mSelected = false;
      }

      if (mSelectedMic == nullptr) {
        mScrollView[1]->SetDirty();
        return;
      }
      mSelectedMic->mSelected = true;

      for (int i = 0; i < mSelectedRig->mIrs.size(); i++) {
        // Add the new Irs which match the micid
        if (mSelectedMic->mMic->id == mSelectedRig->mIrs[i]->mIr->micId) {
          mScrollView[2]->appendChild(mSelectedRig->mIrs[i]);
        }
      }

      mScrollView[1]->SetDirty();
      changeIr(nullptr);
    }

    void changeRig(LibRig* newRig) {
      mScrollView[1]->clearChildren(); // Clear out old mics
      if (mSelectedRig != nullptr) {
        mSelectedRig->mSelected = false;
      }
      mSelectedRig = newRig;
      mScrollView[0]->SetDirty();
      if (mSelectedRig == nullptr) { return; }
      for (int i = 0; i < newRig->mMics.size(); i++) {
        // Add the new ones
        mScrollView[1]->appendChild(mSelectedRig->mMics[i]);
      }
      mSelectedRig->mSelected = true;
      changeMic(nullptr);
    }

    soundwoofer::async::Callback mCallback = std::make_shared<soundwoofer::async::CallbackFunc>(
      [&](soundwoofer::Status status) {
        // if (status != soundwoofer::SUCCESS) { return; }
        auto rigs = soundwoofer::ir::getRig();
        for (auto& i : rigs) { // Build the rigs
          LibRig* rig = new LibRig(i, [&](LibRig* callbackRig) {
            changeRig(callbackRig);
          });
          mScrollView[0]->appendChild(rig);
          mRigs.add(rig);

          for (auto& j : i->microphones) { // Build the mics
            if (j->type != "Microphone") { continue; } // Only Mics
            LibMic* duplicate = nullptr;
            for (int k = 0; k < mMics.size(); k++) {
              if (mMics[k]->mMic->id == j->id) {
                duplicate = mMics[k];
                break;
              }
            }

            if (duplicate != nullptr) {
              rig->mMics.add(duplicate);
              continue;
            } // No dupes
            LibMic* mic = new LibMic(j, [&](LibMic* callbackMic) {
              changeMic(callbackMic);
            });
            mMics.add(mic);
            rig->mMics.add(mic);
          }

          for (auto& j : i->impulses) {
            LibIr* ir = new LibIr(j, [&](LibIr* callbackIr) {
              changeIr(callbackIr);
            });
            mIrs.add(ir);
            rig->mIrs.add(ir);
          }
        }

        for (int r = 0; r < mRigs.size(); r++) {
          LibRig* rig = mRigs[r];
          for (int i = 0; i < rig->mIrs.size(); i++) {
            LibIr* ir = rig->mIrs[i];
            if (ir->mIr->file == mNode->mLoadedIr->file) { // Found the ir
              for (int m = 0; m < rig->mMics.size(); m++) {
                LibMic* mic = rig->mMics[m];
                if (ir->mIr->micId == mic->mMic->id) { // Found the mic
                  changeRig(rig);
                  changeMic(mic);
                  changeIr(ir);
                }
              }
            }
          }
        }
        mPath = soundwoofer::state::irDirectory;
        GetUI()->SetAllControlsDirty();
      }
    );

  public:
    CabLibPopUp(CabLibNode* node) : IControl({}) {
      mRenderPriority = 15;
      mNode = node;
    }

    void OnInit() override {
      for (int i = 0; i < 3; i++) { // Create three scroll views
        mScrollView[i] = new ScrollViewControl();
        mScrollView[i]->SetRenderPriority(16);
        mScrollView[i]->setFullWidthChildren(true);
        mScrollView[i]->setChildPadding(2);
        mScrollView[i]->setCleanUpEnabled(false);
        GetUI()->AttachControl(mScrollView[i]);
      }
      mPath = "Loading...";
      soundwoofer::async::ir::list(mCallback);
    }

    void OnDetached() override {
      for (int i = 0; i < 3; i++) {
        GetUI()->RemoveControl(mScrollView[i]);
      }
      mRigs.clear(true);
      mMics.clear(true);
      mIrs.clear(true);
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
      g.DrawText(Theme::IRBrowser::PATH, mPath.c_str(), mPathTitle);
    }

    void OnMouseDown(float x, float y, const IMouseMod& mod) override {
      const IRECT click(x, y, x, y);
      if (mCloseButton.Contains(click)) {
        GetUI()->RemoveControl(this);
      }
      if (mPathTitle.Contains(click)) {
        WDL_String path;
        path.Set(mPath.c_str());
        path.remove_trailing_dirchars();
        GetUI()->RevealPathInExplorerOrFinder(path);
      }
    }
  };
}
