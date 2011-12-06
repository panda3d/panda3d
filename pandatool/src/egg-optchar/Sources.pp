#define LOCAL_LIBS \
  p3eggcharbase p3converter p3eggbase p3progbase
#define OTHER_LIBS \
  p3egg:c pandaegg:m \
  p3event:c p3pipeline:c p3pstatclient:c p3downloader:c p3net:c p3nativenet:c \
  p3pnmimagetypes:c p3pnmimage:c p3mathutil:c p3linmath:c p3putil:c \
  panda:m \
  p3pandabase:c p3express:c pandaexpress:m \
  p3interrogatedb:c p3dtoolutil:c p3dtoolbase:c p3prc:c p3dconfig:c p3dtoolconfig:m p3dtool:m p3pystub
#define UNIX_SYS_LIBS m

#begin bin_target
  #define TARGET egg-optchar

  #define SOURCES \
    config_egg_optchar.cxx config_egg_optchar.h \
    eggOptchar.cxx eggOptchar.h \
    eggOptcharUserData.I eggOptcharUserData.cxx eggOptcharUserData.h \
    vertexMembership.I vertexMembership.cxx vertexMembership.h

#end bin_target
