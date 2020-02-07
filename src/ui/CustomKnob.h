#pragma once

#include "IControl.h"
#include "src/parameter/ParameterCoupling.h"
#include "src/types/types.h"
namespace guitard {
  class CustomKnob : public IKnobControlBase {
    ParameterCoupling* mCoupling = nullptr;
    IRECT mWidgetBounds; // The knob/slider/button
    IRECT mLabelBounds; // A piece of text above the control
    IRECT mValueBounds; // Text below the contol, usually displaying the value of a parameter

    WDL_String mLabelStr;
    WDL_String mValueStr;
  public:
    CustomKnob(IRECT rect, ParameterCoupling* couple) :
      IKnobControlBase(rect, couple->parameterIdx, EDirection::Vertical, iplug::igraphics::DEFAULT_GEARING)
    {
      mCoupling = couple;
    }

    void Draw(IGraphics& g) override {
    }
    // virtual void DrawWidget(IGraphics& g) override;

    void OnMouseDown(float x, float y, const IMouseMod& mod) override {
      //if (mStyle.showValue && mValueBounds.Contains(x, y)) {
      //  PromptUserInput(mValueBounds);
      //}
      //else
      {
        if (true) {
          GetUI()->HideMouseCursor(true, true);
        }
        IKnobControlBase::OnMouseDown(x, y, mod);
      }
    }

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
      //if (mStyle.hideCursor) {
      //  GetUI()->HideMouseCursor(false);
      //}

      IKnobControlBase::OnMouseUp(x, y, mod);

      SetDirty(true);
    }

    void OnMouseOver(float x, float y, const IMouseMod& mod) override {
      //if (mStyle.showValue && !mDisablePrompt) {
      //  mValueMouseOver = mValueBounds.Contains(x, y);
      //}

      IKnobControlBase::OnMouseOver(x, y, mod);
    }

    void OnMouseOut() override {
      mValueMouseOver = false;
      IKnobControlBase::OnMouseOut();
    }

    //  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {  OnMouseDown(x, y, mod); }
    void OnResize() override {
      SetTargetRECT(MakeRects(mRECT));
      SetDirty(false);
    }

    bool IsHit(float x, float y) const override {
      //if (!mDisablePrompt) {
      //  if (mValueBounds.Contains(x, y))
      //    return true;
      //}

      //return mWidgetBounds.Contains(x, y);
    }

    void SetDirty(bool push, int valIdx = kNoValIdx) override {
      IKnobControlBase::SetDirty(push);

      const IParam* pParam = GetParam();

      if (pParam) {
        // pParam->GetDisplayForHostWithLabel(mValueStr);
      }
    }
    void OnInit() override {
      const IParam* pParam = GetParam();

      if (pParam) {
        pParam->GetDisplayForHostWithLabel(mValueStr);

        if (!mLabelStr.GetLength()) {
          mLabelStr.Set(pParam->GetNameForHost());
        }
      }
    }

    IRECT MakeRects(const IRECT& parent, bool hasHandle = false) {
      IRECT clickableArea = parent;
      const bool mLabelInWidget = false;
      const bool mValueInWidget = true;
      mLabelBounds = IRECT();

      //mWidgetBounds = clickableArea.GetScaledAboutCentre(mStyle.widgetFrac);


      //if (hasHandle) {
      //  mWidgetBounds = GetAdjustedHandleBounds(clickableArea).GetScaledAboutCentre(mStyle.widgetFrac);
      //}

      if (mValueInWidget) {
        mValueBounds = mWidgetBounds;
      }

      return clickableArea;
    }

  protected:
    bool mValueMouseOver = false;
  };
}