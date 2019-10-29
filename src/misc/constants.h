// Wrap the debug message from iplug in the iplug namespace on windows
// since using iplug::DBGMSG will exapnd to iplug::printf() and fail on linux/osx

#if defined(OS_MAC) || defined(OS_LINUX) || defined(OS_WEB) || defined(OS_IOS) || NDEBUG
  #define WDBGMSG(...) DBGMSG(__VA_ARGS__)
#elif defined OS_WIN
  #define WDBGMSG(...) iplug::DBGMSG(__VA_ARGS__)
#endif


// Max amount of nodes in the graph
#define MAXNODES 128

// Amounts of daw params to register at start since dynamic amounts are not well supported
#define MAXDAWPARAMS 128

// This is the name of a node if it wasn't overridden anywhere
#define DefaultNodeName "DEFAULTNODENAME"

#define MAXBUFFER 512

#define SOCKETDIAMETER 30