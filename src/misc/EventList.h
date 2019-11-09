#pragma once

namespace MessageBus {
  enum EVENTID {
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
    TOTALEVENTS
  };
}