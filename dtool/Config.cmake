#
# dtool/Config.cmake
#
# This file defines certain configuration variables that are written
# into the various make scripts.  It is processed by CMake to
# generate build scripts appropriate to each environment.
#

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

mark_as_advanced(PRC_DEFAULT_DIR PRC_DIR_ENVVARS PRC_PATH_ENVVARS
  PRC_PATTERNS PRC_ENCRYPTED_PATTERNS PRC_ENCRYPTION_KEY
  PRC_EXECUTABLE_PATTERNS PRC_EXECUTABLE_ARGS_ENVVAR
  PRC_PUBLIC_KEYS_FILENAME PRC_RESPECT_TRUST_LEVEL
  PRC_DCONFIG_TRUST_LEVEL PRC_INC_TRUST_LEVEL)

# This is the end of the PRC variable customization section.  The
# remaining variables are of general interest to everyone.


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

set(PYTHON_NATIVE
  "Define this true to use the new interrogate feature to generate
Python-native objects directly, rather than requiring a separate
FFI step.  This loads and runs much more quickly than the original
mechanism.  Define this false (that is, empty) to use the original
interfaces." ON)

set(INTERROGATE_OPTIONS "-fnames;-string;-refcount;-assert" CACHE STRING
  "What additional options should be passed to interrogate when
generating either of the above two interfaces?  Generally, you
probably don't want to mess with this.")

set(INTERROGATE "interrogate" CACHE STRING
  "What's the name of the interrogate binary to run?  The default
specified is the one that is built as part of DTOOL.  If you have a
prebuilt binary standing by (for instance, if you are cross-compiling
and cannot run the built version), specify its name instead.")

set(INTERROGATE_MODULE "interrogate_module" CACHE STRING
  "Same as INTERROGATE, except for the interrogate_module binary.")

mark_as_advanced(PYTHON_NATIVE INTERROGATE_OPTIONS)

if(NOT CMAKE_CROSSCOMPILING)
  mark_as_advanced(INTERROGATE INTERROGATE_MODULE)
endif()

### Configure threading support ###
find_package(Threads)
if(THREADS_FOUND)
  # Add basic use flag for threading
  option(BUILD_THREADS
    "If on, compile Panda3D with threading support.
Building in support for threading will enable Panda to take
advantage of multiple CPU's if you have them (and if the OS
supports kernel threads running on different CPU's), but it will
slightly slow down Panda for the single CPU case." ON)
  if(BUILD_THREADS)
    set(HAVE_THREADS TRUE)
  else()
    unset(BUILD_SIMPLE_THREADS CACHE)
    unset(BUILD_OS_SIMPLE_THREADS CACHE)
  endif()
else()
  unset(BUILD_THREADS CACHE)
endif()

# Configure debug threads
# Add advanced threading configuration
if(HAVE_THREADS)
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    option(BUILD_DEBUG_THREADS "If on, enables debugging of thread and sync operations (i.e. mutexes, deadlocks)" ON)
  else()
    option(BUILD_DEBUG_THREADS "If on, enables debugging of thread and sync operations (i.e. mutexes, deadlocks)" OFF)
  endif()
  if(BUILD_DEBUG_THREADS)
    set(DEBUG_THREADS TRUE)
  endif()

  set(HAVE_POSIX_THREADS ${CMAKE_USE_PTHREADS_INIT})
  if(HAVE_POSIX_THREADS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -pthread")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -pthread")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -pthread")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -pthread")
  endif()

  option(BUILD_SIMPLE_THREADS
    "If on, compile with simulated threads.  Threads, by default, use
OS-provided threading constructs, which usually allows for full
multithreading support (i.e. the program can use multiple CPU's).
On the other hand, compiling in this full OS-provided support can
impose some substantial runtime overhead, making the application
run slower on a single-CPU machine. This settings avoid the overhead,
but still gain some of the basic functionality of threads." OFF)

  if(BUILD_SIMPLE_THREADS)
    message(STATUS "Compilation will include simulated threading support.")
    option(BUILD_OS_SIMPLE_THREADS
      "If on, OS threading constructs will be used to perform context switches.
A mutex is used to ensure that only one thread runs at a time, so the
normal SIMPLE_THREADS optimizations still apply, and the normal
SIMPLE_THREADS scheduler is used to switch between threads (instead
of the OS scheduler).  This may be more portable and more reliable,
but it is a hybrid between user-space threads and os-provided threads." ON)

    set(SIMPLE_THREADS TRUE)
    if(BUILD_OS_SIMPLE_THREADS)
      set(OS_SIMPLE_THREADS TRUE)
    endif()
  else()
    unset(BUILD_OS_SIMPLE_THREADS CACHE)

    option(BUILD_PIPELINING "If on, compile with pipelined rendering." ON)
    if(BUILD_PIPELINING)
      message(STATUS "Compilation will include full, pipelined threading support.")
    else()
      message(STATUS "Compilation will include nonpipelined threading support.")
    endif()
  endif()
else()
  message(STATUS "Configuring Panda without threading support.")
endif()


### Configure pipelining ###
if(NOT DEFINED BUILD_PIPELINING)
  option(BUILD_PIPELINING "If on, compile with pipelined rendering." ON)
endif()
if(BUILD_PIPELINING)
  set(DO_PIPELINING TRUE)
endif()

### Configure OS X options ###
if(APPLE)
  option(BUILD_UNIVERSIAL_BINARIES "If on, compiling will create universal OS X binaries." ON)
  if(BUILD_UNIVERSAL_BINARIES)
    message(STATUS "Compilation will create universal binaries.")
    set(UNIVERSAL_BINARIES TRUE)
  else()
    message(STATUS "Compilation will not create universal binaries.")
  endif()
endif()

message(STATUS "")
message(STATUS "See dtool_config.h for more details about the specified configuration.\n")

### Miscellaneous configuration
if(CMAKE_BUILD_TYPE MATCHES "MinSizeRel")
  unset(BUILD_WITH_DEFAULT_FONT CACHE)
  option(BUILD_WITH_DEFAULT_FONT
    "If on, compiles in a default font, so that every TextNode will always
have a font available without requiring the user to specify one.
When turned off, the generated library will save a few kilobytes." OFF)
else()
  option(BUILD_WITH_DEFAULT_FONT
    "If on, compiles in a default font, so that every TextNode will always
have a font available without requiring the user to specify one.
When turned off, the generated library will save a few kilobytes." ON)
endif()
if(BUILD_WITH_DEFAULT_FONT)
  set(COMPILE_IN_DEFAULT_FONT TRUE)
endif()

option(BUILD_PREFER_STDFLOAT
  "Define this true to compile a special version of Panda to use a
'double' floating-precision type for most internal values, such as
positions and transforms, instead of the standard single-precision
'float' type.  This does not affect the default numeric type of
vertices, which is controlled by the runtime config variable
vertices-float64." OFF)
if(BUILD_PREFER_STDFLOAT)
  set(STDFLOAT_DOUBLE TRUE)
endif()

#XXX note from rdb: I've moved the automatically-configured
# compiler settings to LocalSetup.cmake, which is also where
# dtool_config.h.cmake is now being invoked.
# LocalSetup.cmake is included in dtool/CMakeLists.txt, which
# is OK since the variables in there don't have to be used
# outside of dtool_config.h.cmake.

