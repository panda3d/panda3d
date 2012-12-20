#define BUILD_DIRECTORY $[HAVE_EGG]

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#define USE_PACKAGES nurbspp

#begin lib_target
  #define TARGET p3egg2pg
  #define LOCAL_LIBS \
    p3parametrics p3collide p3egg p3pgraph p3chan p3char

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
    animBundleMaker.h \
    characterMaker.h \
    config_egg2pg.h \
    deferredNodeProperty.h \
    eggBinner.h \
    eggLoader.h eggLoader.I \
    eggRenderState.h eggRenderState.I \
    eggSaver.h eggSaver.I \
    egg_parametrics.h \
    load_egg_file.h \
    save_egg_file.h \
    loaderFileTypeEgg.h

  #define INCLUDED_SOURCES \
    animBundleMaker.cxx \
    characterMaker.cxx \
    config_egg2pg.cxx \
    deferredNodeProperty.cxx \
    eggBinner.cxx \
    eggLoader.cxx \
    eggRenderState.cxx \
    eggSaver.cxx \
    egg_parametrics.cxx \
    load_egg_file.cxx \
    save_egg_file.cxx \
    loaderFileTypeEgg.cxx

  #if $[DONT_COMBINE_PGRAPH]    
    #define SOURCES $[SOURCES] $[INCLUDED_SOURCES]
    #define INCLUDED_SOURCES
    #define COMBINED_SOURCES
  #endif

  #define INSTALL_HEADERS \
    egg_parametrics.h load_egg_file.h save_egg_file.h config_egg2pg.h

  #define IGATESCAN load_egg_file.h save_egg_file.h

#end lib_target
