This repository contains a CMake script to build the thirdparty packages that
are necessary for building Panda3D.

Usage example on Windows:

    mkdir build
    cd build
    cmake -G"Visual Studio 16 2019" -A x64 ..

    # to build everything:
    cmake --build . --config Release

    # to just build ffmpeg and its dependencies:
    cmake --build . --config Release --target ffmpeg

Usage example on other systems:

    mkdir build
    cd build
    cmake ..
    make ffmpeg # just build ffmpeg and dependencies
    make all # build everything

Some packages are still forthcoming.  The included packages are ticked.
- [x] artoolkit
- [x] assimp
- [x] bullet
- [x] eigen
- [x] fcollada
- [x] ffmpeg
- [ ] fmodex
- [x] freetype
- [x] harfbuzz
- [x] jpeg
- [x] mimalloc
- [x] nvidiacg (except arm64 or Android)
- [x] ode
- [x] openal
- [x] opencv (macOS only)
- [x] openexr
- [x] openssl
- [x] opus
- [x] png
- [x] squish
- [x] tiff
- [x] vorbis
- [x] vrpn
- [x] zlib


If you want to build a subset of packages, it is easiest to just pass the name
of the target to the build system.  For example, to just build VRPN, you can
call `make vrpn` or add the `--target vrpn` flag to `cmake --build`.
This will still build its dependencies if one package depends on another one.
A way to force a certain package to be disabled is by using the `BUILD_*`
options, eg. `-DBUILD_VRPN=OFF` disables building VRPN, even if it is a
dependency of another package.
