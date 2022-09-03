set(P3PNMIMAGETYPES_HEADERS
  config_pnmimagetypes.h
  pnmFileTypeBMP.h
  pnmFileTypeEXR.h
  pnmFileTypeIMG.h
  pnmFileTypePNG.h
  pnmFileTypePNM.h
  pnmFileTypePfm.h
  pnmFileTypeSGI.h pnmFileTypeSoftImage.h
  pnmFileTypeStbImage.h
  pnmFileTypeTGA.h
  pnmFileTypeTIFF.h
  pnmFileTypeJPG.h
  sgi.h
)

set(P3PNMIMAGETYPES_SOURCES
  config_pnmimagetypes.cxx
  pnmFileTypeBMPReader.cxx pnmFileTypeBMPWriter.cxx
  pnmFileTypeBMP.cxx
  pnmFileTypeEXR.cxx
  pnmFileTypeIMG.cxx
  pnmFileTypeJPG.cxx pnmFileTypeJPGReader.cxx pnmFileTypeJPGWriter.cxx
  pnmFileTypePNG.cxx
  pnmFileTypePNM.cxx
  pnmFileTypePfm.cxx
  pnmFileTypeSGI.cxx
  pnmFileTypeSGIReader.cxx pnmFileTypeSGIWriter.cxx
  pnmFileTypeSoftImage.cxx
  pnmFileTypeStbImage.cxx
  pnmFileTypeTIFF.cxx
  pnmFileTypeTGA.cxx
)

composite_sources(p3pnmimagetypes P3PNMIMAGETYPES_SOURCES)
add_component_library(p3pnmimagetypes SYMBOL BUILDING_PANDA_PNMIMAGETYPES
  ${P3PNMIMAGETYPES_HEADERS} ${P3PNMIMAGETYPES_SOURCES})
target_link_libraries(p3pnmimagetypes p3pnmimage
  PKG::JPEG PKG::TIFF PKG::PNG PKG::OPENEXR)
set_target_properties(p3pnmimagetypes PROPERTIES CXX_EXCEPTIONS ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  # Do not re-export symbols from these libraries.
  if(HAVE_JPEG)
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libjpeg.a")
  endif()

  if(HAVE_TIFF)
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libtiff.a")
  endif()

  if(HAVE_PNG)
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libpng.a")
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libpng16.a")
  endif()

  if(HAVE_OPENEXR)
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libHalf.a")
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libIex.a")
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libIexMath.a")
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libIlmImf.a")
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libIlmImfUtil.a")
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libIlmThread.a")
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libImath.a")
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libOpenEXR.a")
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libOpenEXRCore.a")
    target_link_options(p3pnmimagetypes PRIVATE "LINKER:--exclude-libs,libOpenEXRUtil.a")
  endif()
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3pnmimagetypes
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE COMPONENT CoreDevel)
endif()
