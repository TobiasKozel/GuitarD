#pragma once
namespace guitard {
  namespace MessageBus {
    enum MESSAGE_ID {
      NodeAdd = 0,
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
      OpenPopUp, // Takes an IControl and pushes it on top of the render stack
      NodeSpliceInCombine, // Adds a combine node after the node passed with this event
      GraphStatsChanged, // Fired from the audio thread once a graph did not finish processing fot the first time
      LoadPresetFromString, // Takes a const char*
      SavePresetToSring, // Takes a WDL_String* were the result will be in
      MaxBlockSizeEvent, // Handles both setting and getting the maxBufferSize
      TOTAL_MESSAGE_IDS // Keep this one at the bottom to count all events
    };
  }
}