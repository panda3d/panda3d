set(P3TEXT_HEADERS
  config_text.h
  default_font.h
  dynamicTextFont.I dynamicTextFont.h
  dynamicTextGlyph.I dynamicTextGlyph.h
  dynamicTextPage.I dynamicTextPage.h
  fontPool.I fontPool.h
  geomTextGlyph.I geomTextGlyph.h
  staticTextFont.I staticTextFont.h
  textAssembler.I textAssembler.h
  textFont.I textFont.h
  textGlyph.I textGlyph.h
  textGraphic.I textGraphic.h
  textNode.I textNode.h
  textProperties.I textProperties.h
  textPropertiesManager.I textPropertiesManager.h
)

set(P3TEXT_SOURCES
  config_text.cxx
  default_font.cxx
  dynamicTextFont.cxx
  dynamicTextGlyph.cxx
  dynamicTextPage.cxx
  fontPool.cxx
  geomTextGlyph.cxx
  staticTextFont.cxx
  textAssembler.cxx
  textFont.cxx textGlyph.cxx
  textGraphic.cxx
  textNode.cxx
  textProperties.cxx
  textPropertiesManager.cxx
)

composite_sources(p3text P3TEXT_SOURCES)
add_component_library(p3text SYMBOL BUILDING_PANDA_TEXT
  ${P3TEXT_HEADERS} ${P3TEXT_SOURCES})
target_link_libraries(p3text p3parametrics
  PKG::HARFBUZZ)
if(HAVE_FREETYPE)
  target_link_libraries(p3text p3pnmtext)
endif()
target_interrogate(p3text ALL)

if(HAVE_HARFBUZZ)
  target_compile_definitions(p3text PRIVATE HAVE_HARFBUZZ)

  if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
    target_link_options(p3text PRIVATE "LINKER:--exclude-libs,libharfbuzz.a")
  endif()
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3text
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3TEXT_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
