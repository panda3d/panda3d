#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET pnmimage
  #define LOCAL_LIBS \
    linmath putil express

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
     config_pnmimage.h \
     pnmbitio.h \
     pnmFileType.h pnmFileTypeRegistry.h pnmImage.I  \
     pnmImage.h pnmImageHeader.I pnmImageHeader.h pnmReader.I  \
     pnmReader.h pnmWriter.I pnmWriter.h pnmimage_base.h \
     ppmcmap.h

  #define INCLUDED_SOURCES \
     config_pnmimage.cxx \
     pnmbitio.cxx \
     pnm-image-filter.cxx pnmFileType.cxx  \
     pnmFileTypeRegistry.cxx pnmImage.cxx pnmImageHeader.cxx  \
     pnmReader.cxx pnmWriter.cxx pnmimage_base.cxx \
     ppmcmap.cxx

  #define INSTALL_HEADERS \
    config_pnmimage.h pnmFileType.h pnmFileTypeRegistry.h pnmImage.I \
    pnmImage.h pnmImageHeader.I pnmImageHeader.h pnmReader.I \
    pnmReader.h pnmWriter.I pnmWriter.h pnmimage_base.h

  #define IGATESCAN all

#end lib_target

