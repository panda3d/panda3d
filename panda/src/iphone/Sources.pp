#define BUILD_DIRECTORY $[BUILD_IPHONE]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c 

#define OSX_SYS_FRAMEWORKS Foundation QuartzCore UIKit OpenGLES
#define BUILDING_DLL BUILDING_PANDAGL

#begin bin_target
  #define TARGET iphone_pview
  #define LOCAL_LIBS \
    framework putil collide pgraph chan text \
    pnmimage pnmimagetypes event effects gobj display \
    mathutil putil express dgraph device tform \
    linmath pstatclient panda glstuff

  #define SOURCES \
    config_iphone.h config_iphone.mm \
    pview_delegate.h pview_delegate.mm \
    viewController.h viewController.mm \
    eaglView.h eaglView.mm \
    glesext_shadow.h \
    glesgsg.h glesgsg.mm \
    iPhoneGraphicsPipe.h iPhoneGraphicsPipe.mm \
    iPhoneGraphicsStateGuardian.h iPhoneGraphicsStateGuardian.mm \
    iPhoneGraphicsWindow.h iPhoneGraphicsWindow.I iPhoneGraphicsWindow.mm \
    main.mm

#end bin_target

//#begin bin_target
//  #define TARGET iphone_pview
//  #define SOURCES main.mm
//#end bin_target

