#begin lib_target
  #define TARGET imagebase
  #define LOCAL_LIBS \
    progbase
  #define OTHER_LIBS \
    pnmimage:c panda:m

  #define SOURCES \
    imageBase.cxx imageBase.h imageFilter.cxx imageFilter.h \
    imageReader.cxx imageReader.h imageWriter.I imageWriter.cxx \
    imageWriter.h

  #define INSTALL_HEADERS \
    imageBase.h imageFilter.h imageReader.h imageWriter.I imageWriter.h

#end lib_target

