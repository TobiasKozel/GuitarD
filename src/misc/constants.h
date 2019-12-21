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

// Max amount of sockets a output socket can be connected to (not a hard limit, but weird things will happen)
#define MAX_SOCKET_CONNECTIONS 32

// Max amount of Parametercouplings for a node
#define MAX_NODE_PARAMETERS 32

// Meters are structs to share info about the dsp to the gui
#define MAX_NODE_METERS 32

// This is the name of a node if it wasn't overridden anywhere
#define DEFAULT_NODE_NAME "DEFAULTNODENAME"

// Nodes will allocated this much space for their buffers, larger chunks will be split up and processed normally
#define MAX_BUFFER 512

#define MIN_BLOCK_SIZE 16

#define MAX_UNDOS 8

// Distance in pixels from the cable the cursor needs to be within for the splice in to happen
#define SPLICEIN_DISTANCE 14


#ifdef FLOATCONV
  #define FFTCONVOLVER_USE_SSE
#else
  #define WDL_RESAMPLE_TYPE double
#endif