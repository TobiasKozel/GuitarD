#ifndef DISTRHO_PLUGIN_INFO_H_INCLUDED
#define DISTRHO_PLUGIN_INFO_H_INCLUDED

#define DISTRHO_PLUGIN_BRAND              "CAUDIO"
#define DISTRHO_PLUGIN_NAME               "GuitarD"
#define DISTRHO_PLUGIN_URI                "https://TODO.com"
#define DISTRHO_PLUGIN_LICENSE            "WTFPL"


#define DISTRHO_PLUGIN_HAS_UI             1
#define DISTRHO_PLUGIN_IS_RT_SAFE         1

#define DISTRHO_PLUGIN_NUM_INPUTS         2
#define DISTRHO_PLUGIN_NUM_OUTPUTS        2

#define DISTRHO_PLUGIN_WANT_PROGRAMS      0
#define DISTRHO_PLUGIN_WANT_STATE         1
#define DISTRHO_PLUGIN_WANT_FULL_STATE    1

#define DISTRHO_UI_FILE_BROWSER           0
#define DISTRHO_UI_USER_RESIZABLE         1
#define DISTRHO_UI_DEFAULT_WIDTH          768
#define DISTRHO_UI_DEFAULT_HEIGHT         512

// additional non things not controlling DPF itself
#define DISTRHO_PLUGIN_VERSION_MAJOR      0
#define DISTRHO_PLUGIN_VERSION_MINOR      0
#define DISTRHO_PLUGIN_VERSION_PATCH      1

#define DISTRHO_PLUGIN_STATE_PATCH        "patch"
#define DISTRHO_PLUGIN_STATE_SCREENSHOT   "screenshot"
#define DISTRHO_PLUGIN_STATE_NAME         "name"
#define DISTRHO_PLUGIN_STATE_WINDOW_SIZE  "window"
// Number of states above
#define DISTRHO_PLUGIN_STATE_COUNT        4

// Number of generic automatable parameters exposed to DAW
#define DISTRHO_PLUGIN_PARAMETER_COUNT    64

#endif // DISTRHO_PLUGIN_INFO_H_INCLUDED
