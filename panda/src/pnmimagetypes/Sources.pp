#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

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
    pnmFileTypeYUV.cxx pnmFileTypeYUV.h \
    color.c colrops.c resolu.c header.c

#end lib_target

