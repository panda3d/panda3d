#define BUILD_DIRECTORY $[and $[IS_OSX],$[HAVE_GL],$[HAVE_COCOA]]

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m

#define OSX_SYS_FRAMEWORKS ApplicationServices AppKit Carbon

#begin lib_target
  #define TARGET p3cocoadisplay
  #define LOCAL_LIBS \
    p3display p3putil p3glgsg

  #define COMBINED_SOURCES $[TARGET]_composite1.mm
  
  #define INSTALL_HEADERS \
     config_cocoadisplay.h \
     cocoaGraphicsPipe.h cocoaGraphicsPipe.I \
     cocoaGraphicsWindow.h cocoaGraphicsWindow.I \
     cocoaGraphicsStateGuardian.h cocoaGraphicsStateGuardian.I \
     cocoaPandaApp.h cocoaPandaView.h cocoaPandaWindowDelegate.h
    
  #define INCLUDED_SOURCES \
    config_cocoadisplay.mm \
    cocoaGraphicsPipe.mm \
    cocoaGraphicsStateGuardian.mm \
    cocoaGraphicsWindow.mm \
    cocoaPandaApp.mm \
    cocoaPandaView.mm \
    cocoaPandaWindow.mm \
    cocoaPandaWindowDelegate.mm

  #define SOURCES \
    $[INSTALL_HEADERS]

#end lib_target
