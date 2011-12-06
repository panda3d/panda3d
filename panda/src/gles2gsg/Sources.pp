#define BUILD_DIRECTORY $[HAVE_GLES2]
#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
#define USE_PACKAGES gles2
#begin lib_target
  #define TARGET p3gles2gsg
  #define LOCAL_LIBS \
    p3glstuff p3gsgbase p3gobj p3display \
    p3putil p3linmath p3mathutil p3pnmimage
    
  #define SOURCES \
    config_gles2gsg.h config_gles2gsg.cxx \
    gles2gsg.h gles2gsg.cxx

  #define INSTALL_HEADERS \
    config_gles2gsg.h gles2gsg.h

#end lib_target

