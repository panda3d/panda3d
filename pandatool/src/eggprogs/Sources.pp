#define LOCAL_LIBS \
  converter eggbase progbase
#define OTHER_LIBS \
  egg:c event:c pandaegg:m \
  pnmimagetypes:c pnmimage:c mathutil:c linmath:c putil:c panda:m \
  express:c pandaexpress:m \
  dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub
#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET egg-trans

  #define SOURCES \
    eggTrans.cxx eggTrans.h

#end bin_target

#begin bin_target
  #define TARGET egg-crop

  #define SOURCES \
    eggCrop.cxx eggCrop.h

#end bin_target

#begin bin_target
  #define TARGET egg-texture-cards

  #define SOURCES \
    eggTextureCards.cxx eggTextureCards.h

#end bin_target

#begin bin_target
  #define TARGET egg-make-tube

  #define SOURCES \
    eggMakeTube.cxx eggMakeTube.h

#end bin_target

#begin bin_target
  #define LOCAL_LIBS eggcharbase $[LOCAL_LIBS]
  #define TARGET egg-topstrip

  #define SOURCES \
    eggTopstrip.cxx eggTopstrip.h

#end bin_target

#begin bin_target
  #define TARGET egg2c

  #define SOURCES \
    eggToC.cxx eggToC.h

#end bin_target
