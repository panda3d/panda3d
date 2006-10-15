#define OTHER_LIBS \
  egg:c pandaegg:m \
  linmath:c pnmimagetypes:c pnmimage:c putil:c mathutil:c event:c \
  pipeline:c \
  panda:m \
  pandabase:c express:c pandaexpress:m \
  interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

#begin bin_target
  #define TARGET vrml2egg
  #define LOCAL_LIBS pvrml vrmlegg eggbase progbase

  #define SOURCES \
    vrmlToEgg.cxx vrmlToEgg.h

#end bin_target

#begin bin_target
  #define TARGET vrml-trans
  #define LOCAL_LIBS \
    progbase pvrml

  #define SOURCES \
    vrmlTrans.cxx vrmlTrans.h

#end bin_target
