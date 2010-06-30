#define BUILD_DIRECTORY $[HAVE_FREETYPE]

#define OTHER_LIBS \
    progbase pandatoolbase \
    dtoolutil:c dtool:m prc:c dtoolbase:c dtoolconfig:m \
    express:c pandaexpress:m \
    putil:c pipeline:c linmath:c pnmimage:c mathutil:c pnmtext:c \
    pnmimagetypes:c event:c panda:m \
    pystub

#define UNIX_SYS_LIBS dl

#begin bin_target
  #define USE_PACKAGES freetype

  #define TARGET indexify

  #define SOURCES \
    default_index_icons.cxx default_index_icons.h \
    default_font.cxx default_font.h \
    indexImage.cxx indexImage.h \
    indexParameters.cxx indexParameters.h \
    indexify.cxx indexify.h \
    photo.cxx photo.h \
    rollDirectory.cxx rollDirectory.h

#end bin_target

#begin bin_target
  #define USE_PACKAGES freetype

  #define TARGET font-samples

  #define SOURCES \
    default_font.cxx default_font.h \
    fontSamples.cxx fontSamples.h

#end bin_target
