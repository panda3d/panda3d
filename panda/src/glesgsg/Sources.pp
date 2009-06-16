#define BUILD_DIRECTORY $[HAVE_GLES]
#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c
#define USE_PACKAGES gles
#begin lib_target
  #define TARGET glesgsg
  #define LOCAL_LIBS \
    glstuff gsgbase gobj display \
    putil linmath mathutil pnmimage
    
  #define SOURCES \
    config_glesgsg.h config_glesgsg.cxx \
    glesgsg.h glesgsg.cxx

  #define INSTALL_HEADERS \
    config_glesgsg.h glesgsg.h

#end lib_target

