#define OTHER_LIBS interrogatedb dconfig dtoolutil dtoolbase

#begin lib_target
  #define TARGET egg2sg
  #define LOCAL_LIBS \
    egg builder loader chan char switchnode cull

  #define SOURCES \
    animBundleMaker.cxx animBundleMaker.h characterMaker.cxx \
    characterMaker.h computedVerticesMaker.I computedVerticesMaker.cxx \
    computedVerticesMaker.h config_egg2sg.cxx config_egg2sg.h \
    deferredArcProperty.cxx deferredArcProperty.h \
    deferredArcTraverser.cxx deferredArcTraverser.h eggBinner.cxx \
    eggBinner.h eggLoader.cxx eggLoader.h load_egg_file.cxx \
    load_egg_file.h loaderFileTypeEgg.cxx loaderFileTypeEgg.h

  #define INSTALL_HEADERS \
    load_egg_file.h

#end lib_target

#begin test_bin_target
  #define TARGET test_loader
  #define LOCAL_LIBS \
    egg2sg

  #define SOURCES \
    test_loader.cxx

#end test_bin_target

