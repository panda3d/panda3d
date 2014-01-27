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

# We aggregate the PNMIMAGETYPES libraries depending on which image formats are
# compiled in.
unset(PNMIMAGETYPES_LINK_LIBRARIES)

if(HAVE_JPEG)
  set(PNMIMAGETYPES_LINK_LIBRARIES
    ${PNMIMAGETYPES_LINK_LIBRARIES}
    ${JPEG_LIBRARIES})
endif()

if(HAVE_TIFF)
  set(PNMIMAGETYPES_LINK_LIBRARIES
    ${PNMIMAGETYPES_LINK_LIBRARIES}
    ${TIFF_LIBRARIES})
endif()

if(HAVE_PNG)
  set(PNMIMAGETYPES_LINK_LIBRARIES
    ${PNMIMAGETYPES_LINK_LIBRARIES}
    ${PNG_LIBRARIES})
endif()

composite_sources(p3pnmimagetypes P3PNMIMAGETYPES_SOURCES)
add_library(p3pnmimagetypes ${P3PNMIMAGETYPES_HEADERS} ${P3PNMIMAGETYPES_SOURCES})
target_link_libraries(p3pnmimagetypes p3pnmimage ${PNMIMAGETYPES_LINK_LIBRARIES})
target_interrogate(p3pnmimagetypes ALL)
