#pragma once

#include "./GTypes.h"

namespace guitard {
	/**
	 * Collection of small structs to pass around using the message bus,
	 * or just convenience
	 */

	struct Coord2D {
		Coord2D() { }
		Coord2D(float p_x, float p_y) {
			x = p_x;
			y = p_y;
		}
		float x = 0;
		float y = 0;
	};

	struct Drag {
		Coord2D pos;
		Coord2D delta;
	};

	struct QuickConnectRequest {
		Coord2D pos;
		NodeSocket* from;
	};

	struct NodeDragSpawnRequest {
		Coord2D pos;
		String name;
	};

	struct NodeSpliceInPair {
		Node* node = nullptr; // The Node that needs to be spliced in
		NodeSocket* socket = nullptr;
	};

	struct GraphStats {
		long long executionTime = 0;
		int nodeCount = 0;
		bool valid = true;
	};

	struct SocketConnectRequest {
		NodeSocket* from = nullptr;
		NodeSocket* to = nullptr;
	};

	struct ConnectionDragData {
		bool dragging = false;
		float startY = 0;
		float startX = 0;
		float currentY = 0;
		float currentX = 0;
	};

	struct AutomationAttachRequest {
		Node* automationNode = nullptr;
		void* targetControl = nullptr; // TODO bad this references a ui node
	};

	struct NodeDragEndData {
		Node* node;
		bool addCombineNode;
	};

	struct NodeSelectionChanged {
		NodeUi* node = nullptr;
		bool replace = false;
		bool remove = false;
	};

	struct BlockSizeEvent {
		int blockSize;
		bool set = false;
	};
}