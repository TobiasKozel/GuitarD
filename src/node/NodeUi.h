#pragma once
#ifndef GUITARD_HEADLESS
#include "IControls.h"
#include "../types/iplugTypes.h"
#include "../node/NodeSocket.h"
#include "../node/NodeSocketUi.h"
#include "../misc/MessageBus.h"
#include "../types/gstructs.h"
#include "./NodeShared.h"

namespace guitard {
  class Node;

  struct NodeUiHeader {
    bool hasByPass = false;
    bool hasRemove = true;
    IControl* bypass;
    IControl* remove;
  };


  /**
   * This class represents a Node on the UI, it's seperate to the node itself
   * since it will only exists as long as the UI window is open but is owned by the node
   */
  class NodeUi : public IControl {
  protected:
    MessageBus::Subscription<NodeSpliceInPair> mNodeSpliceInEvent;
    MessageBus::Subscription<QuickConnectRequest> mNodeQuickConnectEvent;

    NodeSocketUi* mInSocketsUi[MAX_NODE_SOCKETS] = { nullptr };
    NodeSocketUi* mOutSocketsUi[MAX_NODE_SOCKETS] = { nullptr };

    NodeUiHeader mHeader;
    PointerList<IControl> mElements;

    ILayerPtr mCachedBgLayer;
    bool mBgIsCached = false;
    bool mUseSvgBg = false;
    IBlend mBlend = { EBlend::Default, 1 };
    bool mNoScale = false;

    IColor mColor;
    ISVG mSvgBg = ISVG(nullptr);
    IText mIconFont;
    bool mDoRender = true;
    bool mSelected = false;
    bool mSelectPressed = false;


  public:
    NodeShared* shared; // TODO this is only temporary
    bool mDragging = false;

    /**
     * Conveniently map names to the parameter couples
     * Const char* works for now since all the accessors are known at compile time, prolly a result of the header only thing
     * TODOG provide a comparator or switch to strings
     */
    std::map<const char*, ParameterCoupling*> mParamsByName;

    explicit NodeUi(NodeShared* pShared) :
      IControl(IRECT(0, 0, 0, 0), kNoParameter)
    {
      shared = pShared;

      NodeUi::setUpDimensions(shared->width, shared->height);

      for (int i = 0; i < shared->parameterCount; i++) {
        /**
         * Keep them around in a map for convenient use
         * Only do this in the UI though
         */
        ParameterCoupling* p = &shared->parameters[i];
        mParamsByName.insert(std::pair<const char*, ParameterCoupling*>(p->name, p));
      }

      mNodeSpliceInEvent.subscribe(shared->bus, MessageBus::NodeSpliceIn, [&](NodeSpliceInPair pair) {
        if (shared->node != pair.node) { return; }
        /**
         * Splice in only works on nodes with at least one in and output
         */
        NodeSocket* in = getQuickConnectSocket(false);
        NodeSocket* out = shared->socketsOut[0];
        NodeSocket* prev = pair.socket->mConnectedTo[0];
        if (in != nullptr && out != nullptr) {
          pair.socket->disconnect();
          in->connect(prev);
          out->connect(pair.socket);
        }
      });

      mNodeQuickConnectEvent.subscribe(shared->bus, MessageBus::QuickConnectSocket, [&](QuickConnectRequest req) {
        if (mTargetRECT.Contains(IRECT{ req.pos.x, req.pos.y, req.pos.x, req.pos.y })) {
          NodeSocket* socket = getQuickConnectSocket(req.from->mIsInput);
          if (socket != nullptr) {
            socket->connect(req.from);
          }
        }
      });

      mIconFont = ICON_FONT;
    }

    virtual ~NodeUi() {
    }

    /**
     * Tries to return a unconnected input or output socket
     * Will return a connected one if no unconnected one is found
     */
    virtual NodeSocket* getQuickConnectSocket(const bool output = false) {
      for (int i = 0; i < MAX_NODE_SOCKETS; i++) {
        NodeSocket* socket = output ? shared->socketsOut[i] : shared->socketsIn[i];
        if (socket != nullptr && !socket->mConnected) {
          // Look for the first unconnected socket
          return socket;
        }
      }
      for (int i = 0; i < MAX_NODE_SOCKETS; i++) {
        NodeSocket* socket = output ? shared->socketsOut[i] : shared->socketsIn[i];
        if (socket != nullptr) {
          // Use a socket even if it is connected
          return socket;
        }
      }
      return nullptr;
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
          ParameterCoupling* p = &this->shared->parameters[0];
          const bool bypassed = static_cast<bool>(p->control->GetValue());
          p->control->SetValueFromUserInput(bypassed ? 0.0 : 1.0);
        }, u8"\uf056", u8"\uf011", mIconFont);
        mElements.add(mHeader.bypass);
        shared->graphics->AttachControl(mHeader.bypass);
      }

      mHeader.remove = new IVButtonControl(IRECT(
        m.R - Theme::Node::HEADER_REMOVE_RIGHT - Theme::Node::HEADER_DISCONNECT_SIZE,
        m.T + Theme::Node::HEADER_DISCONNECT_TOP, m.R - Theme::Node::HEADER_REMOVE_RIGHT,
        m.T + Theme::Node::HEADER_DISCONNECT_TOP + Theme::Node::HEADER_DISCONNECT_SIZE
      ), [&](IControl* pCaller) {
        MessageBus::fireEvent<Node*>(shared->bus, MessageBus::NodeDeleted, this->shared->node);
      });
      mElements.add(mHeader.remove);
      shared->graphics->AttachControl(mHeader.remove);
    }


    virtual void setUpSockets() {
      for (int i = 0; i < shared->inputCount; i++) {
        NodeSocketUi* socket = new NodeSocketUi(shared, shared->socketsIn[i]);
        shared->graphics->AttachControl(socket);
        mInSocketsUi[i] = socket;
        mElements.add(socket);
      }

      for (int i = 0; i < shared->outputCount; i++) {
        NodeSocketUi* socket = new NodeSocketUi(shared, shared->socketsOut[i]);
        shared->graphics->AttachControl(socket);
        mOutSocketsUi[i] = socket;
        mElements.add(socket);
      }
    }

    virtual void setUpControls() {
      for (int i = 0; i < shared->parameterCount; i++) {
        ParameterCoupling* couple = &shared->parameters[i];
        const float px = shared->X + couple->x - (couple->w * 0.5f);
        const float py = shared->Y + couple->y - (couple->h * 0.5f);
        IRECT controlPos(px, py, px + couple->w, py + couple->h);
        // use the daw parameter to sync the values if possible
        if (couple->parameterIdx != kNoParameter) {
          couple->control = new IVKnobControl(
            controlPos, couple->parameterIdx, couple->name, DEFAULT_STYLE, true, false,
            couple->lowAngle, couple->highAngle, couple->centerAngle
          );
          couple->control->SetValue(couple->getNormalized());
        }
        else {
          // use the callback to get the value to the dsp, won't allow automation though
          couple->control = new IVKnobControl(
            controlPos, [couple](IControl* pCaller) {
            // TODOG Add a label with the current value
            couple->setFromNormalized(pCaller->GetValue());
            WDL_String val;
            val.SetFormatted(MAX_PARAM_DISPLAY_LEN, "%.*f", 2, couple->getValue());
            IVectorBase* vcontrol = dynamic_cast<IVectorBase*>(couple->control);
            vcontrol->SetValueStr(val.Get());
          }, couple->name, DEFAULT_STYLE, true, false,
            couple->lowAngle, couple->highAngle, couple->centerAngle
            );
          couple->control->SetValue(couple->getNormalized());
          couple->control->SetDirty();
        }
        shared->graphics->AttachControl(couple->control);
        mElements.add(couple->control);
        if (i == 0 && mHeader.hasByPass) { couple->control->Hide(true); }

        // optionally hide the lables etc
        IVectorBase* vcontrol = dynamic_cast<IVectorBase*>(couple->control);
        if (vcontrol != nullptr) {
          vcontrol->SetShowLabel(couple->showLabel);
          vcontrol->SetShowValue(couple->showValue);
        }
      }
    }

    virtual void setUp() {
      for (int i = 0; i < shared->parameterCount; i++) {
        ParameterCoupling* p = &shared->parameters[i];
        if (strncmp(p->name, "Bypass", 10) == 0) {
          mHeader.hasByPass = true;
          break;
        }
      }
      mElements.add(this);
      setUpControls();
      setUpSockets();
      setUpHeader();
      translate(0, 0);
    }

    void OnDetached() override {
      MessageBus::fireEvent<NodeSelectionChanged>(
        shared->bus, MessageBus::NodeSelectionChange, { this, false, true }
      );
      mDirty = false;
      mDoRender = false;
      for (int i = 0; i < shared->parameterCount; i++) {
        ParameterCoupling* param = &shared->parameters[i];
        if (param->control != nullptr) {
          // this also destroys the object
          shared->graphics->RemoveControl(param->control);
          param->control = nullptr;
        }
      }

      for (int i = 0; i < shared->inputCount; i++) {
        shared->graphics->RemoveControl(mInSocketsUi[i]);
      }

      for (int i = 0; i < shared->outputCount; i++) {
        shared->graphics->RemoveControl(mOutSocketsUi[i]);
      }

      if (mHeader.hasByPass) {
        shared->graphics->RemoveControl(mHeader.bypass);
      }
      shared->graphics->RemoveControl(mHeader.remove);
    }

    virtual void DrawHeader(IGraphics& g) {
      IRECT bound = IRECT(
        mTargetRECT.L, mTargetRECT.T, mTargetRECT.R, mTargetRECT.T + Theme::Node::HEADER_SIZE
      );
      g.FillRect(mSelected ? Theme::Node::HEADER_SELECTED : Theme::Node::HEADER, bound);
      g.DrawText(Theme::Node::HEADER_TEXT, shared->info->displayName.c_str(), bound);
    }

    virtual void DrawBg(IGraphics& g) {
      if (mUseSvgBg) {
#ifdef IGRAPHICS_SKIA
        g.DrawSVG(mSvgBg, mTargetRECT.GetTranslated(0, 90));
#else
        g.DrawSVG(mSvgBg, mTargetRECT);
#endif
      }
      else {
#ifdef NODE_ROUNDED_CORNER
        g.FillRoundRect(mColor, mTargetRECT, Theme::Node::ROUNDED_CORNER);
#else
        g.FillRect(mColor, mTargetRECT);
#endif
      }
    }

    void Draw(IGraphics& g) override {
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
        if (mSelected) {
          g.ApplyLayerDropShadow(mCachedBgLayer, iplug::igraphics::IShadow(
            Theme::Node::HEADER_SELECTED, 0,
            Theme::Node::SHADOW_DIST_X, Theme::Node::SHADOW_DIST_Y,
            1.0, true
          ));
          g.ApplyLayerDropShadow(mCachedBgLayer, iplug::igraphics::IShadow(
            Theme::Node::HEADER_SELECTED, 0,
            -Theme::Node::SHADOW_DIST_X, Theme::Node::SHADOW_DIST_Y,
            1.0, true
          ));
        }
        g.ApplyLayerDropShadow(mCachedBgLayer, iplug::igraphics::IShadow(
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

    void OnMouseDown(float x, float y, const IMouseMod& mod) override {
      IControl::OnMouseDown(x, y, mod);
      mSelectPressed = (!mod.A && mod.C && !mod.S);
      if (!mSelected) {
        MessageBus::fireEvent<NodeSelectionChanged>(
          shared->bus, MessageBus::NodeSelectionChange, { this, !mSelectPressed }
        );
      }
      else if (mSelectPressed) {
        MessageBus::fireEvent<NodeSelectionChanged>(
          shared->bus, MessageBus::NodeSelectionChange, { this, false }
        );
      }
    }

    void OnMouseDblClick(float x, float y, const IMouseMod& mod) override {
      IControl::OnMouseDblClick(x, y, mod);
      /**
       * Delete on alt double click
       */
      if (mod.A && !mod.C && !mod.S) {
        MessageBus::fireEvent<Node*>(shared->bus, MessageBus::NodeDeleted, shared->node);
      }
    }

    void OnMouseUp(float x, float y, const IMouseMod& mod) override {
      if (mDragging) {
        mDragging = false;
        MessageBus::fireEvent<NodeDragEndData>(shared->bus, MessageBus::NodeDraggedEnd, { shared->node, !mod.A && mod.C && mod.S });
      }
      else {
        if (!mSelected && !mSelectPressed) {
          MessageBus::fireEvent<NodeSelectionChanged>(
            shared->bus, MessageBus::NodeSelectionChange, { this, false }
          );
        }
      }
      mSelectPressed = false;
    }

    void OnMouseDrag(const float x, const float y, const float dX, const float dY, const IMouseMod& mod) override {
      if (!mDragging && std::abs(dX) + std::abs(dY) > 1.f) {
        if (mod.A && mod.C && !mod.S) {
          /**
           * Alt and Control
           * Disconnect all the connections of a node
           */
          MessageBus::fireEvent<Node*>(shared->bus, MessageBus::NodeDisconnectAll, shared->node);
          mDragging = true;
          return;
        }
        if (mod.A && !mod.C && !mod.S) {
          /**
           * Alt drag
           * Bypass all connections of a node
           */
          MessageBus::fireEvent<Node*>(shared->bus, MessageBus::BypassNodeConnection, shared->node);
          mDragging = true;
          return;
        }
        if (!mod.A && mod.C && !mod.S) {
          /**
           * Control drag
           * Duplicate the node
           */
          MessageBus::fireEvent<Node*>(shared->bus, MessageBus::CloneNode, shared->node);
          return;
        }
        if (!mod.A && !mod.C && !mod.S && mod.R) {
          /**
           * right click drag
           * Get the fist output and drag it
           */
          NodeSocketUi* socket = mod.L ? mInSocketsUi[0] : mOutSocketsUi[0];
          if (socket != nullptr) {
            socket->OnMouseDown(x, y, mod);
            // swap the captured control to be the outsocket now
            shared->graphics->SetCapturedControl(socket);
          }
          return;
        }
        if (!mod.A && !mod.C && mod.S) {
          /**
           * Shift drag
           * Combine the original signal with output of the node
           */
          MessageBus::fireEvent<Node*>(shared->bus, MessageBus::NodeSpliceInCombine, shared->node);
          return;
        }
        if (!mod.A && mod.C && mod.S) {
          /**
           * Shift + control drag
           * Combine it with duplicate of this node
           */
          return;
        }
      }
      /**
       * Default case is simple drag
       */
      mDragging = true;
      MessageBus::fireEvent<Drag>(shared->bus, MessageBus::NodeDragged, Drag{ {x, y}, {dX, dY } });
      // translate(dX, dY);
    }

    virtual void translate(const float dX, const float dY) {
      mNoScale = true;
      for (int i = 0; i < mElements.size(); i++) {
        moveControl(mElements[i], dX, dY);
      }

      for (int i = 0; i < shared->inputCount; i++) {
        shared->socketsIn[i]->mX += dX;
        shared->socketsIn[i]->mY += dY;
      }

      for (int i = 0; i < shared->outputCount; i++) {
        shared->socketsOut[i]->mX += dX;
        shared->socketsOut[i]->mY += dY;
      }

      shared->X += dX;
      shared->Y += dY;

      shared->graphics->SetAllControlsDirty();
    }

    void OnResize() override {
      if (!mNoScale) {
        mBgIsCached = false;
      }
      mNoScale = false;
    }

    void setTranslation(const float x, const float y) {
      const float dX = x - shared->X;
      const float dY = y - shared->Y;
      translate(dX, dY);
    }

    void setSelected(bool selected) {
      mBgIsCached = false;
      mSelected = selected;
      mDirty = true;
    }




  private:
    static void moveControl(IControl* control, const float x, const float y) {
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
}

#else
namespace guitard {
  class NodeUi;
}
#endif