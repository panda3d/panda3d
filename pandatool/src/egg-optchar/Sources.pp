#define LOCAL_LIBS \
  eggcharbase converter eggbase progbase
#define OTHER_LIBS \
  egg:c pandaegg:m \
  pnmimagetypes:c pnmimage:c linmath:c putil:c panda:m \
  express:c pandaexpress:m \
  dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub
#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET egg-optchar-new

  #define SOURCES \
    config_egg_optchar.cxx config_egg_optchar.h \
    eggOptchar.cxx eggOptchar.h \
    eggOptcharUserData.I eggOptcharUserData.cxx eggOptcharUserData.h

#end bin_target
