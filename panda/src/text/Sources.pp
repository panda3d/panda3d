#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define USE_PACKAGES freetype zlib

  #define TARGET text
  #define LOCAL_LIBS \
    putil gobj pgraph linmath \
    pnmtext pnmimage gsgbase mathutil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
    config_text.h \
    default_font.h \
    dynamicTextFont.I dynamicTextFont.h \
    dynamicTextGlyph.I dynamicTextGlyph.h \
    dynamicTextPage.I dynamicTextPage.h \
    fontPool.I fontPool.h \
    geomTextGlyph.I geomTextGlyph.h \
    qpgeomTextGlyph.I qpgeomTextGlyph.h \
    staticTextFont.I staticTextFont.h \
    textAssembler.I textAssembler.h \
    textFont.I textFont.h \
    textGlyph.I textGlyph.h \
    textNode.I textNode.h \
    textProperties.I textProperties.h \
    textPropertiesManager.I textPropertiesManager.h

  #define INCLUDED_SOURCES \
    config_text.cxx \
    default_font.cxx \
    dynamicTextFont.cxx \
    dynamicTextGlyph.cxx \
    dynamicTextPage.cxx \
    fontPool.cxx \
    geomTextGlyph.cxx \
    qpgeomTextGlyph.cxx \
    staticTextFont.cxx \
    textAssembler.cxx \
    textFont.cxx textGlyph.cxx \
    textNode.cxx \
    textProperties.cxx \
    textPropertiesManager.cxx

  #define INSTALL_HEADERS \
    config_text.h \
    dynamicTextFont.I dynamicTextFont.h \
    dynamicTextGlyph.I dynamicTextGlyph.h \
    dynamicTextPage.I dynamicTextPage.h \
    fontPool.I fontPool.h \
    geomTextGlyph.I geomTextGlyph.h \
    qpgeomTextGlyph.I qpgeomTextGlyph.h \
    staticTextFont.I staticTextFont.h \
    textAssembler.I textAssembler.h \
    textFont.I textFont.h \
    textGlyph.I textGlyph.h \
    textNode.I textNode.h \
    textProperties.I textProperties.h \
    textPropertiesManager.I textPropertiesManager.h


  #define IGATESCAN all

#end lib_target
