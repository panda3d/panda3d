#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES tiff jpeg jpeg2000

#begin lib_target
  #define TARGET pnmimagetypes
  #define LOCAL_LIBS \
    pnm pnmimage

  #define COMBINED_SOURCES \
     $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx \
     $[if $[HAVE_JPEG], $[TARGET]_composite3.cxx] \
     $[if $[HAVE_JPEG2000], $[TARGET]_composite4.cxx]

  #define SOURCES  \
     config_pnmimagetypes.h pnmFileTypeAlias.h pnmFileTypeBMP.h  \
     pnmFileTypeIMG.h pnmFileTypePNM.h pnmFileTypeRadiance.h  \
     pnmFileTypeSGI.h pnmFileTypeSoftImage.h  \
     pnmFileTypeTGA.h pnmFileTypeYUV.h color.c colrops.c resolu.c  \
     header.c \
     $[if $[HAVE_TIFF], pnmFileTypeTIFF.cxx pnmFileTypeTIFF.h] \
     $[if $[HAVE_JPEG], pnmFileTypeJPG.h] \
     $[if $[HAVE_JPEG2000], pnmFileTypeJPG2000.h]

  #define INCLUDED_SOURCES  \
     config_pnmimagetypes.cxx pnmFileTypeAlias.cxx  \
     pnmFileTypeBMPReader.cxx pnmFileTypeBMPWriter.cxx  \
     pnmFileTypeIMG.cxx pnmFileTypePNM.cxx pnmFileTypeBMP.cxx  \
     pnmFileTypeRadiance.cxx pnmFileTypeSGI.cxx  \
     pnmFileTypeSGIReader.cxx pnmFileTypeSGIWriter.cxx  \
     pnmFileTypeSoftImage.cxx \
     pnmFileTypeTGA.cxx pnmFileTypeYUV.cxx \
     $[if $[HAVE_JPEG], pnmFileTypeJPG.cxx pnmFileTypeJPGReader.cxx pnmFileTypeJPGWriter.cxx] \
     $[if $[HAVE_JPEG2000], pnmFileTypeJPG2000.cxx pnmFileTypeJPG2000Reader.cxx pnmFileTypeJPG2000Writer.cxx]

  #define INSTALL_HEADERS \
    config_pnmimagetypes.h

#end lib_target

