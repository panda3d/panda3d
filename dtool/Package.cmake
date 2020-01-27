set(_thirdparty_dir_default "${PROJECT_SOURCE_DIR}/thirdparty")
if(NOT (APPLE OR WIN32) OR NOT IS_DIRECTORY "${_thirdparty_dir_default}")
  set(_thirdparty_dir_default "")
endif()

set(THIRDPARTY_DIRECTORY "${_thirdparty_dir_default}" CACHE PATH
  "Optional location of a makepanda-style thirdparty directory. All libraries
   located here will be prioritized over system libraries. Useful for
   cross-compiling.")

set(THIRDPARTY_DLLS)

if(THIRDPARTY_DIRECTORY)
  # This policy is necessary for PackageName_ROOT variables to be respected
  if(POLICY CMP0074)
    cmake_policy(GET CMP0074 _policy_cmp0074)
  endif()

  if(NOT _policy_cmp0074 STREQUAL "NEW")
    message(FATAL_ERROR
      "Your version of CMake is too old; please upgrade or unset THIRDPARTY_DIRECTORY to continue.")
  endif()

  # Dig up the actual "libs" directory
  if(APPLE)
    set(_package_dir "${THIRDPARTY_DIRECTORY}/darwin-libs-a")

    # Make sure thirdparty has the first shot, not system frameworks
    set(CMAKE_FIND_FRAMEWORK LAST)

  elseif(WIN32)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
      set(_package_dir "${THIRDPARTY_DIRECTORY}/win-libs-vc14-x64")

      file(GLOB _python_dirs "${THIRDPARTY_DIRECTORY}/win-python*-x64")
    else()
      set(_package_dir "${THIRDPARTY_DIRECTORY}/win-libs-vc14")

      file(GLOB _python_dirs "${THIRDPARTY_DIRECTORY}/win-python*")
    endif()

    list(REVERSE _python_dirs) # Descending order of version
    if(NOT DEFINED Python_ROOT)
      set(Python_ROOT "${_python_dirs}")
    endif()

    set(BISON_ROOT "${THIRDPARTY_DIRECTORY}/win-util")
    set(FLEX_ROOT "${THIRDPARTY_DIRECTORY}/win-util")

  else()
    message(FATAL_ERROR
      "You can't use THIRDPARTY_DIRECTORY on this platform. Unset it to continue.")

  endif()

  if(NOT EXISTS "${_package_dir}")
    message(FATAL_ERROR
      "Either your THIRDPARTY_DIRECTORY path does not exist, or it is for the wrong platform.")

  endif()

  foreach(_Package
    ARToolKit
    Assimp
    Bullet
    Cg
    Eigen3
    FCollada
    FFMPEG
    FMODEx
    Freetype
    HarfBuzz
    JPEG
    LibSquish
    ODE
    Ogg
    OpenAL
    OpenEXR
    OpenSSL
    OpusFile
    PNG
    SWResample
    SWScale
    TIFF
    VorbisFile
    VRPN
    ZLIB
  )

    string(TOLOWER "${_Package}" _package)
    string(TOUPPER "${_Package}" _PACKAGE)

    # Some packages in the thirdparty dir have different subdirectory names from
    # the name of the CMake package
    if(_package STREQUAL "cg")
      set(_package "nvidiacg")
    elseif(_package STREQUAL "eigen3")
      set(_package "eigen")
    elseif(_package STREQUAL "ogg")
      set(_package "vorbis") # It's in the same install dir here
    elseif(_package STREQUAL "opusfile")
      set(_package "opus")
    elseif(_package STREQUAL "libsquish")
      set(_package "squish")
    elseif(_package STREQUAL "swresample" OR _package STREQUAL "swscale")
      set(_package "ffmpeg") # These are also part of FFmpeg
    elseif(_package STREQUAL "vorbisfile")
      set(_package "vorbis")
    endif()

    # Set search path
    set(${_Package}_ROOT "${_package_dir}/${_package}")

    # Set up copying DLLs, if necessary
    file(GLOB _dlls "${${_Package}_ROOT}/bin/*.dll")
    if(_dlls)
      set(_havevar "HAVE_${_PACKAGE}")
      set(THIRDPARTY_DLLS_${_havevar} "${_dlls}")
      list(APPEND THIRDPARTY_DLLS "${_havevar}")

    endif()

  endforeach(_Package)

endif()

# This is used to copy the DLLs alongside the output of `package`
function(thirdparty_copy_alongside package)
  set(_dlls)

  foreach(_havevar ${THIRDPARTY_DLLS})
    if(${_havevar})
      list(APPEND _dlls ${THIRDPARTY_DLLS_${_havevar}})
    endif()
  endforeach(_havevar)

  if(NOT _dlls)
    # Don't try to copy/install nothingness
    return()
  endif()

  add_custom_command(TARGET ${package} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${_dlls} $<TARGET_FILE_DIR:${package}>
  )

  # Also install the DLLs
  install(FILES ${_dlls} DESTINATION ${CMAKE_INSTALL_BINDIR})

endfunction(thirdparty_copy_alongside)

#
# ------------ Python ------------
#

set(WANT_PYTHON_VERSION ""
  CACHE STRING "Which Python version to seek out for building Panda3D against.")

if(DEFINED _PREV_WANT_PYTHON_VERSION
    AND NOT _PREV_WANT_PYTHON_VERSION STREQUAL WANT_PYTHON_VERSION)
  # The user changed WANT_PYTHON_VERSION. We need to force FindPython to start
  # anew, deleting any variable that was autodetected last time
  foreach(_prev_var ${_PREV_PYTHON_VALUES})
    string(REPLACE "=" ";" _prev_var "${_prev_var}")
    list(GET _prev_var 0 _prev_var_name)
    list(GET _prev_var 1 _prev_var_sha1)
    string(SHA1 _current_var_sha1 "${${_prev_var_name}}")

    if(_prev_var_sha1 STREQUAL _current_var_sha1)
      unset(${_prev_var_name} CACHE)
    endif()

  endforeach(_prev_var)

  unset(_PREV_PYTHON_VALUES CACHE)

endif()

if(WANT_PYTHON_VERSION)
  # A specific version is requested; ensure we get that specific version
  list(APPEND WANT_PYTHON_VERSION "EXACT")
endif()

get_directory_property(_old_cache_vars CACHE_VARIABLES)
find_package(Python ${WANT_PYTHON_VERSION} QUIET COMPONENTS Interpreter Development)

if(Python_FOUND)
  set(PYTHON_FOUND ON)
  set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
  set(PYTHON_INCLUDE_DIRS ${Python_INCLUDE_DIRS})
  set(PYTHON_LIBRARY_DIRS ${Python_LIBRARY_DIRS})
  set(PYTHON_VERSION_STRING ${Python_VERSION})

elseif(CMAKE_VERSION VERSION_LESS "3.12")
  find_package(PythonInterp ${WANT_PYTHON_VERSION} QUIET)
  find_package(PythonLibs ${PYTHON_VERSION_STRING} QUIET)

  if(PYTHONLIBS_FOUND)
    set(PYTHON_FOUND ON)

    if(NOT PYTHON_VERSION_STRING)
      set(PYTHON_VERSION_STRING ${PYTHONLIBS_VERSION_STRING})
    endif()
  endif()

endif()

if(CMAKE_VERSION VERSION_LESS "3.15")
  # CMake versions this old don't provide Python::Module, so we need to hack up
  # the variables to ensure no explicit linkage against libpython occurs

  if(WIN32)
    # Nothing needed here; explicit linkage is appropriate
    set(PYTHON_LIBRARY "${Python_LIBRARY}")
    set(PYTHON_LIBRARIES ${Python_LIBRARIES})

  elseif(APPLE OR UNIX)
    # Just unset and let the implicit linkage take over
    set(PYTHON_LIBRARY "")
    set(PYTHON_LIBRARIES "")

    if(APPLE)
      # macOS requires this explicit flag on the linker command line to allow the
      # references to the Python symbols to resolve at dynamic link time
      string(APPEND CMAKE_MODULE_LINKER_FLAGS " -undefined dynamic_lookup")

    endif()

  else()
    # On every other platform, guessing is a bad idea - insist the user upgrade
    # their CMake instead.
    message(WARNING "For Python support on this platform, please use CMake >= 3.15!")
    set(PYTHON_FOUND OFF)

  endif()

endif()

package_option(Python
  DEFAULT ON
  "Enables support for Python.  If INTERROGATE_PYTHON_INTERFACE
is also enabled, Python bindings will be generated."
  IMPORTED_AS Python::Module)

# Also detect the optimal install paths:
if(HAVE_PYTHON)
  if(WIN32 AND NOT CYGWIN)
    set(_LIB_DIR ".")
    set(_ARCH_DIR ".")

  elseif(PYTHON_EXECUTABLE)
    execute_process(
      COMMAND ${PYTHON_EXECUTABLE}
        -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(False))"
      OUTPUT_VARIABLE _LIB_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(
      COMMAND ${PYTHON_EXECUTABLE}
        -c "from distutils.sysconfig import get_python_lib; print(get_python_lib(True))"
      OUTPUT_VARIABLE _ARCH_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE)

  else()
    set(_LIB_DIR "")
    set(_ARCH_DIR "")

  endif()

  execute_process(
    COMMAND ${PYTHON_EXECUTABLE}
      -c "from sysconfig import get_config_var as g; print((g('EXT_SUFFIX') or g('SO'))[:])"
    OUTPUT_VARIABLE _EXT_SUFFIX
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)

  if(NOT _EXT_SUFFIX)
    if(CYGWIN)
      set(_EXT_SUFFIX ".dll")
    elseif(WIN32)
      set(_EXT_SUFFIX ".pyd")
    else()
      set(_EXT_SUFFIX ".so")
    endif()
  endif()

  set(PYTHON_LIB_INSTALL_DIR "${_LIB_DIR}" CACHE STRING
    "Path to the Python architecture-independent package directory.")

  set(PYTHON_ARCH_INSTALL_DIR "${_ARCH_DIR}" CACHE STRING
    "Path to the Python architecture-dependent package directory.")

  set(PYTHON_EXTENSION_SUFFIX "${_EXT_SUFFIX}" CACHE STRING
    "Suffix for Python binary extension modules.")

endif()

if(NOT DEFINED _PREV_PYTHON_VALUES)
  # We need to make note of all auto-defined Python variables
  set(_prev_python_values)

  get_directory_property(_new_cache_vars CACHE_VARIABLES)
  foreach(_cache_var ${_new_cache_vars})
    if(_cache_var MATCHES "^(Python|PYTHON)_" AND NOT _old_cache_vars MATCHES ";${_cache_var};")
      string(SHA1 _cache_var_sha1 "${${_cache_var}}")
      list(APPEND _prev_python_values "${_cache_var}=${_cache_var_sha1}")
    endif()
  endforeach(_cache_var)

  set(_PREV_PYTHON_VALUES "${_prev_python_values}" CACHE INTERNAL "Internal." FORCE)
endif()

set(_PREV_WANT_PYTHON_VERSION "${WANT_PYTHON_VERSION}" CACHE INTERNAL "Internal." FORCE)


#
# ------------ Data handling libraries ------------
#

# OpenSSL
find_package(OpenSSL COMPONENTS SSL Crypto QUIET)

package_option(OpenSSL
  DEFAULT ON
  "Enable OpenSSL support"
  IMPORTED_AS OpenSSL::SSL OpenSSL::Crypto)

option(REPORT_OPENSSL_ERRORS
  "Define this true to include the OpenSSL code to report verbose
error messages when they occur." OFF)
option(REPORT_OPENSSL_ERRORS_Debug "" ON)

package_status(OpenSSL "OpenSSL")

# zlib
find_package(ZLIB QUIET)

package_option(ZLIB
  "Enables support for compression of Panda assets."
  IMPORTED_AS ZLIB::ZLIB)

package_status(ZLIB "zlib")


#
# ------------ Image formats ------------
#

# JPEG
find_package(JPEG QUIET)

package_option(JPEG "Enable support for loading .jpg images.")

package_status(JPEG "libjpeg")

# PNG
find_package(PNG QUIET)

package_option(PNG
  "Enable support for loading .png images."
  IMPORTED_AS PNG::PNG)

package_status(PNG "libpng")

# TIFF
find_package(TIFF QUIET)

package_option(TIFF "Enable support for loading .tif images.")

package_status(TIFF "libtiff")

# OpenEXR
find_package(OpenEXR QUIET)

package_option(OpenEXR "Enable support for loading .exr images.")

package_status(OpenEXR "OpenEXR")

# libsquish
find_package(LibSquish QUIET)

package_option(SQUISH
  "Enables support for automatic compression of DXT textures."
  FOUND_AS LibSquish)

package_status(SQUISH "libsquish")


#
# ------------ Asset formats ------------
#

# Assimp
find_package(Assimp QUIET)

package_option(Assimp
  "Build pandatool with support for loading 3D assets supported by Assimp.")

package_status(Assimp "Assimp")

# FCollada
find_package(FCollada QUIET)

package_option(FCollada
  "Build pandatool with support for loading Collada files using FCollada."
  IMPORTED_AS FCollada::FCollada)

package_status(FCollada "FCollada")


#
# ------------ Math libraries ------------
#

# Eigen
find_package(Eigen3 QUIET)

package_option(EIGEN
  "Enables use of the Eigen linear algebra library.
If this is provided, Panda will use this library as the fundamental
implementation of its own linmath library; otherwise, it will use
its own internal implementation.  The primary advantage of using
Eigen is SSE2 support, which is only activated if LINMATH_ALIGN
is also enabled."
  FOUND_AS Eigen3
  LICENSE "MPL-2")

option(LINMATH_ALIGN
  "This is required for activating SSE2 support using Eigen.
Activating this does constrain most objects in Panda to 16-byte
alignment, which could impact memory usage on very-low-memory
platforms.  Currently experimental." ON)

if(LINMATH_ALIGN)
  package_status(EIGEN "Eigen linear algebra library" "vectorization enabled in build")
else()
  package_status(EIGEN "Eigen linear algebra library" "vectorization NOT enabled in build")
endif()

# FFTW
# FFTW 3.3.7, when built with autotools, doesn't install
# FFTW3LibraryDepends.cmake, which will crash us if we use CONFIG mode.  BAH!
# Force MODULE mode to fix that.
find_package(FFTW3 MODULE QUIET)

package_option(FFTW
  "This enables support for compression of animations in .bam files.
  This is only necessary for creating or reading .bam files containing
  compressed animations."
  FOUND_AS "FFTW3"
  LICENSE "GPL")

package_status(FFTW "FFTW")


#
# ------------ Multimedia formats ------------
#

# FFmpeg
find_package(FFMPEG QUIET)
find_package(SWScale QUIET)
find_package(SWResample QUIET)

package_option(FFMPEG
  "Enables support for audio- and video-decoding using the FFmpeg library.")
package_option(SWScale
  "Enables support for FFmpeg's libswscale for video rescaling.")
package_option(SWResample
  "Enables support for FFmpeg's libresample for audio resampling.")

if(HAVE_SWSCALE AND HAVE_SWRESAMPLE)
  set(ffmpeg_features "with swscale and swresample")
elseif(HAVE_SWSCALE)
  set(ffmpeg_features "with swscale")
elseif(HAVE_SWRESAMPLE)
  set(ffmpeg_features "with swresample")
else()
  set(ffmpeg_features "without resampling/rescaling support")
endif()
package_status(FFMPEG "FFmpeg" "${ffmpeg_features}")

# Vorbis
find_package(VorbisFile QUIET)

package_option(VORBIS
  FOUND_AS VorbisFile
  "Enables support for decoding Vorbis-encoded .ogg audio files via libvorbisfile.")

package_status(VORBIS "Vorbis")

# Opus
find_package(OpusFile QUIET)

package_option(OPUS
  FOUND_AS OpusFile
  "Enables support for decoding .opus audio files via libopusfile.")

package_status(OPUS "Opus")

#
# ------------ Audio libraries ------------
#

# FMOD Ex
find_package(FMODEx QUIET)

package_option(FMODEx
  "This enables support for the FMOD Ex sound library,
  from Firelight Technologies. This audio library is free for non-commercial
  use."
  LICENSE "FMOD")

package_status(FMODEx "FMOD Ex sound library")

# OpenAL
find_package(OpenAL QUIET)

package_option(OpenAL
  "This enables support for audio output via OpenAL. Some platforms, such as
  macOS, provide their own OpenAL implementation, which Panda3D can use. But,
  on most platforms this will imply OpenAL Soft, which is LGPL licensed."
  IMPORTED_AS OpenAL::OpenAL
  LICENSE "LGPL")

package_status(OpenAL "OpenAL sound library")

if(OpenAL_FOUND AND APPLE OR OPENAL_FOUND AND APPLE)
  set(HAVE_OPENAL_FRAMEWORK YES)
endif()


#
# ------------ UI libraries ------------
#

# Freetype

find_package(Freetype QUIET)

package_option(Freetype
  "This enables support for the FreeType font-rendering library.  If disabled,
  Panda3D will only be able to read fonts specially made with egg-mkfont."
  IMPORTED_AS freetype)

package_status(Freetype "FreeType")

# HarfBuzz

# Some versions of harfbuzz-config.cmake contain an endless while loop, so we
# force MODULE mode here.
find_package(HarfBuzz MODULE QUIET)

package_option(HarfBuzz
  "This enables support for the HarfBuzz text shaping library."
  IMPORTED_AS harfbuzz::harfbuzz)

package_status(HarfBuzz "HarfBuzz")

# GTK2

set(Freetype_FIND_QUIETLY TRUE) # Fix for builtin FindGTK2
set(GTK2_GTK_FIND_QUIETLY TRUE) # Fix for builtin FindGTK2
find_package(GTK2 QUIET COMPONENTS gtk)

package_option(GTK2)

package_status(GTK2 "gtk+-2")


#
# ------------ Physics engines ------------
#

# Bullet
find_package(Bullet MODULE QUIET)

package_option(Bullet
  "Enable this option to support game dynamics with the Bullet physics library.")

package_status(Bullet "Bullet physics")

# ODE
find_package(ODE QUIET)

package_option(ODE
  "Enable this option to support game dynamics with the Open Dynamics Engine (ODE)."
  LICENSE "BSD-3"
  IMPORTED_AS ODE::ODE)

package_status(ODE "Open Dynamics Engine")


#
# ------------ SpeedTree ------------
#

# SpeedTree
find_package(SpeedTree QUIET)

package_option(SpeedTree
  "Enable this option to include scenegraph support for SpeedTree trees."
  LICENSE "SpeedTree")

package_status(SpeedTree "SpeedTree")


#
# ------------ Rendering APIs ------------
#

# OpenGL
find_package(OpenGL QUIET)

package_option(GL
  "Enable OpenGL support."
  FOUND_AS OPENGL
  IMPORTED_AS OpenGL::GL)

package_status(GL "OpenGL")

# OpenGL ES 1
if(NOT APPLE) # Apple X11 ships the GLES headers but they're broken
  find_package(OpenGLES1 QUIET)
endif()

package_option(GLES1
  "Enable support for OpenGL ES 1.x rendering APIs."
  FOUND_AS OPENGLES1)

package_status(GLES1 "OpenGL ES 1.x")

# OpenGL ES 2
if(NOT APPLE) # Apple X11 ships the GLES headers but they're broken
  find_package(OpenGLES2 QUIET)
endif()

package_option(GLES2
  "Enable support for OpenGL ES 2.x rendering APIs."
  FOUND_AS OPENGLES2)

package_status(GLES2 "OpenGL ES 2.x")

# Direct3D 9
find_package(Direct3D9 QUIET COMPONENTS dxguid dxerr d3dx9)

package_option(DX9
  "Enable support for DirectX 9.  This is typically only viable on Windows."
  FOUND_AS Direct3D9)

package_status(DX9 "Direct3D 9.x")

# Nvidia Cg
find_package(Cg QUIET)

package_option(CG
  "Enable support for Nvidia Cg Shading Language"
  LICENSE "Nvidia")
package_option(CGGL
  "Enable support for Nvidia Cg's OpenGL API."
  LICENSE "Nvidia")
package_option(CGD3D9
  "Enable support for Nvidia Cg's Direct3D 9 API."
  LICENSE "Nvidia")

if(HAVE_CGGL AND HAVE_CGD3D9)
  set(cg_apis "supporting OpenGL and Direct3D 9")
elseif(HAVE_CGGL)
  set(cg_apis "supporting OpenGL")
elseif(HAVE_CGDX9)
  set(cg_apis "supporting Direct3D 9")
else()
  set(cg_apis "WITHOUT rendering backend support")
endif()
package_status(CG "Nvidia Cg Shading Language" "${cg_apis}")


#
# ------------ Display APIs ------------
#

# X11 (and GLX)

if(NOT APPLE)
  find_package(X11 QUIET)
endif()

if(NOT X11_Xkb_FOUND OR NOT X11_Xutil_FOUND)
  # Panda implicitly requires these supplementary X11 libs; if we can't find
  # them, we just say we didn't find X11 at all.
  set(X11_FOUND OFF)
endif()

package_option(X11
  "Provides X-server support on Unix platforms. X11 may need to be linked
against for tinydisplay, but probably only on a Linux platform.")

set(HAVE_GLX_AVAILABLE OFF)
if(HAVE_GL AND HAVE_X11 AND NOT APPLE)
  set(HAVE_GLX_AVAILABLE ON)
endif()

option(HAVE_GLX "Enables GLX. Requires OpenGL and X11." ${HAVE_GLX_AVAILABLE})
if(HAVE_GLX AND NOT HAVE_GLX_AVAILABLE)
  message(SEND_ERROR "HAVE_GLX manually set to ON but it is not available!")
endif()

if(HAVE_GLX)
  package_status(X11 "X11" "with GLX")
else()
  package_status(X11 "X11" "without GLX")
endif()

# EGL
find_package(EGL QUIET)

package_option(EGL
  "Enable support for the Khronos EGL context management interface for
  OpenGL ES.  This is necessary to support OpenGL ES under X11.")

package_status(EGL "EGL")

#
# ------------ Vision tools ------------
#

# OpenCV
find_package(OpenCV QUIET COMPONENTS core highgui OPTIONAL_COMPONENTS videoio)

package_option(OpenCV
  "Enable support for OpenCV.  This will be built into the 'vision' package."
  FOUND_AS OpenCV)

package_status(OpenCV "OpenCV")

# CMake <3.7 doesn't support GREATER_EQUAL, so this uses NOT LESS instead.
if(NOT OpenCV_VERSION_MAJOR LESS 3)
  set(OPENCV_VER_3 ON)

elseif(NOT OpenCV_VERSION_MAJOR LESS 2 AND
       NOT OpenCV_VERSION_MINOR LESS 3)
  set(OPENCV_VER_23 ON)

endif()

# ARToolKit
find_package(ARToolKit QUIET)

package_option(ARToolKit
  "Enable support for ARToolKit.  This will be built into the 'vision' package.")

package_status(ARToolKit "ARToolKit")


#
# ------------ VR integration ------------
#

# VRPN
find_package(VRPN QUIET)

package_option(VRPN
  "Enables support for connecting to VRPN servers.  This is only needed if you
  are building Panda3D for a fixed VRPN-based VR installation.")

package_status(VRPN "VRPN")
