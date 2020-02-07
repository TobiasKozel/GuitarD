#pragma once

/**
 * This is one of the downsides of going header only
 * using namespace will break almost everything,
 * so the most used IPlug types will be typedef'd here
 */
#ifndef GUITARD_HEADLESS

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