#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES jpeg png zlib tiff

#begin lib_target
  #define TARGET pnmimagetypes
  #define LOCAL_LIBS \
    pnmimage

  #define COMBINED_SOURCES \
     $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES  \
     config_pnmimagetypes.h pnmFileTypeAlias.h pnmFileTypeBMP.h  \
     pnmFileTypeIMG.h  \
     pnmFileTypePNG.cxx pnmFileTypePNG.h \
     pnmFileTypeSGI.h pnmFileTypeSoftImage.h  \
     pnmFileTypeTGA.h \
     pnmFileTypeTIFF.cxx pnmFileTypeTIFF.h \
     pnmFileTypeJPG.h

  #define INCLUDED_SOURCES  \
     config_pnmimagetypes.cxx pnmFileTypeAlias.cxx  \
     pnmFileTypeBMPReader.cxx pnmFileTypeBMPWriter.cxx  \
     pnmFileTypeBMP.cxx \
     pnmFileTypeIMG.cxx \
     pnmFileTypeJPG.cxx pnmFileTypeJPGReader.cxx pnmFileTypeJPGWriter.cxx \
     pnmFileTypeSGI.cxx \
     pnmFileTypeSGIReader.cxx pnmFileTypeSGIWriter.cxx  \
     pnmFileTypeSoftImage.cxx \
     pnmFileTypeTGA.cxx \

  #define INSTALL_HEADERS \
    config_pnmimagetypes.h

#end lib_target

