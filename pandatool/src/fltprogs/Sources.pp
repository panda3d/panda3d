#define UNIX_SYS_LIBS m

#define OTHER_LIBS \
  p3egg:c pandaegg:m \
  p3linmath:c p3pnmimagetypes:c p3pnmimage:c p3event:c \
  p3putil:c p3mathutil:c p3pipeline:c p3pstatclient:c p3downloader:c p3net:c p3nativenet:c \
  panda:m \
  p3pandabase:c p3express:c pandaexpress:m \
  p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m p3pystub

#begin bin_target
  #define TARGET fltcopy
  #define LOCAL_LIBS p3cvscopy p3flt

  #define SOURCES \
    fltCopy.cxx fltCopy.h

#end bin_target

#begin bin_target
  #define TARGET flt-trans
  #define LOCAL_LIBS \
    p3progbase p3flt

  #define SOURCES \
    fltTrans.cxx fltTrans.h

#end bin_target

#begin bin_target
  #define TARGET flt-info
  #define LOCAL_LIBS \
    p3progbase p3flt

  #define SOURCES \
    fltInfo.cxx fltInfo.h

#end bin_target

#begin bin_target
  #define TARGET flt2egg
  #define LOCAL_LIBS p3flt p3fltegg p3eggbase p3progbase

  #define SOURCES \
    fltToEgg.cxx fltToEgg.h

#end bin_target

#begin bin_target
  #define TARGET egg2flt
  #define LOCAL_LIBS p3flt p3eggbase p3progbase

  #define SOURCES \
    eggToFlt.cxx eggToFlt.h

#end bin_target
