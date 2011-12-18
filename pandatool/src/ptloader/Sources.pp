#define USE_PACKAGES cg  // from gobj.

#begin lib_target
  #define TARGET p3ptloader
  #define BUILDING_DLL BUILDING_PTLOADER
  #define LOCAL_LIBS \
    p3fltegg p3flt p3lwoegg p3lwo p3dxfegg p3dxf p3vrmlegg p3vrml p3xfileegg p3xfile \
    p3objegg \
    p3converter p3pandatoolbase $[if $[HAVE_FCOLLADA],p3daeegg]
  #define OTHER_LIBS \
    p3egg2pg:c p3egg:c pandaegg:m \
    p3pstatclient:c p3mathutil:c p3linmath:c p3putil:c \
    p3gobj:c p3chan:c p3parametrics:c p3pgraph:c p3pgraphnodes:c \
    p3pnmimage:c p3grutil:c p3collide:c p3tform:c p3text:c \
    p3char:c p3dgraph:c p3display:c p3device:c p3cull:c \
    p3downloader:c p3pipeline:c \
    p3event:c p3gsgbase:c p3movies:c \
    $[if $[HAVE_FREETYPE],p3pnmtext:c] \
    $[if $[HAVE_NET],p3net:c] $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[HAVE_AUDIO],p3audio:c] \
    panda:m \
    p3pandabase:c p3express:c pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    config_ptloader.cxx config_ptloader.h \
    loaderFileTypePandatool.cxx loaderFileTypePandatool.h

  #define INSTALL_HEADERS \
    config_ptloader.h loaderFileTypePandatool.h

#end lib_target
