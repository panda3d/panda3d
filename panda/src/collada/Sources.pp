#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#define LOCAL_LIBS display pgraph express putil pandabase

#define COMBINED_SOURCES collada_composite1.cxx

#define SOURCES \
  colladaLoader.cxx \
  colladaLoader.h colladaLoader.I \
  config_collada.h \
  load_collada_file.h \
  loaderFileTypeDae.h

#define INCLUDED_SOURCES \
  config_collada.cxx \
  load_collada_file.cxx \
  loaderFileTypeDae.cxx

#define INSTALL_HEADERS \
  config_collada.h \
  load_collada_file.h \
  loaderFileTypeDae.h

#begin lib_target
  #define BUILD_TARGET $[HAVE_COLLADA14DOM]
  #define USE_PACKAGES collada14dom
  #define EXTRA_CDEFS PANDA_COLLADA_VERSION=14
  #define TARGET p3collada14
#end lib_target

#begin lib_target
  #define BUILD_TARGET $[HAVE_COLLADA15DOM]
  #define USE_PACKAGES collada15dom
  #define EXTRA_CDEFS PANDA_COLLADA_VERSION=15
  #define TARGET p3collada15
#end lib_target
