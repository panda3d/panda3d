#define BUILD_DIRECTORY $[HAVE_FREETYPE]

#define USE_PACKAGES freetype

#define LOCAL_LIBS \
  eggbase progbase
#define OTHER_LIBS \
  pnmtext:c pnmimagetypes:c pnmimage:c \
  egg:c linmath:c putil:c express:c pandaegg:m panda:m pandaexpress:m \
  dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

#begin bin_target
  #define TARGET egg-mkfont
  
//  #define COMBINED_SOURCES $[TARGET]_composite1.cxx   
  
  #defer SOURCES \
    eggMakeFont.h \
    rangeDescription.h rangeDescription.I \
    rangeIterator.h rangeIterator.I \
    $[INCLUDED_SOURCES]

  #define INCLUDED_SOURCES \
    eggMakeFont.cxx \
    rangeDescription.cxx \
    rangeIterator.cxx

#end bin_target
