#define LOCAL_LIBS \
  imagebase progbase
#define OTHER_LIBS \
  egg:c pandaegg:m \
  linmath:c event:c pipeline:c mathutil:c \
  pnmimagetypes:c pnmimage:c putil:c express:c \
  panda:m \
  pandabase:c pandaexpress:m \
  interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

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

