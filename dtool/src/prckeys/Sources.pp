#define LOCAL_LIBS pystub dtoolutil dtoolbase prc dconfig interrogatedb

#begin bin_target
  #define BUILD_TARGET $[HAVE_SSL]
  #define USE_PACKAGES ssl

  #define TARGET make-prc-key

  #define SOURCES \
    makePrcKey.cxx

  #define INSTALL_HEADERS \
    signPrcFile_src.cxx

#end bin_target

// #begin bin_target
//   #define BUILD_TARGET $[HAVE_SSL]
//   #define USE_PACKAGES ssl
// 
//   #define TARGET panda-sign1
// 
//   #define SOURCES \
//     panda_sign1.cxx
// 
// #end bin_target
