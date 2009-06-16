#define BUILD_DIRECTORY $[HAVE_GLES2]
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c
#define USE_PACKAGES gles2
#begin lib_target
  #define TARGET gles2gsg
  #define LOCAL_LIBS \
    glstuff gsgbase gobj display \
    putil linmath mathutil pnmimage
    
  #define SOURCES \
    config_gles2gsg.h config_gles2gsg.cxx \
    gles2ext_shadow.h \
    gles2gsg.h gles2gsg.cxx

  #define INSTALL_HEADERS \
    config_gles2gsg.h gles2gsg.h

#end lib_target

