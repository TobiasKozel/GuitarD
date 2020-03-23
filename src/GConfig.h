/**
 * A few settings which allow more objects on the stack
 * TODO maybe use constexpr instead of defines
 */

/**
 * Amounts of daw params to register at start since dynamic amounts are not well supported
 */
#define GUITARD_MAX_DAW_PARAMS 256

/**
 * 8 Sockets for each in and output should be enough
 */
#define GUITARD_MAX_NODE_SOCKETS 8

/**
 * Max amount of sockets a output socket can be connected to
 * This is not a hard limit, but weird things will happen on the UI side
 */
#define GUITARD_MAX_SOCKET_CONNECTIONS 32

/**
 * Max amount of Parametercouplings for a node
 */
#define GUITARD_MAX_NODE_PARAMETERS 16

/**
 * Meters are structs to share info about the dsp to the gui
 */
#define GUITARD_MAX_NODE_METERS 8

/**
 * This is the name of a node if it wasn't overridden anywhere
 */
#define GUITARD_DEFAULT_NODE_NAME "DEFAULTNODENAME"

/**
 * Nodes will allocated this much space for their buffers, larger chunks will be split up and processed normally
 * So making this smaller won't crash the dsp but use more cpu
 */
#define GUITARD_MAX_BUFFER 512

/**
 * Means we'll do float convolution since it allows sse
 */
#define GUITARD_FLOAT_CONVOLUTION

#ifdef __arm__ // No sse for arm obviously
  #undef GUITARD_SSE
#endif