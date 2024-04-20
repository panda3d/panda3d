set(P3PNMIMAGE_HEADERS
  config_pnmimage.h
  convert_srgb.h convert_srgb.I
  pfmFile.I pfmFile.h
  pnmbitio.h
  pnmBrush.h pnmBrush.I
  pnmFileType.h pnmFileTypeRegistry.h pnmImage.I
  pnmImage.h pnmImageHeader.I pnmImageHeader.h
  pnmPainter.h pnmPainter.I
  pnmReader.I
  pnmReader.h pnmWriter.I pnmWriter.h pnmimage_base.h
  pnmReaderEmscripten.h
  ppmcmap.h
)

set(P3PNMIMAGE_SOURCES
  config_pnmimage.cxx
  convert_srgb.cxx
  convert_srgb_sse2.cxx
  pfmFile.cxx
  pnm-image-filter.cxx
  pnmbitio.cxx
  pnmBrush.cxx
  pnmFileType.cxx
  pnmFileTypeRegistry.cxx pnmImage.cxx pnmImageHeader.cxx
  pnmPainter.cxx
  pnmReader.cxx pnmWriter.cxx pnmimage_base.cxx
  pnmReaderEmscripten.cxx
  ppmcmap.cxx
)

set(P3PNMIMAGE_IGATEEXT
  pfmFile_ext.cxx
  pfmFile_ext.h
)

if(HAVE_SSE2 AND CMAKE_SIZEOF_VOID_P EQUAL 4)
  # It's only necessary to do this on 32-bit x86; 64-bit makes SSE2 builtin.
  if(MSVC)
    set_source_files_properties(convert_srgb_sse2.cxx PROPERTIES
      SKIP_UNITY_BUILD_INCLUSION YES
      COMPILE_FLAGS /arch:SSE2)
  else()
    set_source_files_properties(convert_srgb_sse2.cxx PROPERTIES
      SKIP_UNITY_BUILD_INCLUSION YES
      COMPILE_FLAGS -msse2)
  endif()
endif()

composite_sources(p3pnmimage P3PNMIMAGE_SOURCES)
add_component_library(p3pnmimage SYMBOL BUILDING_PANDA_PNMIMAGE
  ${P3PNMIMAGE_HEADERS} ${P3PNMIMAGE_SOURCES})
target_link_libraries(p3pnmimage p3mathutil)
target_interrogate(p3pnmimage ALL EXTENSIONS ${P3PNMIMAGE_IGATEEXT})

if(NOT BUILD_METALIBS)
  install(TARGETS p3pnmimage
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3PNMIMAGE_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
