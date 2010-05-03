#define BUILD_DIRECTORY $[HAVE_FREETYPE]
#define USE_PACKAGES freetype

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  #define USE_PACKAGES freetype

  #define TARGET pnmtext
  #define LOCAL_LIBS \
    pnmimage putil linmath pipeline
    
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
