set(P3PNMIMAGE_HEADERS
  config_pnmimage.h
  pfmFile.I pfmFile.h
  pnmbitio.h
  pnmBrush.h pnmBrush.I
  pnmFileType.h pnmFileTypeRegistry.h pnmImage.I 
  pnmImage.h pnmImageHeader.I pnmImageHeader.h 
  pnmPainter.h pnmPainter.I
  pnmReader.I
  pnmReader.h pnmWriter.I pnmWriter.h pnmimage_base.h
  ppmcmap.h
)

set(P3PNMIMAGE_SOURCES
  config_pnmimage.cxx
  pfmFile.cxx
  pnm-image-filter.cxx
  pnmbitio.cxx
  pnmBrush.cxx
  pnmFileType.cxx 
  pnmFileTypeRegistry.cxx pnmImage.cxx pnmImageHeader.cxx 
  pnmPainter.cxx
  pnmReader.cxx pnmWriter.cxx pnmimage_base.cxx
  ppmcmap.cxx
)

composite_sources(p3pnmimage P3PNMIMAGE_SOURCES)
add_library(p3pnmimage ${P3PNMIMAGE_HEADERS} ${P3PNMIMAGE_SOURCES})
target_link_libraries(p3pnmimage p3mathutil)
target_interrogate(p3pnmimage ALL)

install(TARGETS p3pnmimage DESTINATION lib)
