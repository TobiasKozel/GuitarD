#define PLUG_NAME "GuitarD"
#define PLUG_MFR "Tobias"
#define PLUG_VERSION_HEX 0x00010000
#define PLUG_VERSION_STR "1.0.0"
#define PLUG_UNIQUE_ID 'BM7T'
#define PLUG_MFR_ID 'TOBI'
#define PLUG_URL_STR "https://iplug2.github.io"
#define PLUG_EMAIL_STR "spam@me.com"
#define PLUG_COPYRIGHT_STR "Copyright 2019 me lol"

#ifndef GUITARD_HEADLESS
    #define PLUG_CLASS_NAME GuitarD
    #define AUV2_ENTRY GuitarD_Entry
    #define AUV2_ENTRY_STR "GuitarD_Entry"
    #define AUV2_FACTORY GuitarD_Factory
    #define AUV2_VIEW_CLASS GuitarD_View
    #define AUV2_VIEW_CLASS_STR "GuitarD_View"
#endif

#define BUNDLE_NAME "GuitarD"
#define BUNDLE_MFR "Tobias"
#define BUNDLE_DOMAIN "com"

#define SHARED_RESOURCES_SUBPATH "GuitarD"

#define PLUG_CHANNEL_IO "1-2 2-2"

// #define SAMPLE_TYPE_FLOAT

#define PLUG_LATENCY 0
#define PLUG_TYPE 0
#define PLUG_DOES_MIDI_IN 0
#define PLUG_DOES_MIDI_OUT 0
#define PLUG_DOES_MPE 0
#define PLUG_DOES_STATE_CHUNKS 1
#define PLUG_HAS_UI 1
#define PLUG_WIDTH 980
#define PLUG_HEIGHT 550
#define PLUG_MAX_WIDTH 4000
#define PLUG_MAX_HEIGHT 4000
#define PLUG_MIN_WIDTH 100
#define PLUG_MIN_HEIGHT 100

#define PLUG_FPS 60
#define PLUG_SHARED_RESOURCES 0



#define AAX_TYPE_IDS 'EFN1', 'EFN2'
#define AAX_TYPE_IDS_AUDIOSUITE 'EFA1', 'EFA2'
#define AAX_PLUG_MFR_STR "TOBI"
#define AAX_PLUG_NAME_STR "GuitarD\nIPEF"
#define AAX_PLUG_CATEGORY_STR "Effect"
#define AAX_DOES_AUDIOSUITE 1

#define VST3_SUBCATEGORY "Fx"

#define APP_NUM_CHANNELS 2
#define APP_N_VECTOR_WAIT 0
#define APP_MULT 1
#define APP_COPY_AUV3 0
#define APP_RESIZABLE 0
#define APP_SIGNAL_VECTOR_SIZE 64

/**                 ASSETS                  **/

#define ROBOTO_FN "Roboto-Regular.ttf"
#define ICON_FN "forkawesome-webfont.ttf"

#define SVGBITTERBG_FN "bitter.svg"
#define SVGFEEDBACK_FN "feedback.svg"

//#define PNGSTEREOSHAPERBG_FN "stereoshaper_bg.png"
//#define PNGSIMPLECABBG_FN "simplecab_bg.png"
//#define PNGGENERICBG_FN "generic_bg.png"
