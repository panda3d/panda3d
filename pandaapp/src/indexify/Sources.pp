#define BUILD_DIRECTORY $[HAVE_FREETYPE]

#begin bin_target
  #define USE_PACKAGES freetype

  #define TARGET indexify
  #define OTHER_LIBS \
    progbase pandatoolbase \
    pnmimage:c pnmimagetypes:c panda:m pandaexpress:m \
    dtool:m dtoolconfig:m \
    pystub

  #define SOURCES \
    default_index_icons.cxx default_index_icons.h \
    default_font.cxx default_font.h \
    indexImage.cxx indexImage.h \
    indexParameters.cxx indexParameters.h \
    indexify.cxx indexify.h \
    photo.cxx photo.h \
    rollDirectory.cxx rollDirectory.h \
    pnmTextGlyph.cxx pnmTextGlyph.h \
    pnmTextMaker.cxx pnmTextMaker.h

#end bin_target

#begin bin_target
  #define USE_PACKAGES freetype

  #define TARGET font-samples
  #define OTHER_LIBS \
    progbase pandatoolbase \
    pnmimage:c pnmimagetypes:c panda:m pandaexpress:m \
    dtool:m dtoolconfig:m \
    pystub

  #define SOURCES \
    default_font.cxx default_font.h \
    fontSamples.cxx fontSamples.h \
    pnmTextGlyph.cxx pnmTextGlyph.h \
    pnmTextMaker.cxx pnmTextMaker.h

#end bin_target
