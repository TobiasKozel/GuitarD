#pragma once

#include "IControl.h"
#include "src/ui/ScrollViewControl.h"
#include "src/ui/theme.h"

// avoid some UNICODE issues with VST3 SDK and WDL dirscan
#if defined VST3_API && defined OS_WIN
#ifdef FindFirstFile
#undef FindFirstFile
#undef FindNextFile
#undef WIN32_FIND_DATA
#undef PWIN32_FIND_DATA
#define FindFirstFile FindFirstFileA
#define FindNextFile FindNextFileA
#define WIN32_FIND_DATA WIN32_FIND_DATAA
#define LPWIN32_FIND_DATA LPWIN32_FIND_DATAA
#endif
#endif

#include "dirscan.h"
#include "IPlugPaths.h"

using namespace iplug;
using namespace igraphics;

class MicPosition : public IControl {
public:
  typedef std::function<void(MicPosition * c)> MicPositionCallback;
  bool mSelected = false;
  MicPositionCallback mCallback;
  MicPosition(MicPositionCallback callback) : IControl({ 0, 0, 0, 20 }) {
    mCallback = callback;
  }

  void Draw(IGraphics& g) override {
    if (mSelected) {
      g.FillRect(COLOR_ORANGE, mRECT);
      g.DrawText(DEFAULT_TEXT, name.Get(), mRECT);
    }
    else {
      g.FillRect(COLOR_WHITE, mRECT);
      g.DrawText(DEFAULT_TEXT, name.Get(), mRECT);
    }
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    mCallback(this);
  }

  WDL_String name;
  WDL_String path;
};

class Microphone : public IControl {
public:
  typedef std::function<void(Microphone * c)> MicrophoneCallback;
  MicrophoneCallback mCallback;
  WDL_String name;
  WDL_String path;
  WDL_PtrList<MicPosition> mPositions;
  MicPosition::MicPositionCallback mPosCallback;
  bool mSelected = false;
  Microphone(MicrophoneCallback mc, MicPosition::MicPositionCallback pc) : IControl({ 0, 0, 0, 20 }) {
    mCallback = mc;
    mPosCallback = pc;
  }
  ~Microphone() {
    mPositions.Empty(true);
  }

  void Draw(IGraphics& g) override {
    if (mSelected) {
      g.FillRect(COLOR_ORANGE, mRECT);
      g.DrawText(DEFAULT_TEXT, name.Get(), mRECT);
    }
    else {
      g.FillRect(COLOR_WHITE, mRECT);
      g.DrawText(DEFAULT_TEXT, name.Get(), mRECT);
    }
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    mCallback(this);
  }

  void scanPositions() {
    WDL_DirScan dir;
    if (!dir.First(path.Get())) {
      do {
        const char* f = dir.GetCurrentFN();
        // Skip hidden files and folders
        if (f && f[0] != '.') {
          if (!dir.GetCurrentIsDirectory()) {
            // We're only interested in files, each corresponds to a IR
            MicPosition* pos = new MicPosition(mPosCallback);
            pos->name.Append(f);
            if (strncmp(".wav", pos->name.get_fileext(), 5) == 0 ||
                strncmp(".WAV", pos->name.get_fileext(), 5) == 0) {
              dir.GetCurrentFullFN(&pos->path);
              mPositions.Add(pos);
            }
            else {
              delete pos;
            }
          }
        }
      } while (!dir.Next());
    }
  }
};

class Cabinet : public IControl {
public:
  typedef std::function<void(Cabinet* c)> CabinetCallback;
  WDL_String name;
  WDL_String path;
  CabinetCallback mCallback;
  Microphone::MicrophoneCallback mMicCallback;
  MicPosition::MicPositionCallback mPosCallback;
  WDL_PtrList<Microphone> mMics;
  bool mSelected = false;

  Cabinet(const CabinetCallback cc, const Microphone::MicrophoneCallback mc, MicPosition::MicPositionCallback pc)
    : IControl({ 0, 0, 0, 20 })
  {
    mCallback = cc;
    mMicCallback = mc;
    mPosCallback = pc;
  }

  ~Cabinet() {
    mMics.Empty(true);
  }

  void Draw(IGraphics& g) override {
    if (mSelected) {
      g.FillRect(COLOR_ORANGE, mRECT);
      g.DrawText(DEFAULT_TEXT, name.Get(), mRECT);
    }
    else {
      g.FillRect(COLOR_WHITE, mRECT);
      g.DrawText(DEFAULT_TEXT, name.Get(), mRECT);
    }
  }

  void scanMics() {
    WDL_DirScan dir;
    if (!dir.First(path.Get())) {
      do {
        const char* f = dir.GetCurrentFN();
        // Skip hidden files and folders
        if (f && f[0] != '.') {
          if (dir.GetCurrentIsDirectory()) {
            // We're only interested in folders, each corresponds to a mic
            Microphone* mic = new Microphone(mMicCallback, mPosCallback);
            mMics.Add(mic);
            mic->name.Append(f);
            dir.GetCurrentFullFN(&mic->path);
            mic->scanPositions();
          }
        }
      } while (!dir.Next());
    }
  }

  void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    mCallback(this);
  }
};



class CabLibPopUp : public IControl {
  IRECT mCloseButton;
  ScrollViewControl* mScrollView[3] = { nullptr };
  WDL_PtrList<Cabinet> mCabinets;
  Cabinet* mSelectedCab = nullptr;
  Microphone* mSelectedMic = nullptr;
  MicPosition* mSelectedPosition = nullptr;
public:
  CabLibPopUp() : IControl({}) {
    mRenderPriority = 15;
  }

  void OnInit() override {
    for (int i = 0; i < 3; i++) {
      mScrollView[i] = new ScrollViewControl();
      mScrollView[i]->setRenderPriority(16);
      mScrollView[i]->setFullWidthChildren(true);
      mScrollView[i]->setChildPadding(1);
      mScrollView[i]->setCleanUpEnabled(false);
      GetUI()->AttachControl(mScrollView[i]);
    }

    WDL_String iniFolder;
    INIPath(iniFolder, BUNDLE_NAME);
    iniFolder.Append(PATH_DELIMITER);
    iniFolder.Append("impulses");
    WDL_DirScan dir;
    if (!dir.First(iniFolder.Get())) {
      do {
        const char* f = dir.GetCurrentFN();
        // Skip hidden files and folders
        if (f && f[0] != '.') {
          if (dir.GetCurrentIsDirectory()) {
            // We're only interested in folders, at the top level each corresponds to a cab
            Cabinet* cab = new Cabinet(
              [&](Cabinet* cab) { this->onCabChanged(cab); },
              [&](Microphone* mic) { this->onMicChanged(mic); },
              [&](MicPosition* pos) { this->onPositionChanged(pos); }
            );
            mCabinets.Add(cab);
            cab->name.Append(f);
            dir.GetCurrentFullFN(&cab->path);
            cab->scanMics();
            mScrollView[0]->appendChild(cab);
          }
        }
      } while (!dir.Next());
    }
    else {
      WDBGMSG("IR folder doesn't exist!\n");
    }
  }

  void onCabChanged(Cabinet* newCab) {
    if (newCab == mSelectedCab) { return; }
    for (int i = 0; i < mCabinets.GetSize(); i++) {
      mCabinets.Get(i)->mSelected = false;
    }
    if (newCab != nullptr) {
      newCab->mSelected = true;
      const int micIndex = mSelectedCab->mMics.Find(mSelectedMic);
      mSelectedCab = newCab;
      mScrollView[1]->clearChildren();
      for (int i = 0; i < mSelectedCab->mMics.GetSize(); i++) {
        mScrollView[1]->appendChild(mSelectedCab->mMics.Get(i));
      }
      if (newCab->mMics.GetSize() > micIndex) {
        onMicChanged(newCab->mMics.Get(micIndex));
      }
      else {
        onMicChanged(newCab->mMics.Get(0));
      }
       
    }
    mScrollView[0]->SetDirty(false);
  }

  void onMicChanged(Microphone* mic) {
    if (mSelectedCab == nullptr || mic == mSelectedMic) { return; }
    for (int i = 0; i < mSelectedCab->mMics.GetSize(); i++) {
      mSelectedCab->mMics.Get(i)->mSelected = false;
    }
    int positionIndex = 0;
    if (mSelectedMic != nullptr) {
      positionIndex = mSelectedMic->mPositions.Find(mSelectedPosition);
    }
    mSelectedMic = mic;
    mScrollView[2]->clearChildren();
    if (mic != nullptr) {
      mic->mSelected = true;
      for (int i = 0; i < mSelectedMic->mPositions.GetSize(); i++) {
        mScrollView[2]->appendChild(mSelectedMic->mPositions.Get(i));
      }
      if (mic->mPositions.GetSize() > positionIndex) {
        onPositionChanged(mic->mPositions.Get(positionIndex));
      }
      else {
        onPositionChanged(mic->mPositions.Get(0));
      }
    }
    mScrollView[1]->SetDirty(false);
  }

  void onPositionChanged(MicPosition* pos) {
    if (mSelectedMic == nullptr|| pos == mSelectedPosition) { return; }
    mSelectedPosition = pos;
    if (pos != nullptr) {
      for (int i = 0; i < mSelectedMic->mPositions.GetSize(); i++) {
        mSelectedMic->mPositions.Get(i)->mSelected = false;
      }
      pos->mSelected = true;
    }
    mScrollView[2]->SetDirty(false);
  }

  void OnDetach() override {

  }

  void OnResize() override {
    const IRECT dimensions = GetUI()->GetBounds().GetPadded(-10);
    mRECT = mTargetRECT = dimensions;
    mCloseButton = mRECT.GetFromRight(50).GetFromTop(50).GetTranslated(-20, 20);
    IRECT list = dimensions.GetPadded(-10);
    list.T += 60;
    for (int i = 0; i < 3; i++) {
      mScrollView[i]->SetTargetAndDrawRECTs(list.SubRectHorizontal(3, i).GetPadded(-10));
    }
  }

  void Draw(IGraphics& g) override {
    g.FillRect(Theme::Gallery::CATEGORY_BG, mRECT);
    g.FillRect(Theme::Colors::ACCENT, mCloseButton);
  }

  void OnMouseDown(float x, float y, const IMouseMod& mod) override {
    const IRECT click(x, y, x, y);
    if (mCloseButton.Contains(click)) {
      for (int i = 0; i < 3; i++) {
        GetUI()->RemoveControl(mScrollView[i], true);
      }
      mCabinets.Empty(true);
      GetUI()->RemoveControl(this, true);
    }
  }
};
