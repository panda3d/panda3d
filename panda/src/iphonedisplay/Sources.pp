#define BUILD_DIRECTORY $[BUILD_IPHONE]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c 

#define OSX_SYS_FRAMEWORKS Foundation QuartzCore UIKit OpenGLES

#begin lib_target
  #define TARGET iphonedisplay

  #define LOCAL_LIBS \
    framework putil collide pgraph chan text \
    pnmimage pnmimagetypes event effects gobj display \
    mathutil putil express dgraph device tform \
    linmath pstatclient panda glstuff

  #define SOURCES \
    config_iphonedisplay.h config_iphonedisplay.mm \
    viewController.h viewController.mm \
    eaglView.h eaglView.mm \
    glesext_shadow.h \
    glesgsg.h glesgsg.mm \
    iPhoneGraphicsPipe.h iPhoneGraphicsPipe.mm \
    iPhoneGraphicsStateGuardian.h iPhoneGraphicsStateGuardian.mm \
    iPhoneGraphicsWindow.h iPhoneGraphicsWindow.I iPhoneGraphicsWindow.mm

  #define INSTALL_HEADERS \
    config_iphonedisplay.h \
    viewController.h \
    iPhoneGraphicsPipe.h \
    iPhoneGraphicsStateGuardian.h \
    iPhoneGraphicsWindow.h iPhoneGraphicsWindow.I

#end lib_target
