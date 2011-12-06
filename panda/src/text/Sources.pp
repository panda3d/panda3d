#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define USE_PACKAGES freetype zlib

  #define TARGET p3text
  #define LOCAL_LIBS \
    p3putil p3gobj p3pgraph p3linmath \
    p3pnmtext p3pnmimage p3gsgbase p3mathutil \
    p3parametrics
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
    config_text.h \
    default_font.h \
    dynamicTextFont.I dynamicTextFont.h \
    dynamicTextGlyph.I dynamicTextGlyph.h \
    dynamicTextPage.I dynamicTextPage.h \
    fontPool.I fontPool.h \
    geomTextGlyph.I geomTextGlyph.h \
    staticTextFont.I staticTextFont.h \
    textAssembler.I textAssembler.h \
    textFont.I textFont.h \
    textGlyph.I textGlyph.h \
    textGraphic.I textGraphic.h \
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
    staticTextFont.cxx \
    textAssembler.cxx \
    textFont.cxx textGlyph.cxx \
    textGraphic.cxx \
    textNode.cxx \
    textProperties.cxx \
    textPropertiesManager.cxx \
    cmss12.bam_src.c cmss12.bam.pz_src.c persans.ttf_src.c

  #define INSTALL_HEADERS \
    config_text.h \
    dynamicTextFont.I dynamicTextFont.h \
    dynamicTextGlyph.I dynamicTextGlyph.h \
    dynamicTextPage.I dynamicTextPage.h \
    fontPool.I fontPool.h \
    geomTextGlyph.I geomTextGlyph.h \
    staticTextFont.I staticTextFont.h \
    textAssembler.I textAssembler.h \
    textFont.I textFont.h \
    textGlyph.I textGlyph.h \
    textGraphic.I textGraphic.h \
    textNode.I textNode.h \
    textProperties.I textProperties.h \
    textPropertiesManager.I textPropertiesManager.h


  #define IGATESCAN all

#end lib_target
