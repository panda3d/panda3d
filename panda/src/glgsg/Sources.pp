#define BUILD_DIRECTORY $[HAVE_GL]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_PACKAGES gl

#begin lib_target
  #define TARGET glgsg
  #define LOCAL_LIBS \
    glstuff gsgmisc gsgbase gobj display \
    putil linmath mathutil pnmimage
    
  #define SOURCES \
    config_glgsg.h config_glgsg.cxx \
    glgsg.h glgsg.cxx

  #define INSTALL_HEADERS \
    config_glgsg.h glgsg.h

#end lib_target

