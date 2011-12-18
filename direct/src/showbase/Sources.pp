#begin lib_target
  #define TARGET p3showbase
  #define LOCAL_LIBS \
    p3directbase
  #define OTHER_LIBS \
    p3pgraph:c p3pgraphnodes:c p3gsgbase:c p3gobj:c p3mathutil:c p3pstatclient:c \
    p3downloader:c p3pandabase:c p3pnmimage:c p3prc:c \
    p3pipeline:c p3cull:c \
    $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    p3display:c p3linmath:c p3event:c p3putil:c panda:m \
    p3express:c pandaexpress:m \
    p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m

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
  #define TARGET p3extend_frozen
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

