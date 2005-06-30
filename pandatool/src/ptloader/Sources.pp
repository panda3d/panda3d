#begin lib_target
  #define TARGET ptloader
  #define BUILDING_DLL BUILDING_PTLOADER
  #define LOCAL_LIBS \
    fltegg flt lwoegg lwo dxfegg dxf vrmlegg pvrml xfileegg xfile \
    converter pandatoolbase
  #define OTHER_LIBS \
    egg2pg:c egg:c pandaegg:m \
    mathutil:c linmath:c putil:c \
    gobj:c chan:c parametrics:c pgraph:c \
    pnmimage:c grutil:c collide:c tform:c text:c \
    char:c dgraph:c display:c \
    downloader:c \
    event:c gsgbase:c lerp:c panda:m \
    express:c pandaexpress:m \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    config_ptloader.cxx config_ptloader.h \
    loaderFileTypePandatool.cxx loaderFileTypePandatool.h

  #define INSTALL_HEADERS \
    config_ptloader.h loaderFileTypePandatool.h

#end lib_target
