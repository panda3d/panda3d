#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#define USE_NURBSPP yes

#begin lib_target
  #define TARGET egg2sg
  #define LOCAL_LIBS \
    parametrics cull collide egg2pg egg builder loader chan char switchnode

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 

  #define SOURCES \
     config_egg2sg.h  \
     deferredArcProperty.h deferredArcTraverser.h  \
     eggLoader.h load_egg_file.h loaderFileTypeEgg.h 

  #define INCLUDED_SOURCES \
     config_egg2sg.cxx deferredArcProperty.cxx  \
     deferredArcTraverser.cxx eggLoader.cxx  \
     load_egg_file.cxx loaderFileTypeEgg.cxx 

  #define INSTALL_HEADERS \
    load_egg_file.h config_egg2sg.h

#end lib_target

#begin test_bin_target
  #define TARGET test_loader
  #define LOCAL_LIBS \
    egg2sg

  #define SOURCES \
    test_loader.cxx

#end test_bin_target

