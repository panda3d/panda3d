#define DIRECTORY_IF_RIB yes

#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

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

