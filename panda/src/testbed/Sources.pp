#define OTHER_LIBS p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3pystub

#define USE_PACKAGES cg freetype

#define LOCAL_LIBS \
    p3framework p3putil p3collide p3pgraph p3chan p3text \
    p3pnmimage p3pnmimagetypes p3pnmtext p3event p3gobj p3display \
    p3mathutil p3putil p3express p3dgraph p3device p3tform \
    p3linmath p3pstatclient panda

#begin bin_target
  #define TARGET pview

  #define SOURCES \
    pview.cxx
#end bin_target

#begin test_bin_target
  #define TARGET pgrid

  #define SOURCES \
    pgrid.cxx
  #define UNIX_SYS_LIBS m
#end test_bin_target

#begin test_bin_target
  #define TARGET test_lod
  #define SOURCES test_lod.cxx
#end test_bin_target

#begin test_bin_target
  #define TARGET test_texmem
  #define SOURCES test_texmem.cxx
#end test_bin_target

#begin test_bin_target
  #define TARGET test_map
  #define SOURCES test_map.cxx
#end test_bin_target
