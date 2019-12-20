#pragma once

namespace MessageBus {
  enum MESSAGE_ID {
    NodeAdd,
    NodeDeleted,
    NodeDisconnectAll,
    NodeSpliceIn,
    NodeDraggedEnd,
    NodeDragged,
    AwaitAudioMutex,
    DisconnectSocket,
    SocketConnect,
    SocketRedirectConnection,
    PreviewSocket,
    OpenGallery,
    ParametersChanged,
    PushUndoState,
    PopUndoState,
    GetGraphStats,
    ConnectionDragged,
    AttachAutomation,
    VisualizeAutomationTargets,
    PickAutomationTarget,
    BypassNodeConnection,
    TOTAL_MESSAGE_IDS // Keep this one at the bottom
  };
}