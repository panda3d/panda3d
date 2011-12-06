#define BUILD_DIRECTORY $[HAVE_GLES]
#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
#define USE_PACKAGES gles
#begin lib_target
  #define TARGET p3glesgsg
  #define LOCAL_LIBS \
    p3glstuff p3gsgbase p3gobj p3display \
    p3putil p3linmath p3mathutil p3pnmimage
    
  #define SOURCES \
    config_glesgsg.h config_glesgsg.cxx \
    glesgsg.h glesgsg.cxx

  #define INSTALL_HEADERS \
    config_glesgsg.h glesgsg.h

#end lib_target

