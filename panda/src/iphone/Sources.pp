#define BUILD_DIRECTORY $[BUILD_IPHONE]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c 

#define OSX_SYS_FRAMEWORKS Foundation QuartzCore UIKit OpenGLES

#begin bin_target
  #define TARGET iphone_pview

  #define OTHER_LIBS $[OTHER_LIBS] pystub
  #define LOCAL_LIBS \
    iphonedisplay \
    framework putil collide pgraph chan text \
    pnmimage pnmimagetypes event effects gobj display \
    mathutil putil express dgraph device tform \
    linmath pstatclient panda glstuff

  #define SOURCES \
    pview_delegate.h pview_delegate.mm \
    pview_main.mm

#end bin_target
