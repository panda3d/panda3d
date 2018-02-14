#
# dtool/Config.cmake
#
# This file defines certain configuration variables that are written
# into the various make scripts.  It is processed by CMake to
# generate build scripts appropriate to each environment.
#

# Define the plaform we are building on.
# The values "UNIX", "WIN32", "MINGW", "MSYS", and "CYGWIN"
# are automatically provided by CMAKE.  "APPLE" is also provided by
# CMAKE but may be True on systems that are not OS X.
if(CMAKE_SYSTEM_NAME MATCHES "Linux")
  set(IS_LINUX 1)
endif()
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(IS_OSX 1)
endif()
if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  set(IS_FREEBSD 1)
endif()


# Define the type of build we are setting up.
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebInfo)
endif()
set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
  Release RelWithDebInfo Debug MinSizeRel Distribution)

# Provide convenient boolean expression based on build type
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  set(IS_DEBUG_BUILD True)
  set(IS_NOT_DEBUG_BUILD False)
else()
  set(IS_DEBUG_BUILD False)
  set(IS_NOT_DEBUG_BUILD True)
endif()

if(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
  set(IS_MINSIZE_BUILD True)
  set(IS_NOT_MINSIZE_BUILD False)
else()
  set(IS_MINSIZE_BUILD False)
  set(IS_NOT_MINSIZE_BUILD True)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Distribution")
  set(IS_DIST_BUILD True)
  set(IS_NOT_DIST_BUILD False)
else()
  set(IS_DIST_BUILD False)
  set(IS_NOT_DIST_BUILD True)
endif()


# Are we building with static or dynamic linking?
option(BUILD_SHARED_LIBS
  "Causes subpackes to be built separately -- setup for dynamic linking.
Utilities/tools/binaries/etc are then dynamically linked to the
libraries instead of being statically linked." ON)


# The character used to separate components of an OS-specific
# directory name depends on the platform (it is '/' on Unix, '\' on
# Windows).  That character selection is hardcoded into Panda and
# cannot be changed here.  (Note that an internal Panda filename
# always uses the forward slash, '/', to separate the components of a
# directory name.)

# There's a different character used to separate the complete
# directory names in a search path specification.  On Unix, the
# normal convention is ':', on Windows, it has to be ';', because the
# colon is already used to mark the drive letter.  This character is
# selectable here.  Most users won't want to change this.  If
# multiple characters are placed in this string, any one of them may
# be used as a separator character.
if(WIN32)
  set(DEFAULT_PATHSEP ";")
else()
  set(DEFAULT_PATHSEP ":")
endif()


# Panda uses prc files for runtime configuration.  There are many
# compiled-in options to customize the behavior of the prc config
# system; most users won't need to change any of them.  Feel free to
# skip over all of the PRC_* variables defined here.

# The default behavior is to search for files names *.prc in the
# directory specified by the PRC_DIR environment variable, and then
# to search along all of the directories named by the PRC_PATH
# environment variable.  Either of these variables might be
# undefined; if both of them are undefined, the default is to search
# in the directory named here by DEFAULT_PRC_DIR.

# By default, we specify the <auto>/etc dir, which is a special
# syntax that causes it to automatically search up the directory
# tree starting at the location of libpandaexpress.dll for any
# directories called 'etc'.

set(DEFAULT_PRC_DIR "<auto>etc" CACHE STRING
  "The compiled-in default directory to look for the Config.prc file,
in the absence of the PRC_DIR environment variable set, and in
the absence of anything specified via the configpath directive.")

# You can specify the names of the environment variables that are
# used to specify the search location(s) for prc files at runtime.
# These are space-separated lists of environment variable names.
# Specify empty string for either one of these to disable the
# feature.  For instance, redefining PRC_DIR_ENVVARS here to
# PRC_DIR would cause the environment variable $PRC_DIR
# to be consulted at startup instead of the default value of
# $PANDA_PRC_DIR.

set(PRC_DIR_ENVVARS "PANDA_PRC_DIR" CACHE STRING
  "The compiled-in name of the environment variable(s) that contain
the name of a single directory in which to search for prc files.")

set(PRC_PATH_ENVVARS "PANDA_PRC_PATH" CACHE STRING
  "The compiled-in name of the environment variable(s) that contain
the name of multiple directories, separated by DEFAULT_PATHSEP, in
which to search for prc files.")

# You can specify the name of the file(s) to search for in the above
# paths to be considered a config file.  This should be a
# space-separated list of filename patterns.  This is *.prc by
# default; normally there's no reason to change this.

set(PRC_PATTERNS "*.prc" CACHE STRING
  "The filename(s) to search for in the above paths.  Normally this is
*.prc.")

# You can optionally encrypt your prc file(s) to help protect them
# from curious eyes.  You have to specify the encryption key, which
# gets hard-coded into the executable.  (This feature provides mere
# obfuscation, not real security, since the encryption key can
# potentially be extracted by a hacker.)  This requires building with
# OpenSSL.

set(PRC_ENCRYPTED_PATTERNS "*.prc.pe" CACHE STRING
  "The filename(s) for encrypted prc files.")

set(PRC_ENCRYPTION_KEY "" CACHE STRING
  "The encryption key used to decrypt any encrypted prc files
identified by PRC_ENCRYPTED_PATTERNS.")

# One unusual feature of config is the ability to execute one or more
# of the files it discovers as if it were a program, and then treat
# the output of this program as a prc file.  If you want to use this
# feature, define this variable to the filename pattern or patterns
# for such executable-style config programs (e.g. *prc.exe).  This
# can be the same as the above if you like this sort of ambiguity; in
# that case, config will execute the file if it appears to be
# executable; otherwise, it will simply read it.

set(PRC_EXECUTABLE_PATTERNS "" CACHE STRING 
  "The filename(s) to search for, and execute, in the above paths.
Normally this is empty.")

# If you do use the above feature, you'll need another environment
# variable that specifies additional arguments to pass to the
# executable programs.  The default definition, given here, makes
# that variable be $PANDA_PRC_XARGS.  Sorry, the same arguments
# must be supplied to all executables in a given runtime session.

set(PRC_EXECUTABLE_ARGS_ENVVAR "PANDA_PRC_XARGS" CACHE STRING
  "The environment variable that defines optional args to pass to
executables found that match one of the above patterns.")

# You can implement signed prc files, if you require this advanced
# feature.  This allows certain config variables to be set only by a
# prc file that has been provided by a trusted source.  To do this,
# first install and compile Dtool with OpenSSL and run the program
# make-prc-key, and then specify here the output filename generated
# by that program, and then recompile Dtool.

set(PRC_PUBLIC_KEYS_FILENAME "" CACHE STRING "")

# By default, the signed-prc feature, above, is enabled only for a
# release build.  In a normal development environment, any prc file
# can set any config variable, whether  or not it is signed.  Set
# this variable true or false to explicitly enable or disable this
# feature.
#XXX For which build types should this be enabled?
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  set(DEFAULT_PRC_RESPECT_TRUST_LEVEL ON)
else()
  set(DEFAULT_PRC_RESPECT_TRUST_LEVEL OFF)
endif()

option(PRC_RESPECT_TRUST_LEVEL
  "Define if we want to enable the trust_level feature of prc config
variables.  This requires OpenSSL and PRC_PUBLIC_KEYS_FILENAME,
above." ${DEFAULT_PRC_RESPECT_TRUST_LEVEL})

# If trust level is in effect, this specifies the default trust level
# for any legacy (Dconfig) config variables (that is, variables
# created using the config.GetBool(), etc. interface, rather than the
# newer ConfigVariableBool interface).

set(PRC_DCONFIG_TRUST_LEVEL "0" CACHE STRING
  "The trust level value for any legacy (DConfig) variables.")

# If trust level is in effect, you may globally increment the
# (mis)trust level of all variables by the specified amount.
# Incrementing this value by 1 will cause all variables to require at
# least a level 1 signature.

set(PRC_INC_TRUST_LEVEL "0" CACHE STRING
  "The amount by which we globally increment the trust level.")

# Similarly, the descriptions are normally saved only in a
# development build, not in a release build.  Set this value true to
# explicitly save them anyway.
#XXX only for release-release builds
option(PRC_SAVE_DESCRIPTIONS
  "Define if you want to save the descriptions for ConfigVariables."
  ON)

mark_as_advanced(DEFAULT_PRC_DIR PRC_DIR_ENVVARS PRC_PATH_ENVVARS
  PRC_PATTERNS PRC_ENCRYPTED_PATTERNS PRC_ENCRYPTION_KEY
  PRC_EXECUTABLE_PATTERNS PRC_EXECUTABLE_ARGS_ENVVAR
  PRC_PUBLIC_KEYS_FILENAME PRC_RESPECT_TRUST_LEVEL
  PRC_DCONFIG_TRUST_LEVEL PRC_INC_TRUST_LEVEL PRC_SAVE_DESCRIPTIONS)

#
# This is the end of the PRC variable customization section.  The
# remaining variables are of general interest to everyone.
#


option(HAVE_P3D_PLUGIN
  "You may define this to build or develop the plugin." OFF)

option(HAVE_P3D_RTDIST
  "You may define this to build or develop the Panda3D rtdist,
the environment packaged up for distribution with the plugin."
  OFF)

if(HAVE_P3D_RTDIST)
  set(PANDA_PACKAGE_VERSION "local_dev" CACHE STRING "")
  set(PANDA_PACKAGE_HOST_URL "http://localhost/" CACHE STRING "")
endif()

mark_as_advanced(HAVE_P3D_RTDIST PANDA_PACKAGE_VERSION PANDA_PACKAGE_HOST)



# The following options relate to interrogate, the tool that is
# used to generate bindings for non-C++ languages.

option(INTERROGATE_PYTHON_INTERFACE
  "Do you want to generate a Python-callable interrogate interface?
This is only necessary if you plan to make calls into Panda from a
program written in Python.  This is done only if HAVE_PYTHON is also
true." ON)

set(INTERROGATE_C_INTERFACE
  "Do you want to generate a C-callable interrogate interface?  This
generates an interface similar to the Python interface above, with
a C calling convention.  It should be useful for most other kinds
of scripting language; the VR Studio used to use this to make calls
into Panda from Squeak." OFF)

option(HAVE_INTERROGATE
  "Do you even want to build interrogate at all?  This is the program
that reads our C++ source files and generates one of the above
interfaces.  If you won't be building the interfaces, you don't
need the program." ON)

set(INTERROGATE_OPTIONS "-fnames;-string;-refcount;-assert" CACHE STRING
  "What additional options should be passed to interrogate when
generating either of the above two interfaces?  Generally, you
probably don't want to mess with this.")

option(INTERROGATE_VERBOSE
  "Set this if you would like interrogate to generate advanced
debugging information." OFF)

set(INTERROGATE "interrogate" CACHE STRING
  "What's the name of the interrogate binary to run?  The default
specified is the one that is built as part of DTOOL.  If you have a
prebuilt binary standing by (for instance, if you are cross-compiling
and cannot run the built version), specify its name instead.")

set(INTERROGATE_MODULE "interrogate_module" CACHE STRING
  "Same as INTERROGATE, except for the interrogate_module binary.")

mark_as_advanced(INTERROGATE_OPTIONS)

if(NOT CMAKE_CROSSCOMPILING)
  mark_as_advanced(INTERROGATE INTERROGATE_MODULE)
endif()


#
# The following options have to do with the memory allocation system
# that will be used by Panda3D.
#

option(DO_MEMORY_USAGE
  "Do you want to compile in support for tracking memory usage?  This
enables you to define the variable 'track-memory-usage' at runtime
to help track memory leaks, and also report total memory usage on
PStats.  There is some small overhead for having this ability
available, even if it is unused." ${IS_DEBUG_BUILD})

option(SIMULATE_NETWORK_DELAY
  "This option compiles in support for simulating network delay via
the min-lag and max-lag prc variables.  It adds a tiny bit of
overhead even when it is not activated, so it is typically enabled
only in a development build." ${IS_DEBUG_BUILD})

option(SUPPORT_IMMEDIATE_MODE
  "This option compiles in support for immediate-mode OpenGL
rendering.  Since this is normally useful only for researching
buggy drivers, and since there is a tiny bit of per-primitive
overhead to have this option available even if it is unused, it is
by default enabled only in a development build.  This has no effect
on DirectX rendering." ${IS_DEBUG_BUILD})

option(NOTIFY_DEBUG
  "Do you want to include the 'debug' and 'spam' Notify messages?
Normally, these are stripped out when we build for release, but sometimes it's
useful to keep them around. Turn this setting on to achieve that." ${IS_DEBUG_BUILD})

option(SUPPORT_FIXED_FUNCTION
  "This option compiles in support for the fixed-function OpenGL
pipeline.  It is only really useful to turn this off if you are targeting
an OpenGL ES 2 system." ON)

option(USE_MEMORY_DLMALLOC
  "This is an optional alternative memory-allocation scheme
available within Panda.  You can experiment with it to see
if it gives better performance than the system malloc(), but
at the time of this writing, it doesn't appear that it does." OFF)

option(USE_MEMORY_PTMALLOC2
  "This is an optional alternative memory-allocation scheme
available within Panda.  You can experiment with it to see
if it gives better performance than the system malloc(), but
at the time of this writing, it doesn't appear that it does." OFF)

option(MEMORY_HOOK_DO_ALIGN
  "Set this true if you prefer to use the system malloc library even
if 16-byte alignment must be performed on top of it, wasting up to
30% of memory usage.  If you do not set this, and 16-byte alignment
is required and not provided by the system malloc library, then an
alternative malloc system (above) will be used instead." OFF)

option(ALTERNATIVE_MALLOC
  "Do you want to use one of the alternative malloc implementations?"
  OFF)

option(USE_DELETED_CHAIN
  "Define this true to use the DELETED_CHAIN macros, which support
fast re-use of existing allocated blocks, minimizing the low-level
calls to malloc() and free() for frequently-created and -deleted
objects.  There's usually no reason to set this false, unless you
suspect a bug in Panda's memory management code." ON)

mark_as_advanced(DO_MEMORY_USAGE SIMULATE_NETWORK_DELAY
  SUPPORT_IMMEDIATE_MODE USE_MEMORY_DLMALLOC USE_MEMORY_PTMALLOC2
  MEMORY_HOOK_DO_ALIGN ALTERNATIVE_MALLOC USE_DELETED_CHAIN)


#
# This section relates to mobile-device/phone support and options
#

# iPhone support
set(BUILD_IPHONE "" CACHE STRING
  "Panda contains some experimental code to compile for IPhone.  This
requires the Apple IPhone SDK, which is currently only available
for OS X platforms.  Set this to either 'iPhoneSimulator' or
'iPhoneOS'.  Note that this is still *experimental* and incomplete!
Don't enable this unless you know what you're doing!")
set_property(CACHE BUILD_IPHONE PROPERTY STRINGS "" iPhoneSimulator iPhoneOS)


# Android support
option(BUILD_ANDROID
  "Panda contains some experimental code to compile for Android.
This requires the Google Android NDK. Besides BUILD_ANDROID, you'll
also have to set ANDROID_NDK_HOME." OFF)

set(ANDROID_NDK_HOME "" CACHE STRING
  "The location of the Android NDK directory. ANDROID_NDK_HOME may
not contain any spaces.")

set(ANDROID_ABI "armeabi" CACHE STRING
  "Can be be set to armeabi, armeabi-v7a, x86, or mips,
depending on which architecture should be targeted.")
set_property(CACHE ANDROID_ABI PROPERTY STRINGS
  armeabi armeabi-v7a x86 mips)

set(ANDROID_STL "gnustl_shared" CACHE STRING "")
set(ANDROID_PLATFORM "android-9" CACHE STRING "")
set(ANDROID_ARCH "arm" CACHE STRING "")
if(ANDROID_ARCH STREQUAL "arm")
  set(ANDROID_TOOLCHAIN "arm-linux-androideabi")
else()
  set(ANDROID_TOOLCHAIN "")
endif()

mark_as_advanced(ANDROID_NDK_HOME ANDROID_ABI ANDROID_STL
  ANDROID_PLATFORM ANDROID_ARCH)


#
# Now let's check for the presence of various thirdparty libraries.
#

# Is Eigen installed, and should Eigen replace internal linmath?
find_package(Eigen3 QUIET)

package_option(EIGEN
  "Enables experimental support for the Eigen linear algebra library.
If this is provided, Panda will use this library as the fundamental
implementation of its own linmath library; otherwise, it will use
its own internal implementation.  The primary advantage of using
Eigen is SSE2 support, which is only activated if LINMATH_ALIGN
is also enabled."
  LICENSE "MPL-2")

option(LINMATH_ALIGN
  "This is required for activating SSE2 support using Eigen.
Activating this does constrain most objects in Panda to 16-byte
alignment, which could impact memory usage on very-low-memory
platforms.  Currently experimental." ON)

# Always include Eigen, because we include it pretty much everywhere.
if(EIGEN3_FOUND)
  include_directories(${EIGEN3_INCLUDE_DIR})
endif()


# Is Python installed, and should Python interfaces be generated?
set(WANT_PYTHON_VERSION "2.7"
  CACHE STRING "Which Python version to seek out for building Panda3D against.")

find_package(PythonInterp ${WANT_PYTHON_VERSION} QUIET)
find_package(PythonLibs ${PYTHON_VERSION_STRING} QUIET)
if(PYTHONINTERP_FOUND AND PYTHONLIBS_FOUND)
  set(PYTHON_FOUND ON)
else()
  set(PYTHON_FOUND OFF)
endif()

package_option(PYTHON DEFAULT ON
  "Enables support for Python.  If INTERROGATE_PYTHON_INTERFACE
is also enabled, Python bindings will be generated.")

# Also detect the optimal install paths:
if(HAVE_PYTHON)
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

  set(PYTHON_LIB_INSTALL_DIR "${_LIB_DIR}" CACHE STRING
    "Path to the Python architecture-independent package directory.")

  set(PYTHON_ARCH_INSTALL_DIR "${_ARCH_DIR}" CACHE STRING
    "Path to the Python architecture-dependent package directory.")

  # Always include Python, because we include it pretty much everywhere
  # though we don't usually want to link it in as well.
  include_directories(${PYTHON_INCLUDE_DIRS})
endif()


# By default, we'll assume the user only wants to run with Debug
# python if he has to--that is, on Windows when building a debug build.
if(WIN32 AND IS_DEBUG_BUILD)
  set(USE_DEBUG_PYTHON ON)
else()
  set(USE_DEBUG_PYTHON OFF)
endif()

set(GENPYCODE_LIBS
  "libpandaexpress;libpanda;libpandaphysics;libp3direct;libpandafx;libp3vision;libpandaode;libp3vrpn"
  CACHE STRING
  "Define the default set of libraries to be instrumented by
genPyCode.  You may wish to add to this list to add your own
libraries, or if you want to use some of the more obscure
interfaces like libpandaegg and libpandafx.")

mark_as_advanced(GENPYCODE_LIBS)

option(INSTALL_PYTHON_SOURCE
  "Normally, Python source files are copied into the CMake library
install directory, along with the compiled C++ library objects, when
you make install.  If you prefer not to copy these Python source
files, but would rather run them directly out of the source
directory (presumably so you can develop them and make changes
without having to reinstall), comment out this definition and put
your source directory on your PYTHONPATH.")


# Is OpenSSL installed, and where?
find_package(OpenSSL COMPONENTS ssl crypto QUIET)

package_option(OPENSSL DEFAULT ON
  "Enable OpenSSL support")

option(REPORT_OPENSSL_ERRORS
  "Define this true to include the OpenSSL code to report verbose
error messages when they occur." ${IS_DEBUG_BUILD})


# Is libjpeg installed, and where?
find_package(JPEG QUIET)

package_option(JPEG DEFAULT ON
  "Enable support for loading .jpg images.")

# Some versions of libjpeg did not provide jpegint.h.  Redefine this
# to false if you lack this header file.
#set(PHAVE_JPEGINT_H TRUE)

option(HAVE_VIDEO4LINUX
  "Set this to enable webcam support on Linux." ${IS_LINUX})


# Is libpng installed, and where?
find_package(PNG QUIET)

package_option(PNG DEFAULT ON
  "Enable support for loading .png images.")


# Is libtiff installed, and where?
find_package(TIFF QUIET)

package_option(TIFF
  "Enable support for loading .tif images.")


# Is libtar installed, and where?
find_package(Tar QUIET)

package_option(TAR
  "This is used to optimize patch generation against tar files.")


# TODO: FFTW2
# Is libfftw installed, and where?
#find_package(FFTW2 QUIET)

#package_option(FFTW
#  "This enables support for lossless compression of animations in
#.bam files.  This is rarely used, and you probably don't need it.")

#TODO PHAVE_DRFFTW_H


# Is libsquish installed, and where?
find_package(Squish QUIET)

package_option(SQUISH
  "Enables support for automatic compression of DXT textures.")


# Is Cg installed, and where?
find_package(Cg QUIET)
package_option(CG
  "Enable support for Nvidia Cg Shading Language"
  LICENSE "Nvidia")
package_option(CGGL
  "Enable support for Nvidia Cg's OpenGL API."
  LICENSE "Nvidia")
package_option(CGDX8 "Enable support for Nvidia Cg's DX8 API."
  LICENSE "Nvidia")
package_option(CGDX9 "Enable support for Nvidia Cg's DX9 API."
  LICENSE "Nvidia")
package_option(CGDX10 "Enable support for Nvidia Cg's DX10 API."
  LICENSE "Nvidia")


# Is VRPN installed, and where?
find_package(VRPN QUIET)

package_option(VRPN
  "Enables support for connecting to VRPN servers.")


# TODO: Helix
# Is HELIX installed, and where?
#find_package(Helix)

#package_option(HELIX
# "Enables support for Helix media playback.")


# Is ZLIB installed, and where?
find_package(ZLIB QUIET)

package_option(ZLIB DEFAULT ON
  "Enables support for compression of Panda assets.")

# Is FFMPEG installed, and where?
find_package(FFMPEG QUIET)
find_package(SWScale QUIET)
find_package(SWResample QUIET)

package_option(FFMPEG
  "Enables support for audio- and video-decoding using the FFMPEG library.")
package_option(SWSCALE
  "Enables support for FFMPEG's libswscale for video rescaling.")
package_option(SWRESAMPLE
  "Enables support for FFMPEG's libresample for audio resampling.")

# Is ODE installed, and where?
find_package(ODE QUIET)

package_option(ODE
  "Enables support for ridid-body physics using the Open Dynamics Engine.")

# Is OpenGL installed, and where?
find_package(OpenGL QUIET)
set(GL_FOUND ${OPENGL_FOUND})
package_option(GL "Enable OpenGL support.")

# If you are having trouble linking in OpenGL extension functions at
# runtime for some reason, you can set this variable. It also,
# requires you to install the OpenGL header files and compile-time
# libraries appropriate to the version you want to compile against.
set(MIN_GL_VERSION "1 1" CACHE STRING
  "The variable is the major, minor version of OpenGL, separated by a
space (instead of a dot).  Thus, \"1 1\" means OpenGL version 1.1.

This defines the minimum runtime version of OpenGL that Panda will
require. Setting it to a higher version will compile in hard
references to the extension functions provided by that OpenGL
version and below, which may reduce runtime portability to other
systems, but it will avoid issues with getting extension function
pointers.")


# Should build tinydisplay?
option(HAVE_TINYDISPLAY
  "Builds TinyDisplay, a light software renderer based on TinyGL,
that is built into Panda. TinyDisplay is not as full-featured as Mesa
but is many times faster." ${IS_NOT_MINSIZE_BUILD})


# TODO: OpenGL ES
# Is OpenGL ES 1.x installed, and where?
#find_package(OpenGLES)

#package_option(GLES
#  "Enable OpenGL ES 1.x support, a minimal subset of
#OpenGL for mobile devices.")

# Is OpenGL ES 2.x installed, and where?
#find_package(OpenGLES)

#package_option(GLES2
#  "Enable OpenGL ES 2.x support, a version of OpenGL ES but without
#fixed-function pipeline - everything is programmable there.")

# Is EGL installed, and where?
#package_option(EGL
#  "Enable EGL support. EGL is like GLX, but for OpenGL ES.")


# Is SDL installed, and where?
set(Threads_FIND_QUIETLY TRUE) # Fix for builtin FindSDL
set(Eigen3_FIND_QUIETLY TRUE) # Fix for builtin FindSDL
set(PythonLibs_FIND_QUIETLY TRUE) # Fix for builtin FindSDL
set(PythonInterp_FIND_QUIETLY TRUE) # Fix for builtin FindSDL

find_package(SDL QUIET)

package_option(SDL
  "The SDL library is useful only for tinydisplay, and is not even
required for that, as tinydisplay is also supported natively on
each supported platform.")

# Cleanup after builtin FindSDL
mark_as_advanced(SDLMAIN_LIBRARY)
mark_as_advanced(SDL_INCLUDE_DIR)
mark_as_advanced(SDL_LIBRARY)
mark_as_advanced(SDL_LIBRARY_TEMP)


# Is X11 insalled, and where?
find_package(X11 QUIET)

package_option(X11
  "Provides X-server support on Unix platforms. X11 may need to be linked
against for tinydisplay, but probably only on a Linux platform.")
if(NOT UNIX AND HAVE_X11)
  message(SEND_ERROR
    "X11 support is only supported on Unix platforms:
ie. Linux, BSD, OS X, Cygwin, etc...")
endif()


# TODO: XF86DGA
# This defines if we have XF86DGA installed.
#find_package(XF86DGA QUIET)

#package_option(XF86DGA
#  "This enables smooth FPS-style mouse in x11display,
#when mouse mode M_relative is used.")


# TODO: XRANDR
#find_package(Xrandr QUIET)
#package_option(XRANDR
#  "This enables resolution switching in x11display.")


# TODO: XCURSOR
#find_package(Xcursor QUIET)
#package_option(XCURSOR
#  "This enables custom cursor support in x11display.")

if(HAVE_GL AND HAVE_X11)
  option(HAVE_GLX "Enables GLX. Requires OpenGL and X11." ON)
else()
  option(HAVE_GLX "Enables GLX. Requires OpenGL and X11." OFF)
endif()

option(LINK_IN_GLXGETPROCADDRESS
  "Define this to compile in a reference to the glXGetProcAddress().
This is only relevant from platforms using OpenGL under X."
  OFF)

if(WIN32 AND HAVE_GL)
  option(HAVE_WGL "Enable WGL.  Requires OpenGL on Windows." ON)
else()
  option(HAVE_WGL "Enable WGL.  Requires OpenGL on Windows." OFF)
endif()

if(IS_OSX)
  option(HAVE_COCOA "Enable Cocoa. Requires Mac OS X." ON)
  option(HAVE_CARBON "Enable Carbon. Requires Mac OS X." ON)
else()
  option(HAVE_COCOA "Enable Cocoa. Requires Mac OS X." OFF)
  option(HAVE_CARBON "Enable Carbon. Requires Mac OS X." OFF)
endif()

#
# <<<<<< Insert the rest of the Config.pp
#        port of third-party libs here <<<<<<<
#





#
# Miscellaneous settings
#

option(HAVE_WIN_TOUCHINPUT
"Define this if you are building on Windows 7 or better, and you
want your Panda build to run only on Windows 7 or better, and you
need to use the Windows touchinput interfaces." OFF)

option(WANT_NATIVE_NET
  "Define this true to build the low-level native network
implementation.  Normally this should be set true." ON)

option(HAVE_NET
  "Do you want to build the high-level network interface?  This layers
on top of the low-level native_net interface, specified above.
Normally, if you build NATIVE_NET, you will also build NET."
  ${WANT_NATIVE_NET})

option(HAVE_EGG
  "Do you want to build the egg loader?  Usually there's no reason to
avoid building this, unless you really want to make a low-footprint
build (such as, for instance, for the iPhone)." ON)

option(HAVE_AUDIO
  "Do you want to build the audio interface?" ON)

option(DO_PSTATS
  "Enable the pstats client?" ON)

option(USE_PANDAFILESTREAM
  "Enable the PandaFileStream implementation of pfstream etc.?" ON)

# These image formats don't require the assistance of a third-party
# library to read and write, so there's normally no reason to disable
# them int he build, unless you are looking to reduce the memory footprint.
option(HAVE_SGI_RGB "Enable support for loading SGI RGB images."
  ${IS_NOT_MINSIZE_BUILD})
option(HAVE_TGA "Enable support for loading TGA images."
  ${IS_NOT_MINSIZE_BUILD})
option(HAVE_IMG "Enable support for loading IMG images."
  ${IS_NOT_MINSIZE_BUILD})
option(HAVE_SOFTIMAGE_PIC
  "Enable support for loading SOFTIMAGE PIC images."
  ${IS_NOT_MINSIZE_BUILD})
option(HAVE_BMP "Enable support for loading BMP images."
  ${IS_NOT_MINSIZE_BUILD})
option(HAVE_PNM "Enable support for loading PNM images."
  ${IS_NOT_MINSIZE_BUILD})

mark_as_advanced(HAVE_SGI_RGB HAVE_TGA
  HAVE_IMG HAVE_SOFTIMAGE_PIC HAVE_BMP HAVE_PNM)


#
# <<<<< Insert the rest of the Config.pp
#       port of miscellaneous settings here <<<<<
#






# How to invoke bison and flex.  Panda takes advantage of some
# bison/flex features, and therefore specifically requires bison and
# flex, not some other versions of yacc and lex.  However, you only
# need to have these programs if you need to make changes to the
# bison or flex sources (see the next point, below).

find_package(BISON QUIET)
find_package(FLEX QUIET)

# You may not even have bison and flex installed.  If you don't, no
# sweat; Panda ships with the pre-generated output of these programs,
# so you don't need them unless you want to make changes to the
# grammars themselves (files named *.yxx or *.lxx).

set(HAVE_BISON ${BISON_FOUND})
set(HAVE_FLEX ${FLEX_FOUND})


#
# >>>>> Below is entirely temporary config information
#       until the port of Config.pp is finished.
#       It should be re-arranged for above. >>>>>>
#

### Configure threading support ###
find_package(Threads QUIET)

# Add basic use flag for threading
if(THREADS_FOUND)
  option(HAVE_THREADS
    "If on, compile Panda3D with threading support.
Building in support for threading will enable Panda to take
advantage of multiple CPU's if you have them (and if the OS
supports kernel threads running on different CPU's), but it will
slightly slow down Panda for the single CPU case." ON)
else()
  option(HAVE_THREADS
    "If on, compile Panda3D with threading support.
Building in support for threading will enable Panda to take
advantage of multiple CPU's if you have them (and if the OS
supports kernel threads running on different CPU's), but it will
slightly slow down Panda for the single CPU case." OFF)
endif()


# Configure debug threads
if(CMAKE_BUILD_TYPE MATCHES "Debug")
  option(DEBUG_THREADS "If on, enables debugging of thread and sync operations (i.e. mutexes, deadlocks)" ON)
else()
  option(DEBUG_THREADS "If on, enables debugging of thread and sync operations (i.e. mutexes, deadlocks)" OFF)
endif()

if(HAVE_THREADS)
  set(HAVE_POSIX_THREADS ${CMAKE_USE_PTHREADS_INIT})
  if(HAVE_POSIX_THREADS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pthread")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -pthread")
  endif()
endif()

option(SIMPLE_THREADS
  "If on, compile with simulated threads.  Threads, by default, use
OS-provided threading constructs, which usually allows for full
multithreading support (i.e. the program can use multiple CPU's).
On the other hand, compiling in this full OS-provided support can
impose some substantial runtime overhead, making the application
run slower on a single-CPU machine. This settings avoid the overhead,
but still gain some of the basic functionality of threads." OFF)


option(OS_SIMPLE_THREADS
  "If on, OS threading constructs will be used to perform context switches.
A mutex is used to ensure that only one thread runs at a time, so the
normal SIMPLE_THREADS optimizations still apply, and the normal
SIMPLE_THREADS scheduler is used to switch between threads (instead
of the OS scheduler).  This may be more portable and more reliable,
but it is a hybrid between user-space threads and os-provided threads." ON)


### Configure pipelining ###
option(DO_PIPELINING "If on, compile with pipelined rendering." ON)

### Miscellaneous configuration
option(COMPILE_IN_DEFAULT_FONT
  "If on, compiles in a default font, so that every TextNode will always
have a font available without requiring the user to specify one.
When turned off, the generated library will save a few kilobytes."
  ${IS_NOT_MINSIZE_BUILD})

option(STDFLOAT_DOUBLE
  "Define this true to compile a special version of Panda to use a
'double' floating-precision type for most internal values, such as
positions and transforms, instead of the standard single-precision
'float' type.  This does not affect the default numeric type of
vertices, which is controlled by the runtime config variable
vertices-float64." OFF)
