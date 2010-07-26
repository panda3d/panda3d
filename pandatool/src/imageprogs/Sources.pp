#define LOCAL_LIBS \
  imagebase progbase

#define OTHER_LIBS \
    pipeline:c event:c pstatclient:c panda:m \
    pandabase:c pnmimage:c mathutil:c linmath:c putil:c express:c \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m \
    $[if $[WANT_NATIVE_NET],nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],net:c downloader:c]

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

