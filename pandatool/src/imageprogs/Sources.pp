#define LOCAL_LIBS \
  p3imagebase p3progbase

#define OTHER_LIBS \
    p3pipeline:c p3event:c p3pstatclient:c p3grutil:c \
    panda:m \
    p3pandabase:c p3pnmimage:c p3pnmimagetypes:c \
    p3mathutil:c p3linmath:c p3putil:c p3express:c \
    pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],p3net:c p3downloader:c] \
    p3pystub

#begin bin_target
  #define TARGET image-trans
  #define SOURCES \
    imageTrans.cxx imageTrans.h
#end bin_target

#begin bin_target
  #define TARGET image-resize
  #define SOURCES \
    imageResize.cxx imageResize.h imageResize.I
#end bin_target

#begin bin_target
  #define TARGET image-fix-hidden-color
  #define SOURCES \
    imageFixHiddenColor.cxx imageFixHiddenColor.h imageFixHiddenColor.I
#end bin_target

#begin bin_target
  #define TARGET image-transform-colors
  #define SOURCES \
    imageTransformColors.cxx imageTransformColors.h imageTransformColors.I
#end bin_target

#begin bin_target
  #define TARGET image-info
  #define SOURCES \
    imageInfo.cxx imageInfo.h
#end bin_target

