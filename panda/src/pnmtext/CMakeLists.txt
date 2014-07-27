if(HAVE_FREETYPE)
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
  add_library(p3pnmtext ${P3PNMTEXT_HEADERS} ${P3PNMTEXT_SOURCES})
  target_link_libraries(p3pnmtext p3pnmimage ${_FREETYPE_LIBRARIES})
  target_interrogate(p3pnmtext ALL)

  install(TARGETS p3pnmtext DESTINATION lib)
endif()
