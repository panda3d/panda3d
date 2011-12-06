#define BUILD_DIRECTORY $[BUILD_IPHONE]

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c 

#define OSX_SYS_FRAMEWORKS Foundation QuartzCore UIKit OpenGLES

#begin lib_target
  #define TARGET p3iphonedisplay

  #define LOCAL_LIBS \
    p3framework p3putil p3collide p3pgraph p3chan p3text \
    p3pnmimage p3pnmimagetypes p3event p3gobj p3display \
    p3mathutil p3putil p3express p3dgraph p3device p3tform \
    p3linmath p3pstatclient panda p3glstuff p3glesgsg

  #define SOURCES \
    config_iphonedisplay.h config_iphonedisplay.mm \
    viewController.h viewController.mm \
    eaglView.h eaglView.mm \
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
