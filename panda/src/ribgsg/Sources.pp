#define BUILD_DIRECTORY $[HAVE_RIB]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET ribgsg
  #define LOCAL_LIBS \
    pgraph gsgmisc display gobj

  #define SOURCES \
    config_ribgsg.cxx config_ribgsg.h ribGraphicsStateGuardian.I \
    ribGraphicsStateGuardian.cxx ribGraphicsStateGuardian.h \
    ribStuffTraverser.cxx ribStuffTraverser.h

  #define INSTALL_HEADERS \
    ribGraphicsStateGuardian.h

#end lib_target

