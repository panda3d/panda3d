#define LOCAL_LIBS \
  imagebase progbase
#define OTHER_LIBS \
  egg:c pandaegg:m \
  pnmimagetypes:c pnmimage:c putil:c express:c panda:m \
  pandaexpress:m \
  dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub
#define UNIX_SYS_LIBS \
  m

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
  #define TARGET image-info
  #define SOURCES \
    imageInfo.cxx imageInfo.h
#end bin_target

