#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_JPEG yes

#begin lib_target
  #define TARGET pnmimagetypes
  #define LOCAL_LIBS \
    pnm tiff pnmimage

  #define SOURCES \
    config_pnmimagetypes.cxx config_pnmimagetypes.h \
    pnmFileTypeAlias.cxx pnmFileTypeAlias.h pnmFileTypeBMP.cxx \
    pnmFileTypeBMP.h pnmFileTypeBMPReader.cxx pnmFileTypeBMPWriter.cxx \
    pnmFileTypeIMG.cxx pnmFileTypeIMG.h pnmFileTypePNM.cxx \
    pnmFileTypePNM.h pnmFileTypeRadiance.cxx pnmFileTypeRadiance.h \
    pnmFileTypeSGI.cxx pnmFileTypeSGI.h pnmFileTypeSGIReader.cxx \
    pnmFileTypeSGIWriter.cxx pnmFileTypeSoftImage.cxx \
    pnmFileTypeSoftImage.h pnmFileTypeTIFF.cxx pnmFileTypeTIFF.h \
    pnmFileTypeTGA.cxx pnmFileTypeTGA.h \
    pnmFileTypeYUV.cxx pnmFileTypeYUV.h \
    color.c colrops.c resolu.c header.c

  #define IF_JPEG_SOURCES \
    pnmFileTypeJPG.cxx pnmFileTypeJPGReader.cxx pnmFileTypeJPGWriter.cxx

  #define INSTALL_HEADERS \
    config_pnmimagetypes.h

#end lib_target

