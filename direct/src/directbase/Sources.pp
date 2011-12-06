#define USE_PACKAGES cg  // from gobj.

#begin lib_target
  #define TARGET p3directbase
  
  #define SOURCES \
    directbase.cxx directbase.h directsymbols.h \

  #define INSTALL_HEADERS \
    directbase.h directsymbols.h

  // These libraries and frameworks are used by dtoolutil; we redefine
  // them here so they get into the panda build system.
  #if $[ne $[PLATFORM], FreeBSD]
    #define UNIX_SYS_LIBS dl
  #endif
  #define WIN_SYS_LIBS shell32.lib
  #define OSX_SYS_FRAMEWORKS Foundation $[if $[not $[BUILD_IPHONE]],AppKit]

#end lib_target
