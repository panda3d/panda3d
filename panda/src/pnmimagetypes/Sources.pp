#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c
#define USE_PACKAGES jpeg png zlib tiff

#begin lib_target
  #define TARGET pnmimagetypes
  #define LOCAL_LIBS \
    pnmimage

  #define COMBINED_SOURCES \
     $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES  \
     config_pnmimagetypes.h pnmFileTypeBMP.h  \
     pnmFileTypeIMG.h  \
     pnmFileTypePNG.h \
     pnmFileTypePNM.h \
     pnmFileTypeSGI.h pnmFileTypeSoftImage.h  \
     pnmFileTypeTGA.h \
     pnmFileTypeTIFF.h \
     pnmFileTypeJPG.h

  #define INCLUDED_SOURCES  \
     config_pnmimagetypes.cxx  \
     pnmFileTypeBMPReader.cxx pnmFileTypeBMPWriter.cxx  \
     pnmFileTypeBMP.cxx \
     pnmFileTypeIMG.cxx \
     pnmFileTypeJPG.cxx pnmFileTypeJPGReader.cxx pnmFileTypeJPGWriter.cxx \
     pnmFileTypePNG.cxx \
     pnmFileTypePNM.cxx \
     pnmFileTypeSGI.cxx \
     pnmFileTypeSGIReader.cxx pnmFileTypeSGIWriter.cxx  \
     pnmFileTypeSoftImage.cxx \
     pnmFileTypeTIFF.cxx \
     pnmFileTypeTGA.cxx

  #define INSTALL_HEADERS \
    config_pnmimagetypes.h

#end lib_target

