#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET pnmimage
  #define LOCAL_LIBS \
    pnm linmath

  #define SOURCES \
    config_pnmimage.cxx config_pnmimage.h pnm-image-filter.cxx \
    pnmFileType.cxx pnmFileType.h pnmFileTypeRegistry.cxx \
    pnmFileTypeRegistry.h pnmImage.I pnmImage.cxx pnmImage.h \
    pnmImageHeader.I pnmImageHeader.cxx pnmImageHeader.h pnmReader.I \
    pnmReader.cxx pnmReader.h pnmWriter.I pnmWriter.cxx pnmWriter.h

  #define INSTALL_HEADERS \
    config_pnmimage.h pnmFileType.h pnmFileTypeRegistry.h pnmImage.I \
    pnmImage.h pnmImageHeader.I pnmImageHeader.h pnmReader.I \
    pnmReader.h pnmWriter.I pnmWriter.h pnmimage_base.h

#end lib_target

