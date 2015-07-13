#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
#define USE_PACKAGES zlib

#begin lib_target
  #define TARGET p3pnmimage
  #define LOCAL_LIBS \
    p3linmath p3putil p3express p3mathutil

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
     config_pnmimage.h \
     convert_srgb.I convert_srgb.h \
     convert_srgb_sse2.cxx \
     pfmFile.I pfmFile.h \
     pfmFile_ext.cxx pfmFile_ext.h \
     pnmbitio.h \
     pnmBrush.h pnmBrush.I \
     pnmFileType.h pnmFileTypeRegistry.h pnmImage.I  \
     pnmImage.h pnmImageHeader.I pnmImageHeader.h  \
     pnmPainter.h pnmPainter.I \
     pnmReader.I \
     pnmReader.h pnmWriter.I pnmWriter.h pnmimage_base.h \
     ppmcmap.h

  #define INCLUDED_SOURCES \
     config_pnmimage.cxx \
     convert_srgb.cxx \
     pfmFile.cxx \
     pnm-image-filter.cxx \
     pnmbitio.cxx \
     pnmBrush.cxx \
     pnmFileType.cxx  \
     pnmFileTypeRegistry.cxx pnmImage.cxx pnmImageHeader.cxx  \
     pnmPainter.cxx \
     pnmReader.cxx pnmWriter.cxx pnmimage_base.cxx \
     ppmcmap.cxx

  #define INSTALL_HEADERS \
     config_pnmimage.h \
     convert_srgb.I convert_srgb.h \
     pfmFile.I pfmFile.h \
     pfmFile_ext.cxx pfmFile_ext.h \
     pnmBrush.h pnmBrush.I \
     pnmFileType.h pnmFileTypeRegistry.h pnmImage.I \
     pnmImage.h pnmImageHeader.I pnmImageHeader.h \
     pnmPainter.h pnmPainter.I \
     pnmReader.I \
     pnmReader.h pnmWriter.I pnmWriter.h pnmimage_base.h

  #define IGATESCAN all

#end lib_target

