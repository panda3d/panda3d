#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
#define USE_PACKAGES jpeg png zlib tiff

#begin lib_target
  #define TARGET p3pnmimagetypes
  #define LOCAL_LIBS \
    p3pnmimage

  #define COMBINED_SOURCES \
     $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx

  #define SOURCES  \
     config_pnmimagetypes.h pnmFileTypeBMP.h  \
     pnmFileTypeIMG.h  \
     pnmFileTypePNG.h \
     pnmFileTypePNM.h \
     pnmFileTypePfm.h \
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
     pnmFileTypePfm.cxx \
     pnmFileTypeSGI.cxx \
     pnmFileTypeSGIReader.cxx pnmFileTypeSGIWriter.cxx  \
     pnmFileTypeSoftImage.cxx \
     pnmFileTypeTIFF.cxx \
     pnmFileTypeTGA.cxx

  #define INSTALL_HEADERS \
    config_pnmimagetypes.h

#end lib_target

