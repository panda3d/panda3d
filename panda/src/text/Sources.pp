#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define USE_FREETYPE yes

  #define TARGET text
  #define LOCAL_LIBS \
    putil gobj pgraph linmath \
    pnmimage gsgbase mathutil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    config_text.h \
    dynamicTextFont.I dynamicTextFont.h \
    dynamicTextGlyph.I dynamicTextGlyph.h \
    dynamicTextPage.I dynamicTextPage.h \
    geomTextGlyph.I geomTextGlyph.h \
    staticTextFont.I staticTextFont.h \
    stringDecoder.I stringDecoder.h \
    textFont.I textFont.h \
    textGlyph.I textGlyph.h \
    qptextNode.I qptextNode.h qptextNode.cxx

  #define INCLUDED_SOURCES \
    config_text.cxx \
    dynamicTextFont.cxx \
    dynamicTextGlyph.cxx \
    dynamicTextPage.cxx \
    geomTextGlyph.cxx \
    stringDecoder.cxx \
    staticTextFont.cxx \
    textFont.cxx textGlyph.cxx

  #define INSTALL_HEADERS \
    config_text.h \
    dynamicTextFont.I dynamicTextFont.h \
    dynamicTextGlyph.I dynamicTextGlyph.h \
    dynamicTextPage.I dynamicTextPage.h \
    geomTextGlyph.I geomTextGlyph.h \
    staticTextFont.I staticTextFont.h \
    stringDecoder.I stringDecoder.h \
    textFont.I textFont.h \
    textGlyph.I textGlyph.h \
    qptextNode.I qptextNode.h

  #define IGATESCAN all

#end lib_target

