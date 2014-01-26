#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#define USE_PACKAGES rocket
#define BUILD_DIRECTORY $[HAVE_ROCKET]

#define BUILDING_DLL BUILDING_ROCKET

#begin lib_target
  #define TARGET p3rocket
  #define LOCAL_LIBS \
    p3display p3pgraph p3gobj p3linmath p3putil p3dgraph p3text

  #define COMBINED_SOURCES p3rocket_composite1.cxx

  #define SOURCES \
    config_rocket.h \
    rocketFileInterface.h \
    rocketInputHandler.h \
    rocketRegion.h \
    $[if $[HAVE_ROCKET_PYTHON],rocketRegion_ext.cxx rocketRegion_ext.h] \
    rocketRenderInterface.h \
    rocketSystemInterface.h

  #define INCLUDED_SOURCES \
    config_rocket.cxx \
    rocketFileInterface.cxx \
    rocketInputHandler.cxx \
    rocketRegion.cxx \
    rocketRenderInterface.cxx \
    rocketSystemInterface.cxx

  #define INSTALL_HEADERS \
    config_rocket.h rocketRegion.h

#if $[HAVE_ROCKET_PYTHON]
  #define IGATESCAN rocketInputHandler.h rocketInputHandler.cxx rocketRegion.h rocketRegion.cxx rocketRegion_ext.h
#endif

#end lib_target
