if(NOT HAVE_FREETYPE)
  return()
endif()

set(P3PNMTEXT_HEADERS
  config_pnmtext.h
  freetypeFace.h freetypeFace.I
  freetypeFont.h freetypeFont.I
  pnmTextGlyph.h pnmTextGlyph.I
  pnmTextMaker.h pnmTextMaker.I
)

set(P3PNMTEXT_SOURCES
  config_pnmtext.cxx
  freetypeFace.cxx
  freetypeFont.cxx
  pnmTextGlyph.cxx
  pnmTextMaker.cxx
)

composite_sources(p3pnmtext P3PNMTEXT_SOURCES)
add_component_library(p3pnmtext SYMBOL BUILDING_PANDA_PNMTEXT
  ${P3PNMTEXT_HEADERS} ${P3PNMTEXT_SOURCES})
target_link_libraries(p3pnmtext p3parametrics p3pnmimage PKG::FREETYPE)
target_interrogate(p3pnmtext ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3pnmtext
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3PNMTEXT_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
