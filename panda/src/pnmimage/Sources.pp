#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET pnmimage
  #define LOCAL_LIBS \
    pnm linmath putil express

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
     config_pnmimage.h pnmFileType.h pnmFileTypeRegistry.h pnmImage.I  \
     pnmImage.h pnmImageHeader.I pnmImageHeader.h pnmReader.I  \
     pnmReader.h pnmWriter.I pnmWriter.h  

  #define INCLUDED_SOURCES \
     config_pnmimage.cxx pnm-image-filter.cxx pnmFileType.cxx  \
     pnmFileTypeRegistry.cxx pnmImage.cxx pnmImageHeader.cxx  \
     pnmReader.cxx pnmWriter.cxx 

  #define INSTALL_HEADERS \
    config_pnmimage.h pnmFileType.h pnmFileTypeRegistry.h pnmImage.I \
    pnmImage.h pnmImageHeader.I pnmImageHeader.h pnmReader.I \
    pnmReader.h pnmWriter.I pnmWriter.h pnmimage_base.h

#end lib_target

