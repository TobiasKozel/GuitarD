// Wrap the debug message from iplug in the iplug namespace on windows
// since using iplug::DBGMSG will exapnd to iplug::printf() and fail on linux/osx

#if defined(OS_MAC) || defined(OS_LINUX) || defined(OS_WEB) || defined(OS_IOS) || NDEBUG
  #define WDBGMSG(...) DBGMSG(__VA_ARGS__)
#elif defined OS_WIN
  #define WDBGMSG(...) iplug::DBGMSG(__VA_ARGS__)
#endif


// Max amount of nodes in the graph
#define MAX_NODES 128

// Amounts of daw params to register at start since dynamic amounts are not well supported
#define MAX_DAW_PARAMS 256

// 8 Should be enough for most nodes
#define MAX_NODE_SOCKETS 8

// Max amount of Parametercouplings for a node
#define MAX_NODE_PARAMETERS 32

// Meters are structs to share info about the dsp to the gui
#define MAX_NODE_METERS 32

// This is the name of a node if it wasn't overridden anywhere
#define DEFAULT_NODE_NAME "DEFAULTNODENAME"

#define MAX_BUFFER 512

#define MAX_UNDOS 8

#define SPLICEIN_DISTANCE 14


#ifdef FLOATCONV
  #define FFTCONVOLVER_USE_SSE
#else
  #define WDL_RESAMPLE_TYPE double
#endif