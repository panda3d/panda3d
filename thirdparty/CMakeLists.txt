# Many of this is based on the scripts in the Polycode project.  Many thanks
# to the respective authors for going through the work of figuring this out!

cmake_minimum_required(VERSION 3.3)

# Set defaults.  Must be before project().
if(APPLE)
  set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.9" CACHE STRING "")
  set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64 bits
    set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/win-libs-vc14-x64)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    # 32 bits
    set(EXECUTABLE_OUTPUT_PATH  ${PROJECT_SOURCE_DIR}/win-libs-vc14)

  endif()
endif()

project(panda3d-thirdparty)

include(ExternalProject)

set(DEFAULT_ENABLE ON)
if(DISABLE_ALL)
  set(DEFAULT_ENABLE OFF)
endif()

if(APPLE)
  set(NOT_APPLE OFF)
else()
  set(NOT_APPLE ${DEFAULT_ENABLE})
endif()

if(NOT ANDROID)
  option(BUILD_ZLIB "Build zlib" ${DEFAULT_ENABLE})
endif()
option(BUILD_PNG "Build libpng (requires zlib)" ${DEFAULT_ENABLE})
option(BUILD_ASSIMP "Build assimp (requires zlib)" ${DEFAULT_ENABLE})
option(BUILD_BULLET "Build bullet" ${DEFAULT_ENABLE})
option(BUILD_HARFBUZZ "Build harfbuzz (requires freetype)" ${DEFAULT_ENABLE})
option(BUILD_FREETYPE "Build freetype (requires harfbuzz, zlib and png)" ${DEFAULT_ENABLE})
option(BUILD_VORBIS "Build vorbis" ${DEFAULT_ENABLE})
option(BUILD_OPUS "Build opus" ${DEFAULT_ENABLE})
option(BUILD_OPENAL "Build OpenAL" ${NOT_APPLE})
option(BUILD_JPEG "Build libjpeg" ${DEFAULT_ENABLE})
option(BUILD_SQUISH "Build libsquish" ${DEFAULT_ENABLE})
option(BUILD_FCOLLADA "Build FCollada" ${DEFAULT_ENABLE})
option(BUILD_VRPN "Build VRPN" ${DEFAULT_ENABLE})
option(BUILD_TIFF "Build TIFF (requires zlib and jpeg)" ${DEFAULT_ENABLE})
option(BUILD_EIGEN "Copy Eigen headers" ${DEFAULT_ENABLE})
option(BUILD_ODE "Build ODE physics engine" ${DEFAULT_ENABLE})
option(BUILD_ARTOOLKIT "Build ARToolKit" ${DEFAULT_ENABLE})
option(BUILD_NVIDIACG "Copy NVIDIA Cg Toolkit binaries" ${DEFAULT_ENABLE})
option(BUILD_OPENSSL "Build OpenSSL" ${DEFAULT_ENABLE})
option(BUILD_OPENEXR "Build OpenEXR" ${DEFAULT_ENABLE})
option(BUILD_FFMPEG "Build FFMpeg (requires vorbis, freetype and zlib)" ${DEFAULT_ENABLE})
option(BUILD_GLSLANG "Build glslang" ${DEFAULT_ENABLE})
option(BUILD_SPIRV_CROSS "Build spirv-cross" ${DEFAULT_ENABLE})
option(BUILD_SPIRV_TOOLS "Build spirv-tools" ${DEFAULT_ENABLE})
option(BUILD_OPENCV "Build OpenCV" OFF)
option(BUILD_MIMALLOC "Build mimalloc" ${DEFAULT_ENABLE})


if(CMAKE_GENERATOR MATCHES "^Visual Studio [0-9]+ ")
  if(MSVC_TOOLSET_VERSION)
    math(EXPR VCVER "${MSVC_TOOLSET_VERSION} / 10" OUTPUT_FORMAT DECIMAL)
  elseif(NOT (MSVC_VERSION LESS 1900))
    set(VCVER 14)
  else()
    string(REPLACE " " ";" WORDS ${CMAKE_GENERATOR})
    list(GET WORDS 2 VCVER)
  endif()

  set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/win-libs-vc${VCVER})

  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(THIRDPARTY_DIR "${THIRDPARTY_DIR}-x64")
  endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/darwin-libs-a)

elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/linux-libs-arm64)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/linux-libs-x64)
  else()
    set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/linux-libs-a)
  endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/freebsd-libs-arm64)
  elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/freebsd-libs-x64)
  else()
    set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/freebsd-libs-a)
  endif()

elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(THIRDPARTY_DIR ${CMAKE_SOURCE_DIR}/android-libs-${CMAKE_ANDROID_ARCH})

else()
  message(FATAL_ERROR "Unknown platform.")
endif()

if(NOT MSVC)
  set(CMAKE_C_FLAGS "-fpic")
  set(CMAKE_CXX_FLAGS "-fpic")
endif()

message("thirdparty packages will be installed to ${THIRDPARTY_DIR}")

set(COMMON_CMAKE_ARGS
  -DCMAKE_SYSTEM_VERSION=${CMAKE_SYSTEM_VERSION}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_LINKER=${CMAKE_LINKER}
  -DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}
  -DCMAKE_INSTALL_LIBDIR=lib
  -DCMAKE_ANDROID_ARCH=${CMAKE_ANDROID_ARCH}
  -DCMAKE_ANDROID_ARCH_ABI=${CMAKE_ANDROID_ARCH_ABI}
  -DCMAKE_ANDROID_ARCH_TRIPLE=${CMAKE_ANDROID_ARCH_TRIPLE}
  -DCMAKE_ANDROID_ARCH_LLVM_TRIPLE=${CMAKE_ANDROID_ARCH_LLVM_TRIPLE}
  -DANDROID_PLATFORM=${ANDROID_PLATFORM}
  -DBUILD_SHARED_LIBS=OFF
  -DSKIP_INSTALL_EXECUTABLES=ON
  -DSKIP_INSTALL_FILES=ON
  -Wno-dev
)

if(WIN32)
  # Add tools like patch.exe and nasm.exe to the PATH
  list(APPEND CMAKE_MSVCIDE_RUN_PATH "${CMAKE_CURRENT_LIST_DIR}/win-util")
endif()

if(APPLE)
  # Prevent the semicolon from becoming a space in the command.
  string(REPLACE ";" "$<SEMICOLON>" archs "${CMAKE_OSX_ARCHITECTURES}")
  set(COMMON_CMAKE_ARGS ${COMMON_CMAKE_ARGS}
    -DCMAKE_OSX_ARCHITECTURES=${archs}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DCMAKE_OSX_SYSROOT=${CMAKE_OSX_SYSROOT}
  )
endif(APPLE)

if(ANDROID)
  # Android ships with zlib.
  add_custom_target(zlib)
  find_library(ZLIB_LIBRARY z)

elseif(BUILD_ZLIB)
  ExternalProject_Add(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG cacf7f1d4e3d44d871b605da3b647f07d718623f #v1.2.11
    #URL https://github.com/madler/zlib/archive/v1.2.11.tar.gz
    #URL_HASH SHA256=629380c90a77b964d896ed37163f5c3a34f6e6d897311f1df2a7016355c45eff

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DZLIB_TESTS=OFF
    INSTALL_DIR ${THIRDPARTY_DIR}/zlib
  )

  # Remove the shared libraries, which cannot be disabled in the CMake file.
  if(WIN32)
    ExternalProject_Add_Step(zlib remove_extras
      COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/bin"
      COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/share"
      COMMAND ${CMAKE_COMMAND} -E remove "<INSTALL_DIR>/lib/zlib.lib"
      DEPENDEES install
    )
    set(ZLIB_LIBRARY ${THIRDPARTY_DIR}/zlib/lib/zlibstatic.lib)
  elseif(APPLE)
    ExternalProject_Add_Step(zlib remove_extras
      COMMAND ${CMAKE_COMMAND} -E remove "<INSTALL_DIR>/lib/libz.dylib"
                                         "<INSTALL_DIR>/lib/libz.1.dylib"
                                         "<INSTALL_DIR>/lib/libz.1.2.11.dylib"
      DEPENDEES install
    )
    set(ZLIB_LIBRARY ${THIRDPARTY_DIR}/zlib/lib/libz.a)
  else()
    ExternalProject_Add_Step(zlib remove_extras
      COMMAND ${CMAKE_COMMAND} -E remove "<INSTALL_DIR>/lib/libz.so"
                                         "<INSTALL_DIR>/lib/libz.so.1"
                                         "<INSTALL_DIR>/lib/libz.so.1.2.11"
      DEPENDEES install
    )
    set(ZLIB_LIBRARY ${THIRDPARTY_DIR}/zlib/lib/libz.a)
  endif()
else()
  set(ZLIB_LIBRARY ZLIB_LIBRARY-NOTFOUND)
endif()

if(BUILD_PNG)
  ExternalProject_Add(
    png
    DEPENDS zlib

    # Please note that libpng 1.6.32+ requires CMake 3.0.2
    GIT_REPOSITORY https://github.com/glennrp/libpng.git
    GIT_TAG 9fcb846402d8692d25cb5286777a832ca852fa44 #v1.6.31
    #URL https://github.com/glennrp/libpng/archive/v1.6.31.tar.gz
    #URL_HASH SHA256=79c0391804c83a6e0b722c20be7c3a27aeb2fe09afbc3028abad243444c98aa0

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DCMAKE_PREFIX_PATH=${THIRDPARTY_DIR}/zlib
      -DPNG_SHARED=OFF
      -DPNG_STATIC=ON
      -DPNG_TESTS=OFF
      -DPNG_DEBUG=OFF
      -DAWK= # Needed for multi-arch build on macOS
    INSTALL_DIR ${THIRDPARTY_DIR}/png
  )

  ExternalProject_Add_Step(png remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/libpng"
    DEPENDEES install
  )
elseif(BUILD_SQUISH)
  add_custom_target(png)
endif()

if(BUILD_ASSIMP)
  ExternalProject_Add(
    assimp
    DEPENDS zlib
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG 8134cae998ec0e3a44a6ec8330b681f3981107b7 #v5.1.6

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DCMAKE_PREFIX_PATH=${THIRDPARTY_DIR}/zlib
      -DASSIMP_BUILD_TESTS=OFF
      -DASSIMP_BUILD_ASSIMP_TOOLS=OFF
      -DASSIMP_NO_EXPORT=ON
      -DLIBRARY_SUFFIX=
      -DZLIB_LIBRARY_DBG=${ZLIB_LIBRARY}
      -DZLIB_LIBRARY_REL=${ZLIB_LIBRARY}

    INSTALL_DIR ${THIRDPARTY_DIR}/assimp
  )

  ExternalProject_Add_Step(assimp remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/cmake"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/pkgconfig"
    DEPENDEES install
  )
endif()

if(BUILD_BULLET)
  ExternalProject_Add(
    bullet
    GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
    GIT_TAG 2fb92bc40c16e4da5a9018479d4a8e1899702ab8 #v2.83.7
    #URL https://github.com/bulletphysics/bullet3/archive/2.83.7.tar.gz
    #URL_HASH SHA256=919087c764d0e93555c6ef0830c8951a9caafea99f52ac8b366e96adbe0112ed

    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/patches/bullet-btvector3-shuffle.patch

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DINSTALL_LIBS=ON
      -DUSE_MSVC_RUNTIME_LIBRARY_DLL=ON
      -DBUILD_DEMOS=OFF
      -DBUILD_CPU_DEMOS=OFF
      -DBUILD_OPENGL3_DEMOS=OFF
      -DBUILD_BULLET2_DEMOS=OFF
      -DBUILD_BULLET3=OFF
      -DBUILD_EXTRAS=OFF
      -DBUILD_UNIT_TESTS=OFF
      -DINCLUDE_INSTALL_DIR:STRING=include/
    INSTALL_DIR ${THIRDPARTY_DIR}/bullet
  )

  ExternalProject_Add_Step(bullet remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/cmake"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/pkgconfig"
    DEPENDEES install
  )
endif()

if(BUILD_HARFBUZZ)
  # Ugh, freetype and harfbuzz depend on each other?!  Madness.  Well, we'll
  # have to make a temporary build of freetype so we can build HarfBuzz.
  ExternalProject_Add(
    freetype-nohb
    DEPENDS zlib
    URL http://www.panda3d.org/download/noversion/freetype-2.7.1.tar.gz
    URL_HASH SHA256=162ef25aa64480b1189cdb261228e6c5c44f212aac4b4621e28cf2157efb59f5

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DCMAKE_PREFIX_PATH=${THIRDPARTY_DIR}/zlib
      -DWITH_HarfBuzz=OFF
      -DWITH_BZip2=OFF
      -DWITH_PNG=OFF
    INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/freetype-nohb
  )

  # Use the tarball instead of git repo in order to avoid the need for ragel.
  ExternalProject_Add(
    harfbuzz
    DEPENDS freetype-nohb
    URL https://www.freedesktop.org/software/harfbuzz/release/harfbuzz-2.6.4.tar.xz
    URL_HASH SHA256=9413b8d96132d699687ef914ebb8c50440efc87b3f775d25856d7ec347c03c12

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DCMAKE_PREFIX_PATH=${CMAKE_CURRENT_BINARY_DIR}/freetype-nohb
      -DHB_HAVE_FREETYPE=ON
      -DHB_BUILD_TESTS=OFF
      -DHB_BUILD_UTILS=OFF
      -DHB_BUILD_SUBSET=OFF
      -DHB_HAVE_INTROSPECTION=OFF
      -DFREETYPE_LIBRARY=${CMAKE_CURRENT_BINARY_DIR}/freetype-nohb/lib/${CMAKE_STATIC_LIBRARY_PREFIX}freetype${CMAKE_STATIC_LIBRARY_SUFFIX}
      -DFREETYPE_INCLUDE_DIR_freetype2=${CMAKE_CURRENT_BINARY_DIR}/freetype-nohb/include/freetype2
      -DFREETYPE_INCLUDE_DIR_ft2build=${CMAKE_CURRENT_BINARY_DIR}/freetype-nohb/include/freetype2
    INSTALL_DIR ${THIRDPARTY_DIR}/harfbuzz
  )

  ExternalProject_Add_Step(harfbuzz move_headers
    COMMAND ${CMAKE_COMMAND} -E rename "<INSTALL_DIR>/include/harfbuzz" "<INSTALL_DIR>/include-tmp"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/include"
    COMMAND ${CMAKE_COMMAND} -E rename "<INSTALL_DIR>/include-tmp" "<INSTALL_DIR>/include"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/cmake"
    DEPENDEES install
  )
endif()

if(BUILD_FREETYPE)
  ExternalProject_Add(
    freetype
    DEPENDS zlib harfbuzz
    URL http://www.panda3d.org/download/noversion/freetype-2.7.1.tar.gz
    URL_HASH SHA256=162ef25aa64480b1189cdb261228e6c5c44f212aac4b4621e28cf2157efb59f5

    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/patches/freetype-2.7.1-harfbuzz-icu.patch

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DCMAKE_PREFIX_PATH=${THIRDPARTY_DIR}/zlib;${THIRDPARTY_DIR}/harfbuzz
      -DWITH_HarfBuzz=ON
      -DWITH_BZip2=OFF
      -DWITH_PNG=OFF
      -DHARFBUZZ_INCLUDE_DIRS=${THIRDPARTY_DIR}/harfbuzz/include
      -DHARFBUZZ_LIBRARIES=${THIRDPARTY_DIR}/harfbuzz/lib/${CMAKE_STATIC_LIBRARY_PREFIX}harfbuzz${CMAKE_STATIC_LIBRARY_SUFFIX}
      -DHARFBUZZ_ICU_LIBRARIES=${THIRDPARTY_DIR}/harfbuzz/lib/${CMAKE_STATIC_LIBRARY_PREFIX}harfbuzz${CMAKE_STATIC_LIBRARY_SUFFIX}
      -DPC_HARFBUZZ_INCLUDEDIR=${THIRDPARTY_DIR}/harfbuzz/include
      -DPC_HARFBUZZ_LIBDIR=${THIRDPARTY_DIR}/harfbuzz/lib
    INSTALL_DIR ${THIRDPARTY_DIR}/freetype
  )
endif()

if(BUILD_VORBIS)
  ExternalProject_Add(
    ogg
    GIT_REPOSITORY https://github.com/xiph/ogg.git
    GIT_TAG dd85929dbe38be4b3876c9c0d6d5dcb7a128f388

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
    INSTALL_DIR ${THIRDPARTY_DIR}/vorbis
  )

  ExternalProject_Add(
    vorbis
    DEPENDS ogg
    GIT_REPOSITORY https://github.com/xiph/vorbis.git
    GIT_TAG 8ef0f8058854b2ef55d2d42bbe84487a9aadae12

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DOGG_ROOT=${THIRDPARTY_DIR}/vorbis
      -DOGG_INCLUDE_DIRS=${THIRDPARTY_DIR}/vorbis/include
      -DOGG_LIBRARIES=${THIRDPARTY_DIR}/vorbis/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ogg${CMAKE_STATIC_LIBRARY_SUFFIX}
    INSTALL_DIR ${THIRDPARTY_DIR}/vorbis
  )

  ExternalProject_Add_Step(vorbis remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/pkgconfig"
    DEPENDEES install
  )
endif()

if(BUILD_OPUS)
  ExternalProject_Add(
    ogg-opus
    GIT_REPOSITORY https://github.com/xiph/ogg.git
    GIT_TAG dd85929dbe38be4b3876c9c0d6d5dcb7a128f388

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
    INSTALL_DIR ${THIRDPARTY_DIR}/opus
  )

  ExternalProject_Add(
    opus
    GIT_REPOSITORY https://github.com/xnorpx/opus.git
    GIT_TAG e58822ef814b9bd9baeabecd7024aa7a6d4b5adf # PR#100

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
    INSTALL_DIR ${THIRDPARTY_DIR}/opus
  )

  ExternalProject_Add(
    opusfile
    DEPENDS ogg-opus opus
    GIT_REPOSITORY https://github.com/xiph/opusfile.git
    GIT_TAG b7c2ed3bafd9b08cd5d276894ca18fb58bcbfbc9 #0.11

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/libopusfile.cmake <SOURCE_DIR>/CMakeLists.txt

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DOGG_ROOT=${THIRDPARTY_DIR}/opus
      -DOGG_INCLUDE_DIRS=${THIRDPARTY_DIR}/opus/include
      -DOGG_LIBRARIES=${THIRDPARTY_DIR}/opus/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ogg${CMAKE_STATIC_LIBRARY_SUFFIX}
      -DOPUS_ROOT=${THIRDPARTY_DIR}/opus
      -DOPUS_INCLUDE_DIRS=${THIRDPARTY_DIR}/opus/include/opus
      -DOPUS_LIBRARIES=${THIRDPARTY_DIR}/opus/lib/${CMAKE_STATIC_LIBRARY_PREFIX}opus${CMAKE_STATIC_LIBRARY_SUFFIX}
    INSTALL_DIR ${THIRDPARTY_DIR}/opus
  )

  ExternalProject_Add_Step(ogg-opus remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/pkgconfig"
    DEPENDEES install
  )
endif()

if(BUILD_OPENAL)
  ExternalProject_Add(
    openal
    GIT_REPOSITORY https://github.com/kcat/openal-soft.git
    GIT_TAG ae4eacf147e2c2340cc4e02a790df04c793ed0a9 # 1.21.1

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DLIBTYPE=STATIC
      -DALSOFT_HRTF_DEFS=OFF
      -DALSOFT_AMBDEC_PRESETS=OFF
      -DALSOFT_CONFIG=OFF
      -DALSOFT_NO_CONFIG_UTIL=ON
      -DALSOFT_EXAMPLES=OFF
      -DALSOFT_TESTS=OFF
      -DALSOFT_UTILS=OFF
      -DALSOFT_ENABLE_SSE2_CODEGEN=OFF      # for only 32-bit
    INSTALL_DIR ${THIRDPARTY_DIR}/openal
  )

  ExternalProject_Add_Step(openal remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/cmake"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/pkgconfig"
    DEPENDEES install
  )
endif()

if(BUILD_JPEG)
  ExternalProject_Add(
    jpeg
    URL http://www.panda3d.org/download/noversion/jpegsrc.v9b.tar.gz
    URL_HASH SHA256=566241ad815df935390b341a5d3d15a73a4000e5aab40c58505324c2855cbbb8

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/libjpeg.cmake <SOURCE_DIR>/CMakeLists.txt
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/libjpeg-jconfig.h.cmake <SOURCE_DIR>/jconfig.h.cmake

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DBUILD_STATIC=ON
      -DBUILD_EXECUTABLES=OFF
      -DBUILD_TESTS=OFF
    INSTALL_DIR ${THIRDPARTY_DIR}/jpeg
  )

  if(WIN32)
    set(JPEG_LIBRARY ${THIRDPARTY_DIR}/jpeg/lib/jpeg-static.lib)
  else()
    set(JPEG_LIBRARY ${THIRDPARTY_DIR}/jpeg/lib/libjpeg.a)
  endif()
else()
  set(JPEG_LIBRARY JPEG_LIBRARY-NOTFOUND)
endif()

if(BUILD_SQUISH)
  ExternalProject_Add(
    squish
    DEPENDS png
    URL https://storage.googleapis.com/google-code-archive-downloads/v2/code.google.com/libsquish/squish-1.10.tar.gz
    URL_HASH SHA256=aec3428badafdbb3b0ed1ea3a385a8029e65694282459f61b4a538963e8cada3

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/libsquish.cmake <SOURCE_DIR>/CMakeLists.txt
          COMMAND patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/patches/squish-1.10-sse.patch

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DCMAKE_PREFIX_PATH=${THIRDPARTY_DIR}/png
    INSTALL_DIR ${THIRDPARTY_DIR}/squish
  )
endif()

if(BUILD_FCOLLADA)
  ExternalProject_Add(
    fcollada
    GIT_REPOSITORY https://github.com/rdb/fcollada.git
    GIT_TAG b322c8fa82aa13f9424ffc82b493cb442ce6925e

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DBUILD_UNICODE=OFF
      -DBUILD_TESTS=OFF
    INSTALL_DIR ${THIRDPARTY_DIR}/fcollada
  )
endif()

if(BUILD_VRPN)
  ExternalProject_Add(
    vrpn
    # Pull from git since 7.33 has some compile issues
    GIT_REPOSITORY https://github.com/vrpn/vrpn.git
    GIT_TAG 9cfc1a536f39e8815066c39a1cef0f8a515a2860

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DQUATLIB_BUILD_EXAMPLES=OFF
      -DVRPN_BUILD_CLIENTS=OFF
      -DVRPN_BUILD_CLIENT_LIBRARY=ON
      -DVRPN_BUILD_JAVA=OFF
      -DVRPN_BUILD_PYTHON=OFF
      -DVRPN_BUILD_PYTHON_HANDCODED_2X=OFF
      -DVRPN_BUILD_PYTHON_HANDCODED_3X=OFF
      -DVRPN_BUILD_SERVERS=OFF
      -DVRPN_BUILD_SERVER_LIBRARY=OFF
      -DBUILD_TESTING=FALSE
    INSTALL_DIR ${THIRDPARTY_DIR}/vrpn
  )
endif()

if(BUILD_TIFF)
  ExternalProject_Add(
    tiff
    DEPENDS zlib jpeg
    URL http://download.osgeo.org/libtiff/tiff-4.0.6.tar.gz
    URL_HASH SHA256=4d57a50907b510e3049a4bba0d7888930fdfc16ce49f1bf693e5b6247370d68c

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DCMAKE_PREFIX_PATH=${THIRDPARTY_DIR}/zlib;${THIRDPARTY_DIR}/jpeg
      -DJPEG_INCLUDE_DIR=${THIRDPARTY_DIR}/jpeg/include
      -DJPEG_LIBRARY=${JPEG_LIBRARY}
      -Dzlib=ON -Djpeg=ON -Dlzma=OFF -Dcxx=OFF
    INSTALL_DIR ${THIRDPARTY_DIR}/tiff
  )

  # Remove the extra bin and share directories it created.
  ExternalProject_Add_Step(tiff remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/bin"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/share"
    DEPENDEES install
  )
endif()

if(BUILD_EIGEN)
  ExternalProject_Add(
    eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.3.9/eigen-3.3.9.tar.gz
    URL_HASH SHA256=7985975b787340124786f092b3a07d594b2e9cd53bbfe5f3d9b1daee7d55f56f

    # Suppresses warning on MSVC
    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/patches/eigen-3.3.9-bad-alloc.patch

    # No need to build anything; just copy over the Eigen directory.
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""

    INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/Eigen <INSTALL_DIR>/include/Eigen
            COMMAND ${CMAKE_COMMAND} -E copy_if_different <SOURCE_DIR>/signature_of_eigen3_matrix_library <INSTALL_DIR>/include/signature_of_eigen3_matrix_library
    INSTALL_DIR ${THIRDPARTY_DIR}/eigen
  )
endif()

if(BUILD_ODE)
  ExternalProject_Add(
    ode
    URL https://bitbucket.org/odedevs/ode/downloads/ode-0.13.1.tar.gz
    URL_HASH SHA256=35e55e05c6c6ebb813a546017285a7aceedb3e8e55c8ff102e80e26bd84c5658

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/libode.cmake <SOURCE_DIR>/CMakeLists.txt
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/libode-config.h.cmake <SOURCE_DIR>/ode/src/config.h.cmake

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
    INSTALL_DIR ${THIRDPARTY_DIR}/ode
  )
endif()

if(BUILD_ARTOOLKIT)
  ExternalProject_Add(
    artoolkit
    URL http://www.panda3d.org/download/noversion/ARToolKit-2.72.1.tgz
    URL_HASH SHA256=1a49a67c6e196b0e9990e90255ce840287ac47b5a53371a68a6dd93a5425d259

    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/artoolkit.cmake <SOURCE_DIR>/CMakeLists.txt
          COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/artoolkit-config.h.cmake <SOURCE_DIR>/include/AR/config.h.cmake

    CMAKE_ARGS ${COMMON_CMAKE_ARGS} -DCMAKE_OSX_ARCHITECTURES=x86_64
    INSTALL_DIR ${THIRDPARTY_DIR}/artoolkit
  )
endif()

if(BUILD_NVIDIACG AND CMAKE_SYSTEM_NAME STREQUAL "Windows")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    ExternalProject_Add(
      nvidiacg
      URL http://www.panda3d.org/download/noversion/nvidiacg-win64.zip
      URL_HASH SHA256=955d6e38bcba7a48e0efc8f2f0f7d5de39901788806e411744c63b55c83edfe2
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib <INSTALL_DIR>/lib
              COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/bin <INSTALL_DIR>/bin
              COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include <INSTALL_DIR>/include
      INSTALL_DIR ${THIRDPARTY_DIR}/nvidiacg
    )
  else()
    ExternalProject_Add(
      nvidiacg
      URL http://www.panda3d.org/download/noversion/nvidiacg-win32.zip
      URL_HASH SHA256=602ae301f3b68953a56a81ba89db0f53c0e10e8350faeb27842c5b827d9783a3
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib <INSTALL_DIR>/lib
              COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/bin <INSTALL_DIR>/bin
              COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include <INSTALL_DIR>/include
      INSTALL_DIR ${THIRDPARTY_DIR}/nvidiacg
    )
  endif()
elseif(BUILD_NVIDIACG AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  ExternalProject_Add(
    nvidiacg
    URL http://www.panda3d.org/download/noversion/NVIDIA_Cg.tgz
    URL_HASH SHA256=cd6b12cca647749101b0286353095c7435251097d1aed0171cfc8afdcededd29

    CONFIGURE_COMMAND chmod -R u+w <SOURCE_DIR>
    BUILD_COMMAND install_name_tool -id @loader_path/../Frameworks/Cg.framework/Cg <SOURCE_DIR>/Library/Frameworks/Cg.framework/Cg
    INSTALL_COMMAND rm -rf "<INSTALL_DIR>/Cg.framework"
            COMMAND cp -R <SOURCE_DIR>/Library/Frameworks/Cg.framework <INSTALL_DIR>/Cg.framework

    INSTALL_DIR ${THIRDPARTY_DIR}/nvidiacg
  )
elseif(BUILD_NVIDIACG AND CMAKE_SYSTEM_NAME STREQUAL "Linux")
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    ExternalProject_Add(
      nvidiacg
      URL http://developer.download.nvidia.com/cg/Cg_3.1/Cg-3.1_April2012_x86_64.tgz
      URL_HASH SHA256=e8ff01e6cc38d1b3fd56a083f5860737dbd2f319a39037528fb1a74a89ae9878
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib64 <INSTALL_DIR>/lib
              COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include <INSTALL_DIR>/include
      INSTALL_DIR ${THIRDPARTY_DIR}/nvidiacg
    )
  else()
    ExternalProject_Add(
      nvidiacg
      URL http://developer.download.nvidia.com/cg/Cg_3.1/Cg-3.1_April2012_x86.tgz
      URL_HASH SHA256=cef3591e436f528852db0e8c145d3842f920e0c89bcfb219c466797cb7b18879
      CONFIGURE_COMMAND ""
      BUILD_COMMAND ""
      INSTALL_COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/lib <INSTALL_DIR>/lib
              COMMAND ${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/include <INSTALL_DIR>/include
      INSTALL_DIR ${THIRDPARTY_DIR}/nvidiacg
    )
  endif()
endif()

if(BUILD_OPENSSL AND MSVC)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ssl_config VC-WIN64A)
  else()
    set(ssl_config VC-WIN32)
  endif()

  ExternalProject_Add(
    openssl
    URL https://www.openssl.org/source/openssl-1.1.1m.tar.gz
    URL_HASH SHA256=f89199be8b23ca45fc7cb9f1d8d3ee67312318286ad030f5316aca6462db6c96

    CONFIGURE_COMMAND perl Configure ${ssl_config} no-shared --prefix=<INSTALL_DIR>
    BUILD_COMMAND nmake build_libs
    BUILD_IN_SOURCE 1

    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/patches/openssl-1.1.1-no-pdb.patch

    INSTALL_COMMAND nmake install_dev
    INSTALL_DIR ${THIRDPARTY_DIR}/openssl
  )

  ExternalProject_Add_Step(openssl remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/bin"
    DEPENDEES install
  )
elseif(BUILD_OPENSSL AND NOT APPLE)
  ExternalProject_Add(
    openssl
    URL https://www.openssl.org/source/openssl-1.1.1m.tar.gz
    URL_HASH SHA256=f89199be8b23ca45fc7cb9f1d8d3ee67312318286ad030f5316aca6462db6c96

    CONFIGURE_COMMAND ./config no-shared --prefix=<INSTALL_DIR> ${CMAKE_C_FLAGS}
    BUILD_COMMAND ${MAKE}
    BUILD_IN_SOURCE 1

    INSTALL_DIR ${THIRDPARTY_DIR}/openssl
  )

  ExternalProject_Add_Step(openssl remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/bin"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/ssl"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/pkgconfig"
    DEPENDEES install
  )
elseif(BUILD_OPENSSL AND APPLE)
  set(ssl_flags ${CMAKE_C_FLAGS}
                no-shared no-dso
                -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}
                -Wno-nullability-completeness -Wno-expansion-to-defined)

  if(CMAKE_OSX_SYSROOT)
    set(ssl_flags ${ssl_flags} -isysroot${CMAKE_OSX_SYSROOT} -I/usr/include -Wl,-syslibroot,${CMAKE_OSX_SYSROOT})
  endif()

  if("arm64" IN_LIST CMAKE_OSX_ARCHITECTURES)
    ExternalProject_Add(
      openssl-arm64
      URL https://www.openssl.org/source/openssl-1.1.1m.tar.gz
      URL_HASH SHA256=f89199be8b23ca45fc7cb9f1d8d3ee67312318286ad030f5316aca6462db6c96

      CONFIGURE_COMMAND /usr/bin/perl ./Configure darwin64-arm64-cc --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR> ${ssl_flags}
      BUILD_COMMAND ${MAKE}
      BUILD_IN_SOURCE 1

      INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl-arm64
    )
  endif()

  if("x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES)
    ExternalProject_Add(
      openssl-x86_64
      URL https://www.openssl.org/source/openssl-1.1.1m.tar.gz
      URL_HASH SHA256=f89199be8b23ca45fc7cb9f1d8d3ee67312318286ad030f5316aca6462db6c96

      CONFIGURE_COMMAND /usr/bin/perl ./Configure darwin64-x86_64-cc --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR> ${ssl_flags}
      BUILD_COMMAND ${MAKE}
      BUILD_IN_SOURCE 1

      INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl-x86_64
    )
  endif()

  if("i386" IN_LIST CMAKE_OSX_ARCHITECTURES)
    ExternalProject_Add(
      openssl-i386
      URL https://www.openssl.org/source/openssl-1.1.1m.tar.gz
      URL_HASH SHA256=f89199be8b23ca45fc7cb9f1d8d3ee67312318286ad030f5316aca6462db6c96

      CONFIGURE_COMMAND /usr/bin/perl ./Configure darwin-i386-cc --prefix=<INSTALL_DIR> --openssldir=<INSTALL_DIR> ${ssl_flags}
      BUILD_COMMAND ${MAKE}
      BUILD_IN_SOURCE 1

      INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/openssl-i386
    )
  endif()

  list(LENGTH CMAKE_OSX_ARCHITECTURES num_archs)
  list(GET CMAKE_OSX_ARCHITECTURES 0 first_arch)

  if(num_archs EQUAL 1)
    set(libcrypto_command ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/openssl-${first_arch}/lib/libcrypto.a" "${THIRDPARTY_DIR}/openssl/lib/libcrypto.a")
    set(libssl_command ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/openssl-${first_arch}/lib/libssl.a" "${THIRDPARTY_DIR}/openssl/lib/libssl.a")
  else()
    set(libcrypto_command lipo -create -output "${THIRDPARTY_DIR}/openssl/lib/libcrypto.a")
    set(libssl_command lipo -create -output "${THIRDPARTY_DIR}/openssl/lib/libssl.a")
    foreach(arch ${CMAKE_OSX_ARCHITECTURES})
      set(libcrypto_command ${libcrypto_command} -arch ${arch} "${CMAKE_CURRENT_BINARY_DIR}/openssl-${arch}/lib/libcrypto.a")
      set(libssl_command ${libssl_command} -arch ${arch} "${CMAKE_CURRENT_BINARY_DIR}/openssl-${arch}/lib/libssl.a")
    endforeach()
  endif()

  add_custom_target(openssl ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${THIRDPARTY_DIR}/openssl/lib"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/openssl-${first_arch}/include" "${THIRDPARTY_DIR}/openssl/include"
    COMMAND ${libcrypto_command}
    COMMAND ${libssl_command}
  )
  foreach(arch ${CMAKE_OSX_ARCHITECTURES})
    add_dependencies(openssl openssl-${arch})
  endforeach()
endif()

if(BUILD_OPENEXR)
  ExternalProject_Add(
    openexr
    DEPENDS zlib
    GIT_REPOSITORY https://github.com/AcademySoftwareFoundation/openexr.git
    GIT_TAG e94d82b2165da4e2af167aa74630906202e3d43b #v3.1.4

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DCMAKE_PREFIX_PATH=${THIRDPARTY_DIR}/zlib
      -DBUILD_TESTING=OFF
      -DOPENEXR_FORCE_INTERNAL_IMATH=ON
      -DOPENEXR_LIB_SUFFIX=
      -DOPENEXR_BUILD_TOOLS=OFF
      -DOPENEXR_INSTALL_TOOLS=OFF
      -DOPENEXR_INSTALL_EXAMPLES=OFF
      -DOPENEXR_INSTALL_PKG_CONFIG=OFF
      -DIMATH_LIB_SUFFIX=
      -DIMATH_INSTALL_PKG_CONFIG=OFF
    INSTALL_DIR ${THIRDPARTY_DIR}/openexr
  )
endif()

if(BUILD_FFMPEG AND APPLE)
  set(ffmpeg_osx_flags "${ffmpeg_osx_flags} -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} -isysroot ${CMAKE_OSX_SYSROOT}")

  foreach(arch ${CMAKE_OSX_ARCHITECTURES})
    if(arch STREQUAL "arm64")
      set(configure_flags --enable-cross-compile --arch=arm64 --target-os=darwin)
    else()
      set(configure_flags)
    endif()

    ExternalProject_Add(
      ffmpeg-${arch}
      DEPENDS ogg vorbis freetype zlib
      URL https://ffmpeg.org/releases/ffmpeg-4.4.1.tar.gz
      URL_HASH SHA256=82c98f74777f623710b72f9a3389fd38c1ed93bc661107e65df19234e395f6b9

      CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR> --datadir=${CMAKE_BINARY_DIR}/etc --disable-shared --enable-static --enable-pic --disable-doc --disable-debug --disable-programs --disable-outdevs --enable-runtime-cpudetect ${configure_flags}
        "--cc=${CMAKE_C_COMPILER}"
        "--cxx=${CMAKE_CXX_COMPILER}"
        "--ar=${CMAKE_C_COMPILER_AR}"
        "--ranlib=${CMAKE_C_COMPILER_RANLIB}"
        "--nm=${CMAKE_NM}"
        "--strip=${CMAKE_STRIP}"
        "--extra-cflags=${CMAKE_C_FLAGS} ${ffmpeg_osx_flags} -arch ${arch} -Wno-attributes"
        "--extra-cxxflags=${CMAKE_CXX_FLAGS} ${ffmpeg_osx_flags} -arch ${arch} -Wno-attributes"
        "--extra-ldflags=${ffmpeg_osx_flags} -arch ${arch}"
      BUILD_COMMAND ${MAKE}
      BUILD_IN_SOURCE 1

      INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/ffmpeg-${arch}
    )
  endforeach()

  list(LENGTH CMAKE_OSX_ARCHITECTURES num_archs)
  list(GET CMAKE_OSX_ARCHITECTURES 0 first_arch)

  foreach(lib avcodec avdevice avfilter avformat avutil swresample swscale)
    if(num_archs EQUAL 1)
      set(lib${lib}_command ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg-${first_arch}/lib/lib${lib}.a" "${THIRDPARTY_DIR}/openssl/lib/lib${lib}.a")
    else()
      set(lib${lib}_command lipo -create -output "${THIRDPARTY_DIR}/ffmpeg/lib/lib${lib}.a")
      foreach(arch ${CMAKE_OSX_ARCHITECTURES})
        set(lib${lib}_command ${lib${lib}_command} -arch ${arch} "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg-${arch}/lib/lib${lib}.a")
      endforeach()
    endif()
  endforeach()

  add_custom_target(ffmpeg ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "${THIRDPARTY_DIR}/ffmpeg/lib"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_BINARY_DIR}/ffmpeg-${first_arch}/include" "${THIRDPARTY_DIR}/ffmpeg/include"
    COMMAND ${libavcodec_command}
    COMMAND ${libavdevice_command}
    COMMAND ${libavformat_command}
    COMMAND ${libavutil_command}
    COMMAND ${libswresample_command}
    COMMAND ${libswscale_command}
  )
  foreach(arch ${CMAKE_OSX_ARCHITECTURES})
    add_dependencies(ffmpeg ffmpeg-${arch})
  endforeach()

elseif(BUILD_FFMPEG AND ANDROID)
  if(CMAKE_ANDROID_ARCH STREQUAL "arm")
    set(ffmpeg_arch armv7-a)
    set(ffmpeg_target armv7a-linux-androideabi${CMAKE_SYSTEM_VERSION})
  else()
    set(ffmpeg_arch ${CMAKE_ANDROID_ARCH})
    set(ffmpeg_target ${CMAKE_ANDROID_ARCH_TRIPLE}${CMAKE_SYSTEM_VERSION})
  endif()

  ExternalProject_Add(
    ffmpeg
    DEPENDS ogg vorbis freetype zlib
    URL https://ffmpeg.org/releases/ffmpeg-4.4.1.tar.gz
    URL_HASH SHA256=82c98f74777f623710b72f9a3389fd38c1ed93bc661107e65df19234e395f6b9

    CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR> --datadir=${CMAKE_BINARY_DIR}/etc --disable-shared --enable-static --enable-pic --disable-doc --disable-debug --disable-programs --disable-outdevs --enable-runtime-cpudetect --enable-cross-compile --arch=${ffmpeg_arch} --target-os=android
        "--cc=${CMAKE_C_COMPILER}"
        "--cxx=${CMAKE_CXX_COMPILER}"
        "--ar=${CMAKE_AR}"
        "--ranlib=${CMAKE_RANLIB}"
        "--nm=${CMAKE_NM}"
        "--strip=${CMAKE_STRIP}"
        "--extra-cflags=${CMAKE_C_FLAGS} -target ${ffmpeg_target}  -Wno-attributes"
        "--extra-cxxflags=${CMAKE_CXX_FLAGS} -target ${ffmpeg_target} -Wno-attributes"
        "--extra-ldflags=-target ${ffmpeg_target}"
    BUILD_COMMAND ${MAKE}
    BUILD_IN_SOURCE 1

    INSTALL_DIR ${THIRDPARTY_DIR}/ffmpeg
  )

  ExternalProject_Add_Step(ffmpeg remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/pkgconfig"
    DEPENDEES install
  )
elseif(BUILD_FFMPEG AND NOT WIN32)
  ExternalProject_Add(
    ffmpeg
    DEPENDS ogg vorbis freetype zlib
    URL https://ffmpeg.org/releases/ffmpeg-4.4.1.tar.gz
    URL_HASH SHA256=82c98f74777f623710b72f9a3389fd38c1ed93bc661107e65df19234e395f6b9

    CONFIGURE_COMMAND ./configure --prefix=<INSTALL_DIR> --datadir=${CMAKE_BINARY_DIR}/etc --disable-shared --enable-static --enable-pic --disable-doc --disable-debug --disable-programs --disable-outdevs --enable-runtime-cpudetect
        "--cc=${CMAKE_C_COMPILER}"
        "--cxx=${CMAKE_CXX_COMPILER}"
        "--ar=${CMAKE_C_COMPILER_AR}"
        "--ranlib=${CMAKE_C_COMPILER_RANLIB}"
        "--nm=${CMAKE_NM}"
        "--strip=${CMAKE_STRIP}"
        "--extra-cflags=${CMAKE_C_FLAGS} -Wno-attributes"
        "--extra-cxxflags=${CMAKE_CXX_FLAGS} -Wno-attributes"
    BUILD_COMMAND ${MAKE}
    BUILD_IN_SOURCE 1

    INSTALL_DIR ${THIRDPARTY_DIR}/ffmpeg
  )

  ExternalProject_Add_Step(ffmpeg remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/pkgconfig"
    DEPENDEES install
  )
endif()

if(BUILD_GLSLANG)
  ExternalProject_Add(
    glslang
    GIT_REPOSITORY https://github.com/KhronosGroup/glslang.git
    GIT_TAG 6dc24ffa47174662a2599cc033069a10b77b46b8

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DBUILD_EXTERNAL=OFF
      -DBUILD_SHARED_LIBS=OFF
      -DBUILD_TESTING=OFF
      -DENABLE_EXCEPTIONS=OFF
      -DENABLE_GLSLANG_BINARIES=OFF
      -DENABLE_HLSL=ON
      -DENABLE_RTTI=OFF
      -DENABLE_SPVREMAPPER=OFF
    INSTALL_DIR ${THIRDPARTY_DIR}/glslang
  )
endif()

if(BUILD_SPIRV_CROSS)
  ExternalProject_Add(
    spirv-cross
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
    GIT_TAG d2e1e7ba989135618f82bfeb6d97695d0d9bb9f5

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DSPIRV_CROSS_CLI=OFF
      -DSPIRV_CROSS_ENABLE_CPP=OFF
      -DSPIRV_CROSS_ENABLE_C_API=OFF
      -DSPIRV_CROSS_ENABLE_GLSL=ON
      -DSPIRV_CROSS_ENABLE_HLSL=ON
      -DSPIRV_CROSS_ENABLE_MSL=ON
      -DSPIRV_CROSS_ENABLE_REFLECT=OFF
      -DSPIRV_CROSS_ENABLE_TESTS=OFF
      -DSPIRV_CROSS_ENABLE_UTIL=OFF
      -DSPIRV_CROSS_EXCEPTIONS_TO_ASSERTIONS=ON
      -DSPIRV_CROSS_FORCE_PIC=ON
      -DSPIRV_CROSS_SHARED=OFF
      -DSPIRV_CROSS_STATIC=ON
    INSTALL_DIR ${THIRDPARTY_DIR}/spirv-cross
  )

  ExternalProject_Add_Step(spirv-cross remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/share"
    DEPENDEES install
  )
endif()

if(BUILD_SPIRV_TOOLS)
  ExternalProject_Add(
    spirv-headers
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Headers.git
    GIT_TAG 75b30a659c8a4979104986652c54cc421fc51129

    BUILD_COMMAND cmake -E echo "Skipping build step."
    INSTALL_COMMAND cmake -E echo "Skipping install step."
    EXCLUDE_FROM_ALL TRUE
  )

  ExternalProject_Add(
    spirv-tools
    DEPENDS spirv-headers
    GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
    GIT_TAG c79edd260c2b503f0eca57310057b4a100999cc5

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DSPIRV-Headers_SOURCE_DIR=${CMAKE_CURRENT_BINARY_DIR}/spirv-headers-prefix/src/spirv-headers
      -DSPIRV_BUILD_COMPRESSION=OFF
      -DSPIRV_SKIP_EXECUTABLES=ON
      -DSPIRV_SKIP_TESTS=ON
    INSTALL_DIR ${THIRDPARTY_DIR}/spirv-tools
  )

  ExternalProject_Add_Step(spirv-tools remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/pkgconfig"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/SPIRV-Tools"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/SPIRV-Tools-opt"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/SPIRV-Tools-reduce"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/SPIRV-Tools-link"
    DEPENDEES install
  )
endif()

if(BUILD_OPENCV)
  ExternalProject_Add(
    opencv
    GIT_REPOSITORY https://github.com/opencv/opencv.git
    GIT_TAG 6484732509400ff1cfed4ba44b4c8a1a11799e40 # 2.4.3

    PATCH_COMMAND patch -p1 < ${CMAKE_CURRENT_LIST_DIR}/patches/opencv-2.4.3-highgui-avfoundation.patch

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DBUILD_SHARED_LIBS=ON
      -DWITH_CARBON=OFF
      -DWITH_COCOA=ON
      -DBUILD_ZLIB=OFF
      -DPYTHON_USE_NUMPY=OFF
      -DBUILD_DOCS=OFF
      -DBUILD_TESTS=OFF
      -DBUILD_PERF_TESTS=OFF
      -DWITH_CUDA=OFF
      -DENABLE_PRECOMPILED_HEADERS=OFF
      -DBUILD_opencv_calib3d=OFF
      -DBUILD_opencv_contrib=OFF
      -DBUILD_opencv_core=ON
      -DBUILD_opencv_features2d=OFF
      -DBUILD_opencv_flann=OFF
      -DBUILD_opencv_gpu=OFF
      -DBUILD_opencv_highgui=ON
      -DBUILD_opencv_imgproc=ON
      -DBUILD_opencv_legacy=OFF
      -DBUILD_opencv_ml=OFF
      -DBUILD_opencv_nonfree=OFF
      -DBUILD_opencv_objdetect=OFF
      -DBUILD_opencv_photo=OFF
      -DBUILD_opencv_python=OFF
      -DBUILD_opencv_stitching=OFF
      -DBUILD_opencv_ts=OFF
      -DBUILD_opencv_video=OFF
      -DBUILD_opencv_videostab=OFF
    INSTALL_DIR ${THIRDPARTY_DIR}/opencv
  )

  ExternalProject_Add_Step(opencv remove_extras
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/bin"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/share"
    COMMAND ${CMAKE_COMMAND} -E remove_directory "<INSTALL_DIR>/lib/pkgconfig"
    DEPENDEES install
  )
endif()

if(BUILD_MIMALLOC)
  ExternalProject_Add(
    mimalloc
    GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
    GIT_TAG f412df7a2b64421e1f1d61fde6055a6ea288e8f5 # v2.0.3

    CMAKE_ARGS ${COMMON_CMAKE_ARGS}
      -DMI_BUILD_SHARED=OFF
      -DMI_BUILD_OBJECT=OFF
      -DMI_BUILD_TESTS=OFF
      -DMI_BUILD_STATIC=ON
      -DMI_INSTALL_TOPLEVEL=ON
      -DMI_OVERRIDE=OFF
    INSTALL_DIR ${THIRDPARTY_DIR}/mimalloc
  )
endif()
