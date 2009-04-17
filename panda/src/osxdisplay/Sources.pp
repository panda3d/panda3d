#define BUILD_DIRECTORY $[and $[IS_OSX],$[HAVE_GL]]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c 

#define OSX_SYS_FRAMEWORKS ApplicationServices Carbon AGL CoreServices Cocoa  
#define USE_PACKAGES gl cggl 

#begin lib_target
  #define TARGET osxdisplay
  #define LOCAL_LIBS \
    display putil glgsg

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
