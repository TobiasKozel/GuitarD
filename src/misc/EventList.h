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
    SocketRedirectConnection, // Takes a SocketConnectRequest and will replace the current connection with the one provided
    PreviewSocket,
    OpenGallery,
    ParametersChanged, // Informs the DAW about a change of parameter names (doesn't seem to bother VST3)
    PushUndoState,
    PopUndoState,
    GetGraphStats,
    ConnectionDragged,
    AttachAutomation, // Takes AutomationAttachRequest and links up automation accordingly
    VisualizeAutomationTargets, // Takes a node and will show all the IControl it automates
    PickAutomationTarget, // Enter a mode which allows picking a IControl as an automation target
    BypassNodeConnection, // Bypasses a whole nodes connections and disconnect all in/outputs
    SeverNodeConnection,  // Cut all the connections
    CloneNode,
    QuickConnectSocket, // Event fired from a NodeSocketUi and subscribed to by NodeUi to allow connecting without hitting the socket itself
    SortRenderStack, // Keeps the render stack in proper order e.g after adding a node, creating a popup
    OpenPopUp, // Takes an IControl and pushes it on top of the render stack
    TOTAL_MESSAGE_IDS // Keep this one at the bottom to count all events
  };
}