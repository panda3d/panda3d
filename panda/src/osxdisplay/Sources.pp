#define BUILD_DIRECTORY $[and $[IS_OSX],$[HAVE_GL],$[HAVE_CARBON]]

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c 

#define OSX_SYS_FRAMEWORKS ApplicationServices Carbon AGL CoreServices Cocoa  
#define USE_PACKAGES gl cggl 

#begin lib_target
  #define TARGET p3osxdisplay
  #define LOCAL_LIBS \
    p3display p3putil p3glgsg

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx
  
  #define INSTALL_HEADERS \
     config_osxdisplay.h \
     osxGraphicsPipe.h \
     osxGraphicsWindow.h osxGraphicsWindow.I \
     osxGraphicsStateGuardian.h
    
  #define INCLUDED_SOURCES \
    config_osxdisplay.cxx osxGraphicsPipe.cxx osxGraphicsStateGuardian.cxx osxGraphicsBuffer.cxx

  #define SOURCES \
    $[INSTALL_HEADERS] osxGraphicsWindow.mm

#end lib_target
