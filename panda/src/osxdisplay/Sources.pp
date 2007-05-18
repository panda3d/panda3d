#define BUILD_DIRECTORY $[IS_OSX]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c 

#define OSX_SYS_FRAMEWORKS ApplicationServices Carbon AGL CoreServices Cocoa  
#define USE_PACKAGES gl cggl 

#begin lib_target
  #define TARGET osxdisplay
  #define LOCAL_LIBS \
    display putil glgsg
    
  
  #define INSTALL_HEADERS \
     config_osxdisplay.h \
     osxGraphicsPipe.h \
     osxGraphicsWindow.h \
     osxGraphicsStateGuardian.h
    
  #define INCLUDED_SOURCES \
    config_osxdisplay.cxx osxGraphicsPipe.cxx osxGraphicsWindow.mm osxGraphicsStateGuardian.cxx osxGraphicsBuffer.cxx

  #define SOURCES \
    osxDisplay.xx $[INCLUDED_SOURCES] $[INSTALL_HEADERS]

#end lib_target
