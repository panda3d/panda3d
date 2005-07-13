#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET fltcopy
  #define LOCAL_LIBS cvscopy flt

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c pnmimagetypes:c pnmimage:c event:c \
    putil:c mathutil:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    fltCopy.cxx fltCopy.h

#end bin_target

#begin bin_target
  #define TARGET flt-trans
  #define LOCAL_LIBS \
    progbase flt
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c pnmimagetypes:c pnmimage:c putil:c event:c mathutil:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    fltTrans.cxx fltTrans.h

#end bin_target

#begin bin_target
  #define TARGET flt-info
  #define LOCAL_LIBS \
    progbase flt
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    event:c linmath:c mathutil:c \
    pnmimagetypes:c pnmimage:c putil:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    fltInfo.cxx fltInfo.h

#end bin_target

#begin bin_target
  #define TARGET flt2egg
  #define LOCAL_LIBS flt fltegg eggbase progbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c pnmimagetypes:c pnmimage:c putil:c mathutil:c event:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    fltToEgg.cxx fltToEgg.h

#end bin_target

#begin bin_target
  #define TARGET egg2flt
  #define LOCAL_LIBS flt eggbase progbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    putil:c event:c linmath:c pnmimagetypes:c pnmimage:c mathutil:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    eggToFlt.cxx eggToFlt.h

#end bin_target
