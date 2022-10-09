#pragma once

#include <string>

namespace guitard {
	class WrappedConvolver;
	class ParameterManager;
	class Node;
	class NodeUi;
	class Graph;
	struct NodeSocket;

	typedef
#ifdef SAMPLE_TYPE_FLOAT
	float
#else
	double
#endif
	sample;

	typedef std::string String;

	static const int kNoParameter = -1;
	static const int kNoValIdx = -1;
	static const double PI = 3.14159265358979323846;
}

	#ifdef NDEBUG
		#define WDBGMSG(...)
	#else
		#define WDBGMSG(...) printf(__VA_ARGS__);
	#endif

/**
 * A macro to disable all kinds of implicit copy mechanisms
 */
#define GUITARD_NO_COPY(name) \
	name(const name&) = delete; \
	name(const name*) = delete; \
	name(name&&) = delete; \
	name& operator= (const name&) = delete; \
	name& operator= (name&&) = delete;
