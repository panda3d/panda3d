#begin bin_target
  #define TARGET image-trans
  #define LOCAL_LIBS \
    imagebase progbase
  #define OTHER_LIBS \
    pnmimagetypes:c pnmimage:c putil:c express:c panda:m dtool
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    imageTrans.cxx imageTrans.h

  #define INSTALL_HEADERS \

#end bin_target

