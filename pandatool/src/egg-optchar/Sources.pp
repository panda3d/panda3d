#define LOCAL_LIBS \
  eggcharbase converter eggbase progbase
#define OTHER_LIBS \
  egg:c pandaegg:m \
  event:c pipeline:c pstatclient:c downloader:c net:c nativenet:c \
  pnmimagetypes:c pnmimage:c mathutil:c linmath:c putil:c \
  panda:m \
  pandabase:c express:c pandaexpress:m \
  interrogatedb:c dtoolutil:c dtoolbase:c prc:c dconfig:c dtoolconfig:m dtool:m pystub
#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET egg-optchar

  #define SOURCES \
    config_egg_optchar.cxx config_egg_optchar.h \
    eggOptchar.cxx eggOptchar.h \
    eggOptcharUserData.I eggOptcharUserData.cxx eggOptcharUserData.h \
    vertexMembership.I vertexMembership.cxx vertexMembership.h

#end bin_target
