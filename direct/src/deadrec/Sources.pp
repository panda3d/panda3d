#begin lib_target
  #define TARGET p3deadrec
  #define LOCAL_LIBS \
    p3directbase
  #define OTHER_LIBS \
    p3express:c p3linmath:c \
    p3interrogatedb:c p3dconfig:c \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    p3prc:c p3pandabase:c p3putil:c \
    p3pipeline:c

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    config_deadrec.h \
    smoothMover.h smoothMover.I
  
  #define INCLUDED_SOURCES \  
    config_deadrec.cxx \
    smoothMover.cxx

  #define INSTALL_HEADERS \
    config_deadrec.h \
    smoothMover.h smoothMover.I

  #define IGATESCAN \
    all
#end lib_target
