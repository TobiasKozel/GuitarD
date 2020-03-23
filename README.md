# GuitarD

It's a basic multi effects processor which follows a node based approach.
It's fairly unstable and experimental but contains most of the important features.
This whole thing started as my bachelors thesis, but I plan on developing it further to a point where it's usable.

## Builds
No pre-build Binaries yet.

## Compilation
To build a plugin you'll need [this fork of iPlug2](https://github.com/TobiasKozel/iPlug2) and switch to the `guitard` branch there.

To get iPlug2 up and running, make sure all its dependencies are downloaded. To do tha, run:

`iPlug2/Dependencies/download-prebuilt-libs.sh`

`iPlug2/Dependencies/IPlug/download-iplug-libs.sh` (use git bash on windows)

After that, simply clone this repository into the Examples folder of iPlug2 and open the IDE project for your platform.

Make sure the [FAUST compiler](https://github.com/grame-cncm/faust/releases) is installed and in you PATH environment variable. Run `./scripts/compile_faust.py` to compile all the DSP code.

The VST/AU plugin works on Windows and Mac OS X.

The DSP code can be compiled without iPlug and the GUI so it can be included in other plugins easily. Just include `./src/headless/headless.h` and you should be good to go. Everything is header only to make the code as portable as possible. The headless version was testet on Windows (MSVC, gcc), Mac OS X (clang) and Linux (gcc, clang).

## Where to start

The code seperates UI and DSP fairly well, so I'd recommend having a look at the DSP side first (Graph.h and Node.h). The whole UI is just tacked on top of it and controlls the DSP part.
