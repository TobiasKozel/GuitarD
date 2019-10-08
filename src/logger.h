// Wrap the debug message from iplug in the iplug namespace on windows
// since using iplug::DBGMSG will exapnd to iplug::printf() and fail on linux/osx

#if defined(OS_MAC) || defined(OS_LINUX) || defined(OS_WEB) || defined(OS_IOS) || NDEBUG
  #define WDBGMSG(...) DBGMSG(__VA_ARGS__)
#elif defined OS_WIN
  #define WDBGMSG(...) iplug::DBGMSG(__VA_ARGS__)
#endif
