#begin lib_target
  #define BUILD_TARGET $[HAVE_ASSIMP]

  #define TARGET p3assimp
  #define BUILDING_DLL BUILDING_ASSIMP
  #define LOCAL_LIBS \
    p3pandatoolbase
  #define USE_PACKAGES assimp

  #define OTHER_LIBS \
    p3egg2pg:c p3egg:c pandaegg:m \
    p3pstatclient:c p3mathutil:c p3linmath:c p3putil:c \
    p3gobj:c p3chan:c p3parametrics:c p3pgraph:c p3pgraphnodes:c \
    p3pnmimage:c p3grutil:c p3collide:c p3tform:c p3text:c \
    p3char:c p3dgraph:c p3display:c p3device:c p3cull:c \
    p3downloader:c p3pipeline:c \
    p3event:c p3gsgbase:c p3movies:c \
    panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m

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
