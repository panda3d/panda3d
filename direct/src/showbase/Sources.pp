#begin lib_target
  #define TARGET showbase
  #define LOCAL_LIBS \
    directbase
  #define OTHER_LIBS \
    pgraph:c pgraphnodes:c gsgbase:c gobj:c mathutil:c pstatclient:c \
    lerp:c downloader:c pandabase:c pnmimage:c prc:c \
    pipeline:c cull:c \
    $[if $[HAVE_NET],net:c] $[if $[WANT_NATIVE_NET],nativenet:c] \
    display:c linmath:c event:c putil:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m

  #define WIN_SYS_LIBS \
    User32.lib

  #define SOURCES \
    showBase.cxx showBase.h \
    $[if $[IS_OSX],showBase_assist.mm]

  #define IGATESCAN all
#end lib_target

// Define a Python extension module for operating on frozen modules.
// This is a pure C module; it involves no Panda code or C++ code.
#begin lib_target
  #define BUILD_TARGET $[HAVE_PYTHON]
  #define TARGET extend_frozen
  #define LIB_PREFIX
  #if $[OSX_PLATFORM]
    #define LINK_AS_BUNDLE 1
    #define BUNDLE_EXT .so
  #endif
  #if $[WINDOWS_PLATFORM]
    #define DYNAMIC_LIB_EXT .pyd
  #endif

  #define SOURCES extend_frozen.c
#end lib_target

#if $[CTPROJS]
  #define INSTALL_SCRIPTS ppython
#endif

