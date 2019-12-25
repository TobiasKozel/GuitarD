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

struct MicPosition {
  WDL_String name;
  WDL_String path;
};

struct Microphone {
  WDL_String name;
  WDL_String path;
  WDL_PtrList<MicPosition> mPositions;
  ~Microphone() {
    mPositions.Empty(true);
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
            MicPosition* pos = new MicPosition();
            pos->name.Append(f);
            if (strncmp(".wav", pos->name.get_fileext(), 5) == 0,
              strncmp(".WAV", pos->name.get_fileext(), 5) == 0)
            {
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
  WDL_String name;
  WDL_String path;
  WDL_PtrList<Microphone> mMics;

  Cabinet() : IControl({0, 0, 0, 20}) {
    
  }

  ~Cabinet() {
    mMics.Empty(true);
  }

  void Draw(IGraphics& g) override {
    g.FillRect(COLOR_WHITE, mRECT);
    g.DrawText(DEFAULT_TEXT, name.Get(), mRECT);
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
            Microphone* mic = new Microphone();
            mMics.Add(mic);
            mic->name.Append(f);
            dir.GetCurrentFullFN(&mic->path);
            mic->scanPositions();
          }
        }
      } while (!dir.Next());
    }
  }
};



class CabLibPopUp : public IControl {
  IRECT mCloseButton;
  ScrollViewControl* mScrollView[3] = { nullptr };
  WDL_PtrList<Cabinet> mCabinets;
public:
  CabLibPopUp() : IControl({}) {
    mRenderPriority = 15;
  }

  void OnInit() override {
    for (int i = 0; i < 3; i++) {
      mScrollView[i] = new ScrollViewControl();
      mScrollView[i]->setRenderPriority(16);
      mScrollView[i]->setFullWidthChildren(true);
      GetUI()->AttachControl(mScrollView[i]);
    }

    WDL_String iniFolder;
    INIPath(iniFolder, BUNDLE_NAME);
    iniFolder.Append(PATH_DELIMITER);
    iniFolder.Append("impulses");
    WDL_DirScan dir;
    WDBGMSG("Cabinets\n");
    if (!dir.First(iniFolder.Get())) {
      do {
        const char* f = dir.GetCurrentFN();
        // Skip hidden files and folders
        if (f && f[0] != '.') {
          if (dir.GetCurrentIsDirectory()) {
            // We're only interested in folders, at the top level each corresponds to a cab
            Cabinet* cab = new Cabinet();
            mCabinets.Add(cab);
            cab->name.Append(f);
            dir.GetCurrentFullFN(&cab->path);
            cab->scanMics();
            mScrollView[0]->appendChild(cab);
            WDBGMSG(cab->name.Get());
          }
        }
      } while (!dir.Next());
    }
    else {
      WDBGMSG("IR folder doesn't exist!\n");
    }
  }

  void OnDetach() override {
    for (int i = 0; i < 3; i++) {
      GetUI()->RemoveControl(mScrollView[i], true);
    }
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
      GetUI()->RemoveControl(this, true);
    }
  }
};
