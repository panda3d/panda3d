#define DIRECTORY_IF_RIB yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET ribgsg
  #define LOCAL_LIBS \
    gsgmisc display gobj sgattrib sgraph sgraphutil light

  #define SOURCES \
    config_ribgsg.cxx config_ribgsg.h ribGraphicsStateGuardian.I \
    ribGraphicsStateGuardian.cxx ribGraphicsStateGuardian.h \
    ribStuffTraverser.cxx ribStuffTraverser.h

  #define INSTALL_HEADERS \
    ribGraphicsStateGuardian.h

#end lib_target

