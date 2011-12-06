#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#define LOCAL_LIBS p3display p3pgraph p3express p3putil p3pandabase

#define COMBINED_SOURCES p3collada_composite1.cxx

#define SOURCES \
  colladaBindMaterial.cxx \
  colladaBindMaterial.h \
  colladaInput.cxx \
  colladaInput.h colladaInput.I \
  colladaLoader.cxx \
  colladaLoader.h colladaLoader.I \
  colladaPrimitive.cxx \
  colladaPrimitive.h colladaPrimitive.I \
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
