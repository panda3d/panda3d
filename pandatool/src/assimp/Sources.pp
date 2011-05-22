#begin lib_target
  #define BUILD_TARGET $[HAVE_ASSIMP]

  #define TARGET p3assimp
  #define BUILDING_DLL BUILDING_ASSIMP
  #define LOCAL_LIBS \
    pandatoolbase
  #define USE_PACKAGES assimp

  #define OTHER_LIBS \
    egg2pg:c egg:c pandaegg:m \
    pstatclient:c mathutil:c linmath:c putil:c \
    gobj:c chan:c parametrics:c pgraph:c pgraphnodes:c \
    pnmimage:c grutil:c collide:c tform:c text:c \
    char:c dgraph:c display:c device:c cull:c \
    downloader:c pipeline:c \
    event:c gsgbase:c lerp:c movies:c \
    panda:m \
    pandabase:c express:c pandaexpress:m \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m \
    assimp:c

  #define SOURCES \
    assimpLoader.cxx assimpLoader.h \
    config_assimp.cxx config_assimp.h \
    loaderFileTypeAssimp.cxx loaderFileTypeAssimp.h \
    pandaIOStream.cxx pandaIOStream.h \
    pandaIOSystem.cxx pandaIOSystem.h \
    pandaLogger.cxx pandaLogger.h

  #define INSTALL_HEADERS \
    assimpLoader.h \
    config_assimp.h loaderFileTypeAssimp.h

#end lib_target
