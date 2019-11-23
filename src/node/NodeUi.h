#pragma once
#include "IControl.h"
#include "src/node/NodeSocket.h"
#include "src/node/NodeSocketUi.h"
#include "src/misc/MessageBus.h"
#include "src/misc/GStructs.h"
#include "NodeShared.h"

class Node;

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
protected:
  NodeShared* shared;

  MessageBus::Subscription<NodeSpliceInPair> mNodeSpliceInEvent;

  WDL_PtrList<NodeSocketUi> mInSocketsUi;
  WDL_PtrList<NodeSocketUi> mOutSocketsUi;

  NodeUiHeader mHeader;
  WDL_PtrList<IControl> mElements;

  ILayerPtr mCachedBgLayer;
  bool mBgIsCached = false;
  bool mUseSvgBg = false;
  IBlend mBlend = { EBlend::Default, 1 };
  bool mNoScale = false;

  bool mDragging;
  IColor mColor;
  ISVG mSvgBg = ISVG(nullptr);
  IText mIconFont;

public:
  explicit NodeUi(NodeShared* pShared) :
    IControl(IRECT(0, 0, 0, 0), kNoParameter)
  {
    mDragging = false;
    shared = pShared;

    NodeUi::setUpDimensions(shared->width, shared->height);

    for (int i = 0; i < shared->parameters.GetSize(); i++) {
      /**
       * Keep them around in a map for convenient use
       * Only do this in the UI though
       */
      ParameterCoupling* p = shared->parameters.Get(i);
      mParamsByName.insert(pair<const char*, ParameterCoupling*>(p->name, p));
    }

    mNodeSpliceInEvent.subscribe(shared->bus, MessageBus::NodeSpliceIn, [&](NodeSpliceInPair pair) {
      if (shared->node != pair.node) { return; }
      /**
       * Splice in only works on nodes with at least one in and output
       * Since there's no way to choose from multiple ones, the first ones will
       * always be used
       */
      NodeSocket* in = shared->socketsIn.Get(0);
      NodeSocket* out = shared->socketsOut.Get(0);
      NodeSocket* prev = pair.socket->mConnectedTo;
      if (in != nullptr && out != nullptr) {
        pair.socket->disconnect();
        in->connect(prev);
        out->connect(pair.socket);
      }
    });

    mIconFont = ICON_FONT;
  }

  virtual ~NodeUi() {
  }

  virtual void setUpDimensions(float w, float h) {
    IRECT rect;
    rect.L = shared->X - w / 2;
    rect.R = shared->X + w / 2;
    rect.T = shared->Y - h / 2;
    rect.B = shared->Y + h / 2;
    SetTargetAndDrawRECTs(rect);
#ifdef NODE_SHADOW
    // The RECT needs to be bigger to allow the shadow
    mRECT.Pad(Theme::Node::SHADOW_BOUNDS);
#endif
    
  }

  virtual void setColor(IColor c) {
    mUseSvgBg = false;
    mColor = c;
  }

  virtual void setSvg(const char* path) {
    mSvgBg = shared->graphics->LoadSVG(path);
    if (mSvgBg.IsValid()) {
      mUseSvgBg = true;
    }
  }

  virtual void setUpHeader() {
    IRECT m = mTargetRECT;
    m.B = m.T + Theme::Node::HEADER_SIZE;

    if (mHeader.hasByPass) {
      mHeader.bypass = new ITextToggleControl(IRECT(
        m.L + Theme::Node::HEADER_BYPASS_LEFT, m.T + Theme::Node::HEADER_BYPASS_TOP,
        m.L + Theme::Node::HEADER_BYPASS_LEFT + Theme::Node::HEADER_BYPASS_SIZE,
        m.T + Theme::Node::HEADER_BYPASS_TOP + Theme::Node::HEADER_BYPASS_SIZE
      ), [&](IControl* pCaller) {
        auto p = this->shared->parameters.Get(0);
        const bool bypassed = static_cast<bool>(p->control->GetValue());
        p->control->SetValueFromUserInput(bypassed ? 0.0 : 1.0);
      }, u8"\uf056", u8"\uf011", mIconFont);
      mElements.Add(mHeader.bypass);
      shared->graphics->AttachControl(mHeader.bypass);
    }


    mHeader.disconnect = new IVButtonControl(IRECT(
      m.R - Theme::Node::HEADER_DISCONNECT_RIGHT - Theme::Node::HEADER_DISCONNECT_SIZE,
      m.T + Theme::Node::HEADER_DISCONNECT_TOP, m.R - Theme::Node::HEADER_DISCONNECT_RIGHT,
      m.T + Theme::Node::HEADER_DISCONNECT_TOP + Theme::Node::HEADER_DISCONNECT_SIZE
    ), [&](IControl* pCaller) {
      MessageBus::fireEvent<Node*>(shared->bus, MessageBus::NodeDisconnectAll, this->shared->node);
    });
    mElements.Add(mHeader.disconnect);
    shared->graphics->AttachControl(mHeader.disconnect);

    mHeader.remove = new IVButtonControl(IRECT(
      m.R - Theme::Node::HEADER_REMOVE_RIGHT - Theme::Node::HEADER_DISCONNECT_SIZE,
      m.T + Theme::Node::HEADER_DISCONNECT_TOP, m.R - Theme::Node::HEADER_REMOVE_RIGHT,
      m.T + Theme::Node::HEADER_DISCONNECT_TOP + Theme::Node::HEADER_DISCONNECT_SIZE
    ), [&](IControl* pCaller) {
      MessageBus::fireEvent<Node*>(shared->bus, MessageBus::NodeDeleted, this->shared->node);
    });
    mElements.Add(mHeader.remove);
    shared->graphics->AttachControl(mHeader.remove);
  }

  virtual void setUpSockets() {
    for (int i = 0; i < shared->socketsIn.GetSize(); i++) {
      NodeSocketUi* socket = new NodeSocketUi(
        shared->bus, shared->graphics, shared->socketsIn.Get(i), mTargetRECT.L, mTargetRECT.T + i * 50.f + mTargetRECT.H() * 0.5f
      );
      shared->graphics->AttachControl(socket);
      mInSocketsUi.Add(socket);
      mElements.Add(socket);
    }

    for (int i = 0; i < shared->socketsOut.GetSize(); i++) {
      NodeSocketUi* socket = new NodeSocketUi(
        shared->bus, shared->graphics, shared->socketsOut.Get(i), mTargetRECT.R - 30, mTargetRECT.T + i * 50.f + mTargetRECT.H() * 0.5f
      );
      shared->graphics->AttachControl(socket);
      mOutSocketsUi.Add(socket);
      mElements.Add(socket);
    }
  }

  virtual void setUpControls() {
    for (int i = 0; i < shared->parameters.GetSize(); i++) {
      ParameterCoupling* couple = shared->parameters.Get(i);
      const double value = *(couple->value);
      const float px = shared->X + couple->x - (couple->w * 0.5f);
      const float py = shared->Y + couple->y - (couple->h * 0.5f);
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
          // TODOG Add a label with the current value
          couple->setFromNormalized(pCaller->GetValue());
        }, couple->name, DEFAULT_STYLE, true, true
        );
      }
      
      couple->control->SetValue(couple->getNormalized());
      shared->graphics->AttachControl(couple->control);
      mElements.Add(couple->control);
      if (i == 0 && mHeader.hasByPass) { couple->control->Hide(true); }

      // optionally hide the lables etc
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
    for (int i = 0; i < shared->parameters.GetSize(); i++) {
      ParameterCoupling* p = shared->parameters.Get(i);
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

  virtual void cleanUp() const {
    for (int i = 0; i < shared->parameters.GetSize(); i++) {
      ParameterCoupling* param = shared->parameters.Get(i);
      if (param->control != nullptr) {
        // this also destroys the object
        shared->graphics->RemoveControl(param->control, true);
        param->control = nullptr;
      }
    }

    for (int i = 0; i < mInSocketsUi.GetSize(); i++) {
      shared->graphics->RemoveControl(mInSocketsUi.Get(i), true);
    }

    for (int i = 0; i < mOutSocketsUi.GetSize(); i++) {
      shared->graphics->RemoveControl(mOutSocketsUi.Get(i), true);
    }

    if (mHeader.hasByPass) {
      shared->graphics->RemoveControl(mHeader.bypass, true);
    }
    shared->graphics->RemoveControl(mHeader.disconnect, true);
    shared->graphics->RemoveControl(mHeader.remove, true);
  }

  virtual void DrawHeader(IGraphics& g) {
    g.FillRect(Theme::Node::HEADER,
      IRECT(
        mTargetRECT.L, mTargetRECT.T, mTargetRECT.R, mTargetRECT.T + Theme::Node::HEADER_SIZE
      )
    );
  }

  virtual void DrawBg(IGraphics& g) {
    if (mUseSvgBg) {
      g.DrawSVG(mSvgBg, mTargetRECT);
    }
    else {
#ifdef NODE_ROUNDED_CORNER
      g.FillRoundRect(mColor, mTargetRECT, Theme::Node::ROUNDED_CORNER);
#else
      g.FillRect(mColor, mTargetRECT);
#endif
    }
  }

  virtual void Draw(IGraphics& g) override {
#if defined(NODE_SHADOW) || defined (NODE_CACHE_BG)
    if (!mBgIsCached) {
      g.StartLayer(this, mRECT);
#endif
      // This part will always run
      DrawBg(g);
      DrawHeader(g);
#if defined(NODE_SHADOW) || defined(NODE_CACHE_BG)
      mCachedBgLayer = g.EndLayer();
#ifdef NODE_SHADOW
      g.ApplyLayerDropShadow(mCachedBgLayer, IShadow(
        Theme::Node::SHADOW_COLOR, Theme::Node::SHADOW_BLUR,
        Theme::Node::SHADOW_DIST_X, Theme::Node::SHADOW_DIST_Y,
        1.0, true
      ));
#endif
      mBgIsCached = true;
    }
    g.DrawFittedLayer(mCachedBgLayer, mRECT, &mBlend);
#endif
  }

  virtual void OnMouseUp(float x, float y, const IMouseMod& mod) override {
    if (mDragging) {
      mDragging = false;
      MessageBus::fireEvent<Node*>(shared->bus, MessageBus::NodeDraggedEnd, shared->node);
      return;
    }
  }

  virtual void OnMouseDrag(const float x, const float y, const float dX, const float dY, const IMouseMod& mod) override {
    mDragging = true;
    MessageBus::fireEvent<Coord2D>(shared->bus, MessageBus::NodeDragged, Coord2D {x, y});
    translate(dX, dY);
  }

  virtual void translate(const float dX, const float dY) {
    mNoScale = true;
    for (int i = 0; i < mElements.GetSize(); i++) {
      moveControl(mElements.Get(i), dX, dY);
    }

    for (int i = 0; i < shared->socketsIn.GetSize(); i++) {
      shared->socketsIn.Get(i)->mX += dX;
      shared->socketsIn.Get(i)->mY += dY;
    }

    for (int i = 0; i < shared->socketsOut.GetSize(); i++) {
      shared->socketsOut.Get(i)->mX += dX;
      shared->socketsOut.Get(i)->mY += dY;
    }

    shared->X += dX;
    shared->Y += dY;

    shared->graphics->SetAllControlsDirty();
  }

  virtual void OnResize() override {
    if (!mNoScale) {
      mBgIsCached = false;
    }
    mNoScale  = false;
  }

  void setTranslation(float x, float y) {
    const float dX = x - shared->X;
    const float dY = y - shared->Y;
    translate(dX, dY);
  }

  map<const char*, ParameterCoupling*> mParamsByName;


private:
  static void moveControl(IControl* control, float x, float y) {
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
};
