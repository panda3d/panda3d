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
target_link_libraries(p3text p3parametrics)
if(HAVE_FREETYPE)
  target_link_libraries(p3text p3pnmtext)
endif()
target_interrogate(p3text ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3text DESTINATION lib RUNTIME DESTINATION bin)
endif()
install(FILES ${P3TEXT_HEADERS} DESTINATION include/panda3d)
