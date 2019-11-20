#pragma once
#include "IControl.h"
#include "src/parameter/ParameterCoupling.h"
#include "src/node/NodeSocket.h"
#include "src/node/NodeSocketUi.h"
#include "src/misc/MessageBus.h"
#include "src/misc/GStructs.h"

class Node;

struct NodeUiParam {
  MessageBus::Bus* pBus;
  iplug::igraphics::IGraphics* pGraphics;
  float width;
  float height;
  float* X;
  float* Y;
  WDL_PtrList<ParameterCoupling>* pParameters;

  WDL_PtrList<NodeSocket>* inSockets;
  WDL_PtrList<NodeSocket>* outSockets;
  Node* node;
};

using namespace iplug;
using namespace igraphics;

struct NodeUiHeader {
  bool hasByPass = false;
  bool hasRemove = true;
  IControl* bypass;
  IControl* disconnect;
  IControl* remove;
};


/**
 * This class represents a Node on the UI, it's seperate to the node itself
 * since it will only exists as long as the UI window is open but is owned by the node
 */
class NodeUi : public IControl {
public:
  NodeUi(NodeUiParam pParam) :
    IControl(IRECT(0, 0, 0, 0), kNoParameter)
  {
    mDragging = false;
    X = pParam.X;
    Y = pParam.Y;
    mBus = pParam.pBus;
    mGraphics = pParam.pGraphics;
    mParameters = pParam.pParameters;
    mInSockets = pParam.inSockets;
    mOutSockets = pParam.outSockets;
    mParentNode = pParam.node;

    setUpDimensions(pParam.width, pParam.height);

    for (int i = 0; i < mParameters->GetSize(); i++) {
      /**
       * Keep them around in a map for convenient use
       * Only do this in the UI though
       */
      ParameterCoupling* p = mParameters->Get(i);
      mParamsByName.insert(pair<const char*, ParameterCoupling*>(p->name, p));
    }

    mNodeSpliceInEvent.subscribe(mBus, MessageBus::NodeSpliceIn, [&](NodeSpliceInPair pair) {
      if (mParentNode != pair.node) { return; }
      /**
       * Splice in only works on nodes with at least one in and output
       * Since there's no way to choose from multiple ones, the first ones will
       * always be used
       */
      NodeSocket* in = mInSockets->Get(0);
      NodeSocket* out = mOutSockets->Get(0);
      NodeSocket* prev = pair.socket->connectedTo;
      if (in != nullptr && out != nullptr) {
        pair.socket->disconnect();
        in->connect(prev);
        out->connect(pair.socket);
      }
    });

    mIconFont = ICONFONT;
  }

  ~NodeUi() {
  }

  virtual void setUpDimensions(float w, float h) {
    IRECT rect;
    rect.L = *X - w / 2;
    rect.R = *X + w / 2;
    rect.T = *Y - h / 2;
    rect.B = *Y + h / 2;
    SetTargetAndDrawRECTs(rect);
#ifdef NODESSHADOW
    // The RECT needs to be bigger to allow the shadow
    mRECT.Pad(NODESHADOWBOUNDS);
#endif
    
  }

  virtual void setColor(IColor c) {
    mUseSvgBg = false;
    mColor = c;
  }

  virtual void setSvg(const char* path) {
    mSvgBg = mGraphics->LoadSVG(path);
    if (mSvgBg.IsValid()) {
      mUseSvgBg = true;
    }
  }

  virtual void setUpHeader() {
    IRECT m = mTargetRECT;
    m.B = m.T + NODEHEADERSIZE;

    if (mHeader.hasByPass) {
      mHeader.bypass = new ITextToggleControl(IRECT(
        m.L + NODEHEADERBYPASSLEFT, m.T + NODEHEADERBYPASSTOP,
        m.L + NODEHEADERBYPASSLEFT + NODEHEADERBYPASSSIZE,
        m.T + NODEHEADERBYPASSTOP + NODEHEADERBYPASSSIZE
      ), [&](IControl* pCaller) {
        auto p = this->mParameters->Get(0);
        bool bypassed = static_cast<bool>(p->control->GetValue());
        p->control->SetValueFromUserInput(bypassed ? 0.0 : 1.0);
      }, u8"\uf056", u8"\uf011", mIconFont);
      mElements.Add(mHeader.bypass);
      mGraphics->AttachControl(mHeader.bypass);
    }


    mHeader.disconnect = new IVButtonControl(IRECT(
      m.R - NODEHEADERDISCONNECTRIGHT - NODEHEADERDISCONNECTSIZE,
      m.T + NODEHEADERDISCONNECTTOP, m.R - NODEHEADERDISCONNECTRIGHT,
      m.T + NODEHEADERDISCONNECTTOP + NODEHEADERDISCONNECTSIZE
    ), [&](IControl* pCaller) {
      MessageBus::fireEvent<Node*>(mBus, MessageBus::NodeDisconnectAll, this->mParentNode);
    });
    mElements.Add(mHeader.disconnect);
    mGraphics->AttachControl(mHeader.disconnect);

    mHeader.remove = new IVButtonControl(IRECT(
      m.R - NODEHEADERREMOVERIGHT - NODEHEADERDISCONNECTSIZE,
      m.T + NODEHEADERDISCONNECTTOP, m.R - NODEHEADERREMOVERIGHT,
      m.T + NODEHEADERDISCONNECTTOP + NODEHEADERDISCONNECTSIZE
    ), [&](IControl* pCaller) {
      MessageBus::fireEvent<Node*>(mBus, MessageBus::NodeDeleted, this->mParentNode);
    });
    mElements.Add(mHeader.remove);
    mGraphics->AttachControl(mHeader.remove);
  }

  virtual void setUpSockets() {
    for (int i = 0; i < mInSockets->GetSize(); i++) {
      NodeSocketUi* socket = new NodeSocketUi(
        mBus, mGraphics, mInSockets->Get(i), mTargetRECT.L, mTargetRECT.T + i * 50 + mTargetRECT.H() * 0.5
      );
      mGraphics->AttachControl(socket);
      mInSocketsUi.Add(socket);
      mElements.Add(socket);
    }

    for (int i = 0; i < mOutSockets->GetSize(); i++) {
      NodeSocketUi* socket = new NodeSocketUi(
        mBus, mGraphics, mOutSockets->Get(i), mTargetRECT.R - 30, mTargetRECT.T + i * 50 + mTargetRECT.H() * 0.5
      );
      mGraphics->AttachControl(socket);
      mOutSocketsUi.Add(socket);
      mElements.Add(socket);
    }
  }

  virtual void setUpControls() {
    for (int i = 0; i < mParameters->GetSize(); i++) {
      ParameterCoupling* couple = mParameters->Get(i);
      double value = *(couple->value);
      float px = *X + couple->x - (couple->w * 0.5f);
      float py = *Y + couple->y - (couple->h * 0.5f);
      IRECT controlPos(px, py, px + couple->w, py + couple->h);
      // use the daw parameter to sync the values if possible
      if (couple->parameterIdx != kNoParameter) {
        couple->parameter->Set(value);
        couple->control = new IVKnobControl(
          controlPos, couple->parameterIdx
        );
      }
      else {
        // use the callback to get the value to the dsp, won't allow automation though
        couple->control = new IVKnobControl(
          controlPos, [couple](IControl* pCaller) {
          // TODOG Add a label and handle nonlinear scalings according to the type
          couple->baseValue =
            (pCaller->GetValue() * (couple->max - couple->min)) + couple->min;
        }, couple->name, DEFAULT_STYLE, true, true
        );
      }
      couple->control->SetValue(
        (value - couple->min) / (couple->max - couple->min)
      );
      mGraphics->AttachControl(couple->control);
      mElements.Add(couple->control);
      if (i == 0 && mHeader.hasByPass) { couple->control->Hide(true); }

      // optinally hide the lables etc
      if (!couple->showLabel || !couple->showValue) {
        IVectorBase* vcontrol = dynamic_cast<IVectorBase*>(couple->control);
        if (vcontrol != nullptr) {
          vcontrol->SetShowLabel(couple->showLabel);
          vcontrol->SetShowValue(couple->showValue);
        }
      }

    }
  }

  virtual void setUp() {
    for (int i = 0; i < mParameters->GetSize(); i++) {
      ParameterCoupling* p = mParameters->Get(i);
      if (p->name == "Bypass") {
        mHeader.hasByPass = true;
        break;
      }
    }
    mElements.Add(this);
    setUpControls();
    setUpSockets();
    setUpHeader();
    translate(0, 0);
  }

  void cleanUp() {
    for (int i = 0; i < mParameters->GetSize(); i++) {
      ParameterCoupling* param = mParameters->Get(i);
      if (param->control != nullptr) {
        // this also destroys the object
        mGraphics->RemoveControl(param->control, true);
        param->control = nullptr;
      }
    }

    for (int i = 0; i < mInSocketsUi.GetSize(); i++) {
      mGraphics->RemoveControl(mInSocketsUi.Get(i), true);
    }

    for (int i = 0; i < mOutSocketsUi.GetSize(); i++) {
      mGraphics->RemoveControl(mOutSocketsUi.Get(i), true);
    }

    if (mHeader.hasByPass) {
      mGraphics->RemoveControl(mHeader.bypass, true);
    }
    mGraphics->RemoveControl(mHeader.disconnect, true);
    mGraphics->RemoveControl(mHeader.remove, true);
  }

  virtual void DrawHeader(IGraphics& g) {
    g.FillRect(IColor(255, NODEHEADERCOLOR), IRECT(mTargetRECT.L, mTargetRECT.T, mTargetRECT.R, mTargetRECT.T + NODEHEADERSIZE));
  }

  virtual void DrawBg(IGraphics& g) {
    if (mUseSvgBg) {
      g.DrawSVG(mSvgBg, mTargetRECT);
    }
    else {
#ifdef NODESROUNDEDCORNER
      g.FillRoundRect(mColor, mTargetRECT, NODESHADOWROUND);
#else
      g.FillRect(mColor, mTargetRECT);
#endif
    }
  }

  virtual void Draw(IGraphics& g) override {
#if defined(NODESSHADOW) || defined (NODESCACHEBG)
    if (!mBgIsCached) {
      g.StartLayer(this, mRECT);
#endif
      // This part will always run
      DrawBg(g);
      DrawHeader(g);
#if defined(NODESSHADOW) || defined(NODESCACHEBG)
      mCachedBgLayer = g.EndLayer();
#ifdef NODESSHADOW
      g.ApplyLayerDropShadow(mCachedBgLayer, IShadow(NODESHADOWCOLOR, NODESHADOWBLUR, NODESHADOWDIST, NODESHADOWDIST, 1.0, true));
#endif
      mBgIsCached = true;
    }
    g.DrawFittedLayer(mCachedBgLayer, mRECT, &mBlend);
#endif
  }

  virtual void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    if (mDragging) {
      mDragging = false;
      MessageBus::fireEvent<Node*>(mBus, MessageBus::NodeDraggedEnd, mParentNode);
      return;
    }
  }

  virtual void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override {
    mDragging = true;
    MessageBus::fireEvent<Coord2d>(mBus, MessageBus::NodeDragged, Coord2d {x, y});
    translate(dX, dY);
  }

  virtual void translate(float dX, float dY) {
    mNoScale = true;
    for (int i = 0; i < mElements.GetSize(); i++) {
      moveControl(mElements.Get(i), dX, dY);
    }

    for (int i = 0; i < mInSockets->GetSize(); i++) {
      mInSockets->Get(i)->X += dX;
      mInSockets->Get(i)->Y += dY;
    }

    for (int i = 0; i < mOutSockets->GetSize(); i++) {
      mOutSockets->Get(i)->X += dX;
      mOutSockets->Get(i)->Y += dY;
    }

    *X += dX;
    *Y += dY;

    mGraphics->SetAllControlsDirty();
  }

  virtual void OnResize() override {
    if (!mNoScale) {
      mBgIsCached = false;
    }
    mNoScale  = false;
  }

  void setTranslation(float x, float y) {
    float dX = x - *X;
    float dY = y - *Y;
    translate(dX, dY);
  }

  map<const char*, ParameterCoupling*> mParamsByName;


private:
  void moveControl(IControl* control, float x, float y) {
    IRECT rect = control->GetTargetRECT();
    rect.T += y;
    rect.L += x;
    rect.B += y;
    rect.R += x;
    control->SetTargetRECT(rect);
    rect = control->GetRECT();
    rect.T += y;
    rect.L += x;
    rect.B += y;
    rect.R += x;
    control->SetRECT(rect);
  }

protected:
  MessageBus::Bus* mBus;
  MessageBus::Subscription<NodeSpliceInPair> mNodeSpliceInEvent;
  WDL_PtrList<ParameterCoupling>* mParameters;
  WDL_PtrList<NodeSocket>* mInSockets;
  WDL_PtrList<NodeSocket>* mOutSockets;
  WDL_PtrList<NodeSocketUi> mInSocketsUi;
  WDL_PtrList<NodeSocketUi> mOutSocketsUi;

  NodeUiHeader mHeader;
  WDL_PtrList<IControl> mElements;
  Node* mParentNode;

  ILayerPtr mCachedBgLayer;
  bool mBgIsCached = false;
  bool mUseSvgBg = false;
  IBlend mBlend = { EBlend::Default, 1 };
  bool mNoScale = false;

  bool mDragging;
  float* X;
  float* Y;
  IColor mColor;
  ISVG mSvgBg = ISVG(nullptr);
  IGraphics* mGraphics;
  IText mIconFont;
};
