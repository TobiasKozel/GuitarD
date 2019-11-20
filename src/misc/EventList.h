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
    TOTAL_MESSAGE_IDS
  };
}