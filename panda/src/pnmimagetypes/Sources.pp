#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_JPEG yes

#begin lib_target
  #define TARGET pnmimagetypes
  #define LOCAL_LIBS \
    pnm tiff pnmimage

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES  \
     config_pnmimagetypes.h pnmFileTypeAlias.h pnmFileTypeBMP.h  \
     pnmFileTypeIMG.h pnmFileTypePNM.h pnmFileTypeRadiance.h  \
     pnmFileTypeSGI.h pnmFileTypeSoftImage.h pnmFileTypeTIFF.h  \
     pnmFileTypeTGA.h pnmFileTypeYUV.h color.c colrops.c resolu.c  \
     header.c  

  #define INCLUDED_SOURCES  \
     config_pnmimagetypes.cxx pnmFileTypeAlias.cxx  \
     pnmFileTypeBMPReader.cxx pnmFileTypeBMPWriter.cxx  \
     pnmFileTypeIMG.cxx pnmFileTypePNM.cxx pnmFileTypeBMP.cxx  \
     pnmFileTypeRadiance.cxx pnmFileTypeSGI.cxx  \
     pnmFileTypeSGIReader.cxx pnmFileTypeSGIWriter.cxx  \
     pnmFileTypeSoftImage.cxx pnmFileTypeTIFF.cxx  \
     pnmFileTypeTGA.cxx pnmFileTypeYUV.cxx
     

  #define IF_JPEG_SOURCES \
    pnmFileTypeJPG.cxx pnmFileTypeJPG.h \
    pnmFileTypeJPGReader.cxx pnmFileTypeJPGWriter.cxx

  #define INSTALL_HEADERS \
    config_pnmimagetypes.h

#end lib_target

