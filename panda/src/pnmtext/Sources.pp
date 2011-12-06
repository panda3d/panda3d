#define BUILD_DIRECTORY $[HAVE_FREETYPE]
#define USE_PACKAGES freetype

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  #define USE_PACKAGES freetype

  #define TARGET p3pnmtext
  #define LOCAL_LIBS \
    p3pnmimage p3putil p3linmath p3pipeline
    
  #define SOURCES \
    config_pnmtext.cxx config_pnmtext.h \
    freetypeFace.cxx freetypeFace.h freetypeFace.I \
    freetypeFont.cxx freetypeFont.h freetypeFont.I \
    pnmTextGlyph.cxx pnmTextGlyph.h pnmTextGlyph.I \
    pnmTextMaker.cxx pnmTextMaker.h pnmTextMaker.I

  #define INSTALL_HEADERS \
    config_pnmtext.h \
    freetypeFace.h freetypeFace.I \
    freetypeFont.h freetypeFont.I \
    pnmTextGlyph.h pnmTextGlyph.I \
    pnmTextMaker.h pnmTextMaker.I

  #define IGATESCAN all

#end lib_target
