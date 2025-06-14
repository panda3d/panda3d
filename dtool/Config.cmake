#
# dtool/Config.cmake
#
# This file defines certain configuration variables that are written
# into the various make scripts.  It is processed by CMake to
# generate build scripts appropriate to each environment.
#

include(CMakeDependentOption)

# Define our target platform.
# The values "UNIX", "WIN32", "MINGW", "MSYS", and "CYGWIN"
# are automatically provided by CMAKE.  "APPLE" is also provided by
# CMAKE but may be True on systems that are not OS X.
if(CMAKE_SYSTEM_NAME MATCHES "Linux" OR CMAKE_SYSTEM_NAME MATCHES "Android")
  set(IS_LINUX 1)
endif()
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
  set(IS_OSX 1)
endif()
if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  set(IS_FREEBSD 1)
endif()

# Define the type of build we are setting up.

set(_configs Standard Release RelWithDebInfo Debug MinSizeRel)
if(DEFINED CMAKE_CXX_FLAGS_COVERAGE)
  list(APPEND _configs Coverage)
endif()

# Are we building with static or dynamic linking?
if(EMSCRIPTEN OR WASI)
  set(_default_shared OFF)
else()
  set(_default_shared ON)
endif()
option(BUILD_SHARED_LIBS
  "Causes subpackages to be built separately -- setup for dynamic linking.
Utilities/tools/binaries/etc are then dynamically linked to the
libraries instead of being statically linked." ${_default_shared})

option(BUILD_METALIBS
  "Should we build 'metalibs' -- fewer, larger libraries that contain the bulk
of the code instead of many smaller components.  Note that turning this off
will still result in the 'metalibs' being built, but they will instead be many
smaller stub libraries and not 'meta' libraries." ON)

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

if(UNIX)
  set(_default_prc "<auto>etc/panda3d")
else()
  set(_default_prc "<auto>etc")
endif()
set(DEFAULT_PRC_DIR "${_default_prc}" CACHE STRING
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
per_config_option(PRC_SAVE_DESCRIPTIONS
  "Define if you want to save the descriptions for ConfigVariables."
  Debug Standard)

mark_as_advanced(DEFAULT_PRC_DIR PRC_DIR_ENVVARS PRC_PATH_ENVVARS
  PRC_PATTERNS PRC_ENCRYPTED_PATTERNS PRC_ENCRYPTION_KEY
  PRC_EXECUTABLE_PATTERNS PRC_EXECUTABLE_ARGS_ENVVAR
  PRC_PUBLIC_KEYS_FILENAME PRC_RESPECT_TRUST_LEVEL
  PRC_DCONFIG_TRUST_LEVEL PRC_INC_TRUST_LEVEL PRC_SAVE_DESCRIPTIONS)

#
# This is the end of the PRC variable customization section.  The
# remaining variables are of general interest to everyone.
#

# The following options relate to interrogate, the tool that is
# used to generate bindings for non-C++ languages.

cmake_dependent_option(INTERROGATE_PYTHON_INTERFACE
  "Do you want to generate a Python-callable interrogate interface?
This is only necessary if you plan to make calls into Panda from a
program written in Python.  This is done only if HAVE_PYTHON is also
true." ON "HAVE_PYTHON" OFF)

set(INTERROGATE_C_INTERFACE
  "Do you want to generate a C-callable interrogate interface?  This
generates an interface similar to the Python interface above, with
a C calling convention.  It should be useful for most other kinds
of scripting language; the VR Studio used to use this to make calls
into Panda from Squeak." OFF)

set(INTERROGATE_OPTIONS "-fnames;-string;-refcount;-assert" CACHE STRING
  "What additional options should be passed to interrogate when
generating either of the above two interfaces?  Generally, you
probably don't want to mess with this.")

option(INTERROGATE_VERBOSE
  "Set this if you would like interrogate to generate advanced
debugging information." OFF)

set(_default_build_interrogate OFF)
if (INTERROGATE_C_INTERFACE OR INTERROGATE_PYTHON_INTERFACE)
  set(_default_build_interrogate ON)
endif()

option(BUILD_INTERROGATE
  "Do you want to build interrogate from source?  This is necessary
if you wish to build Python or other bindings around Panda3D's C++
interface.  Set this to false if you already have a compatible
version of interrogate installed." ${_default_build_interrogate})

mark_as_advanced(INTERROGATE_OPTIONS)

if(BUILD_INTERROGATE)
  include(ExternalProject)

  set(_interrogate_dir "${PROJECT_BINARY_DIR}/interrogate")

  ExternalProject_Add(
    panda3d-interrogate

    GIT_REPOSITORY https://github.com/panda3d/interrogate.git
    GIT_TAG d2844d994fcc465a4e22b10001d3ac5c4012b814

    PREFIX ${_interrogate_dir}
    CMAKE_ARGS
      -DHAVE_PYTHON=OFF
      -DBUILD_SHARED_LIBS=OFF
      -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>

    EXCLUDE_FROM_ALL ON
    BUILD_BYPRODUCTS "${_interrogate_dir}/bin/interrogate"
                     "${_interrogate_dir}/bin/interrogate_module"
  )

  add_executable(interrogate IMPORTED GLOBAL)
  add_dependencies(interrogate panda3d-interrogate)
  set_target_properties(interrogate PROPERTIES IMPORTED_LOCATION "${_interrogate_dir}/bin/interrogate")

  add_executable(interrogate_module IMPORTED GLOBAL)
  add_dependencies(interrogate_module panda3d-interrogate)
  set_target_properties(interrogate_module PROPERTIES IMPORTED_LOCATION "${_interrogate_dir}/bin/interrogate_module")

else()
  find_program(INTERROGATE_EXECUTABLE interrogate)
  find_program(INTERROGATE_MODULE_EXECUTABLE interrogate_module)

  add_executable(interrogate IMPORTED GLOBAL)
  if(INTERROGATE_EXECUTABLE)
    set_target_properties(interrogate PROPERTIES
      IMPORTED_LOCATION "${INTERROGATE_EXECUTABLE}")

  elseif(INTERROGATE_PYTHON_INTERFACE OR INTERROGATE_C_INTERFACE)
    message(FATAL_ERROR
      "Requested interrogate bindings, but interrogate not found.  Set "
      "BUILD_INTERROGATE to build interrogate from source, or set "
      "INTERROGATE_EXECUTABLE to the location of this tool.")
  endif()

  add_executable(interrogate_module IMPORTED GLOBAL)
  if(INTERROGATE_MODULE_EXECUTABLE)
    set_target_properties(interrogate_module PROPERTIES
      IMPORTED_LOCATION "${INTERROGATE_MODULE_EXECUTABLE}")

  elseif(INTERROGATE_PYTHON_INTERFACE)
    message(FATAL_ERROR
      "Requested interrogate bindings, but interrogate not found.  Set "
      "BUILD_INTERROGATE to build interrogate from source, or set "
      "INTERROGATE_MODULE_EXECUTABLE to the location of this tool.")
  endif()

endif()

#
# The following options have to do with optional debugging features.
#

per_config_option(DO_MEMORY_USAGE
  "Do you want to compile in support for tracking memory usage?  This
enables you to define the variable 'track-memory-usage' at runtime
to help track memory leaks, and also report total memory usage on
PStats.  There is some small overhead for having this ability
available, even if it is unused." Debug Standard)

per_config_option(DO_COLLISION_RECORDING
  "Do you want to enable debugging features for the collision system?"
  Debug Standard)

per_config_option(DO_PSTATS
  "Enable support for performance profiling using PStats?"
  Debug Standard)

per_config_option(DO_DCAST
  "Add safe typecast checking?  This adds significant overhead."
  Debug Standard)

per_config_option(SIMULATE_NETWORK_DELAY
  "This option compiles in support for simulating network delay via
the min-lag and max-lag prc variables.  It adds a tiny bit of
overhead even when it is not activated, so it is typically enabled
only in a development build."
  Debug)

per_config_option(NOTIFY_DEBUG
  "Do you want to include the 'debug' and 'spam' Notify messages?
Normally, these are stripped out when we build for release, but sometimes it's
useful to keep them around. Turn this setting on to achieve that."
  Debug Standard)

mark_as_advanced(SIMULATE_NETWORK_DELAY DO_MEMORY_USAGE DO_DCAST)

#
# The following options have to do with the memory allocation system.
#

find_package(MIMALLOC 1.0 QUIET)

package_option(MIMALLOC
  "The mimalloc allocator.  See also USE_MEMORY_MIMALLOC, which
you will need to use to activate it by default.  If you do not set
USE_MEMORY_MIMALLOC, Panda will decide whether to use it."
  IMPORTED_AS mimalloc-static)

if (WIN32 AND HAVE_MIMALLOC)
  set(_prefer_mimalloc ON)
else()
  set(_prefer_mimalloc OFF)
endif()

option(USE_MEMORY_MIMALLOC
  "This is an optional memory allocator with good multi-threading
support.  It is recommended on Windows, where it gives much better
performance than the built-in malloc.  However, it does not appear
to be significantly faster on glibc-based systems." ${_prefer_mimalloc})

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

if (WIN32 AND NOT HAVE_MIMALLOC)
  option(USE_DELETED_CHAIN
    "Define this true to use the DELETED_CHAIN macros, which support
fast re-use of existing allocated blocks, minimizing the low-level
calls to malloc() and free() for frequently-created and -deleted
objects.  This is significantly better than built-in malloc on Windows
but suffers with multiple threads, where mimalloc performs better, so
it is preferred to get mimalloc instead and turn this OFF." ON)
else()
  option(USE_DELETED_CHAIN
    "Define this true to use the DELETED_CHAIN macros, which support
fast re-use of existing allocated blocks, minimizing the low-level
calls to malloc() and free() for frequently-created and -deleted
objects.  However, modern memory allocators generally perform as good,
especially with threading, so best leave this OFF." OFF)
endif()

mark_as_advanced(USE_MEMORY_DLMALLOC USE_MEMORY_PTMALLOC2
  USE_MEMORY_MIMALLOC MEMORY_HOOK_DO_ALIGN USE_DELETED_CHAIN)

if(USE_MEMORY_MIMALLOC)
  package_status(MIMALLOC "mimalloc memory allocator")
else()
  package_status(MIMALLOC "mimalloc memory allocator (not used)")
endif()

unset(_prefer_mimalloc)

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

set(ANDROID_ABI "armeabi-v7a" CACHE STRING
  "Can be be set to armeabi-v7a, arm64-v8a, x86, or x86_64,
depending on which architecture should be targeted.")
set_property(CACHE ANDROID_ABI PROPERTY STRINGS
  armeabi-v7a arm64-v8a x86 x86_64)

set(ANDROID_STL "c++_shared" CACHE STRING "")
set(ANDROID_PLATFORM "android-14" CACHE STRING "")
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


# By default, we'll assume the user only wants to run with Debug
# python if he has to--that is, on Windows when building a debug build.
if(WIN32)
  per_config_option(USE_DEBUG_PYTHON "" Debug)
else()
  option(USE_DEBUG_PYTHON "" OFF)
endif()

cmake_dependent_option(HAVE_VIDEO4LINUX
  "Set this to enable webcam support on Linux." ON
  "IS_LINUX" OFF)


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

option(SUPPORT_FIXED_FUNCTION
  "This option compiles in support for the fixed-function OpenGL pipeline.
It is only really useful to turn this off to save space if you are building
an application that only needs to use an OpenGL 3.2+ context and only uses
custom GLSL shaders." ON)

option(SUPPORT_IMMEDIATE_MODE
  "This option compiles in support for immediate-mode OpenGL
rendering.  Since this is normally useful only for researching
buggy drivers, and since there is a tiny bit of per-primitive
overhead to have this option available even if it is unused, it is
by default enabled only in a development build.  This has no effect
on DirectX rendering." OFF)

mark_as_advanced(SUPPORT_FIXED_FUNCTION)

# Should build tinydisplay?
option(HAVE_TINYDISPLAY
  "Builds TinyDisplay, a light software renderer based on TinyGL,
that is built into Panda. TinyDisplay is not as full-featured as Mesa
but is many times faster." ON)

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


if(HAVE_GL AND HAVE_X11 AND NOT APPLE)
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

cmake_dependent_option(HAVE_COCOA "Enable Cocoa. Requires Mac OS X." ON
  "APPLE" OFF)

#
# Miscellaneous settings
#

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

option(USE_PANDAFILESTREAM
  "Enable the PandaFileStream implementation of pfstream etc.?" ON)

# These image formats don't require the assistance of a third-party
# library to read and write, so there's normally no reason to disable
# them int he build, unless you are looking to reduce the memory footprint.
option(HAVE_SGI_RGB "Enable support for loading SGI RGB images." ON)
option(HAVE_TGA "Enable support for loading TGA images." ON)
option(HAVE_IMG "Enable support for loading IMG images." ON)
option(HAVE_SOFTIMAGE_PIC "Enable support for loading SOFTIMAGE PIC images." ON)
option(HAVE_BMP "Enable support for loading BMP images." ON)
option(HAVE_PNM "Enable support for loading PNM images." ON)
option(HAVE_SGI_RGB "" ON)
option(HAVE_TGA "" ON)
option(HAVE_IMG "" ON)
option(HAVE_SOFTIMAGE_PIC "" ON)
option(HAVE_BMP "" ON)
option(HAVE_PNM "" ON)

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


### Configure threading support ###
set(CMAKE_THREAD_PREFER_PTHREAD ON)
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads QUIET)

set(THREADS_LIBRARIES "${CMAKE_THREAD_LIBS_INIT}")
set(HAVE_POSIX_THREADS ${CMAKE_USE_PTHREADS_INIT})

# Add basic use flag for threading
if(EMSCRIPTEN OR WASI)
  set(_default_threads OFF)
else()
  set(_default_threads ON)
endif()
package_option(THREADS
  "If on, compile Panda3D with threading support.
Building in support for threading will enable Panda to take
advantage of multiple CPU's if you have them (and if the OS
supports kernel threads running on different CPU's), but it will
slightly slow down Panda for the single CPU case."
  DEFAULT ${_default_threads}
  IMPORTED_AS Threads::Threads)

# Configure debug threads
option(DEBUG_THREADS
  "If on, enables debugging of thread and sync operations (i.e. mutexes,
deadlocks).  Very slow, disabled by default." OFF)

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
When turned off, the generated library will save a few kilobytes." ON)

option(STDFLOAT_DOUBLE
  "Define this true to compile a special version of Panda to use a
'double' floating-precision type for most internal values, such as
positions and transforms, instead of the standard single-precision
'float' type.  This does not affect the default numeric type of
vertices, which is controlled by the runtime config variable
vertices-float64." OFF)
