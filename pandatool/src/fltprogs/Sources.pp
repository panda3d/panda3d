#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET fltcopy
  #define LOCAL_LIBS cvscopy flt

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    fltCopy.cxx fltCopy.h

#end bin_target

#begin bin_target
  #define TARGET flt-trans
  #define LOCAL_LIBS \
    progbase flt
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    fltTrans.cxx fltTrans.h

#end bin_target

#begin bin_target
  #define TARGET flt-info
  #define LOCAL_LIBS \
    progbase flt
  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    fltInfo.cxx fltInfo.h

#end bin_target

#begin bin_target
  #define TARGET flt2egg
  #define LOCAL_LIBS flt fltegg eggbase progbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    fltToEgg.cxx fltToEgg.h

#end bin_target

#begin bin_target
  #define TARGET egg2flt
  #define LOCAL_LIBS flt eggbase progbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    linmath:c panda:m \
    express:c pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub

  #define SOURCES \
    eggToFlt.cxx eggToFlt.h

#end bin_target
