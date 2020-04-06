#pragma once
#include <string>

namespace guitard {
  class WrappedConvolver;
  class ParameterManager;
  class Node;
  class NodeUi;
  class Graph;
  struct NodeSocket;

  typedef
#ifdef SAMPLE_TYPE_FLOAT
  float
#else
  double
#endif
  sample;

  typedef std::string String;

  static const int kNoParameter = -1;
  static const int kNoValIdx = -1;
  static const double PI = 3.14159265358979323846;
}


// Wrap the debug message from iplug in the iplug namespace on windows
// since using iplug::DBGMSG will exapnd to iplug::printf() and fail on linux/osx
#if defined (GUITARD_HEADLESS)
  #ifdef NDEBUG
    #define WDBGMSG(...) printf(__VA_ARGS__);
  #else
    #define WDBGMSG(...)
  #endif
#else
  #include "IPlugLogger.h"
  #if defined(OS_MAC) || defined(OS_LINUX) || defined(OS_WEB) || defined(OS_IOS) || NDEBUG
    #define WDBGMSG(...) DBGMSG(__VA_ARGS__)
  #elif defined OS_WIN
    #define WDBGMSG(...) iplug::DBGMSG(__VA_ARGS__)
  #endif
#endif

/**
 * A macro to disable all kinds of implicit copy mechanisms
 */
#define GUITARD_NO_COPY(name) \
name(const name&) = delete; \
name(const name*) = delete; \
name(name&&) = delete; \
name& operator= (const name&) = delete; \
name& operator= (name&&) = delete;

/**
 * This is one of the downsides of going header only
 * using namespace will break almost everything,
 * so the most used IPlug types will be typedef'd here
 */
#ifndef GUITARD_HEADLESS
#include "IControls.h"

namespace guitard {
  typedef iplug::IParam IParam;
  typedef iplug::igraphics::IControl IControl;
  typedef iplug::igraphics::IKnobControlBase  IKnobControlBase;
  typedef iplug::igraphics::IVButtonControl IVButtonControl;
  typedef iplug::igraphics::ITextToggleControl ITextToggleControl;
  typedef iplug::igraphics::IGraphics IGraphics;
  typedef iplug::igraphics::IRECT IRECT;
  typedef iplug::igraphics::IColor IColor;
  typedef iplug::igraphics::IText IText;
  typedef iplug::igraphics::EAlign EAlign;
  typedef iplug::igraphics::EVAlign EVAlign;
  typedef iplug::igraphics::IVKnobControl IVKnobControl;
  typedef iplug::igraphics::IVectorBase IVectorBase;
  typedef iplug::igraphics::ISVG ISVG;
  typedef iplug::igraphics::IBitmap IBitmap;
  typedef iplug::igraphics::ILayerPtr ILayerPtr;
  typedef iplug::igraphics::IBlend IBlend;
  typedef iplug::igraphics::EBlend EBlend;
  typedef iplug::igraphics::EDirection EDirection;
  typedef iplug::igraphics::IMouseMod IMouseMod;
  typedef iplug::igraphics::IVStyle IVStyle;
  typedef iplug::IKeyPress IKeyPress;
  typedef iplug::igraphics::IPopupMenu IPopupMenu;
  const IVStyle DEFAULT_STYLE = IVStyle();
}
#endif