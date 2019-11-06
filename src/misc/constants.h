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
#define MAXDAWPARAMS 256

// This is the name of a node if it wasn't overridden anywhere
#define DefaultNodeName "DEFAULTNODENAME"

#define MAXBUFFER 512

#define SOCKETDIAMETER 30

#define DEBUGFONT iplug::igraphics::IText { 16, COLOR_WHITE, "Roboto-Regular", iplug::igraphics::EAlign::Center, iplug::igraphics::EVAlign::Middle, 0 }

#define COLORBACKGROUND 245, 245, 245
#define COLORPANEL1 220, 220, 220
#define COLORACCENT 233, 140, 36
#define BACKGROUNDDETAILDIST  140
#define BACKGROUNDDETAILSIZE  12
#define BACKGROUNDDETAILWIDTH  1.5
