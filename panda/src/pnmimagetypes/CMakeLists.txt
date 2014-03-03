set(P3PNMIMAGETYPES_HEADERS
  config_pnmimagetypes.h pnmFileTypeBMP.h
  pnmFileTypeIMG.h
  pnmFileTypePNG.h
  pnmFileTypePNM.h
  pnmFileTypePfm.h
  pnmFileTypeSGI.h pnmFileTypeSoftImage.h
  pnmFileTypeTGA.h
  pnmFileTypeTIFF.h
  pnmFileTypeJPG.h
)

set(P3PNMIMAGETYPES_SOURCES
  config_pnmimagetypes.cxx
  pnmFileTypeBMPReader.cxx pnmFileTypeBMPWriter.cxx
  pnmFileTypeBMP.cxx
  pnmFileTypeIMG.cxx
  pnmFileTypeJPG.cxx pnmFileTypeJPGReader.cxx pnmFileTypeJPGWriter.cxx
  pnmFileTypePNG.cxx
  pnmFileTypePNM.cxx
  pnmFileTypePfm.cxx
  pnmFileTypeSGI.cxx
  pnmFileTypeSGIReader.cxx pnmFileTypeSGIWriter.cxx
  pnmFileTypeSoftImage.cxx
  pnmFileTypeTIFF.cxx
  pnmFileTypeTGA.cxx
)

composite_sources(p3pnmimagetypes P3PNMIMAGETYPES_SOURCES)
add_library(p3pnmimagetypes ${P3PNMIMAGETYPES_HEADERS} ${P3PNMIMAGETYPES_SOURCES})
target_link_libraries(p3pnmimagetypes p3pnmimage ${_JPEG_LIBRARIES}
  ${_TIFF_LIBRARIES} ${_PNG_LIBRARIES})

install(TARGETS p3pnmimagetypes DESTINATION lib)
