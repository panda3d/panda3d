#begin bin_target
  #define TARGET vrml2egg
  #define LOCAL_LIBS pvrml vrmlegg eggbase progbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c pnmimagetypes:c pnmimage:c putil:c mathutil:c event:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    vrmlToEgg.cxx vrmlToEgg.h

#end bin_target

#begin bin_target
  #define TARGET vrml-trans
  #define LOCAL_LIBS \
    progbase pvrml
  #define OTHER_LIBS \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    vrmlTrans.cxx vrmlTrans.h

#end bin_target
