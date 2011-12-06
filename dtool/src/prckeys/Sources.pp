#define LOCAL_LIBS p3dtoolutil p3dtoolbase p3prc p3dconfig p3interrogatedb p3pystub

#begin bin_target
  #define BUILD_TARGET $[HAVE_OPENSSL]
  #define USE_PACKAGES openssl

  #define TARGET make-prc-key

  #define SOURCES \
    makePrcKey.cxx

  #define INSTALL_HEADERS \
    signPrcFile_src.cxx

#end bin_target

// #begin bin_target
//   #define BUILD_TARGET $[HAVE_OPENSSL]
//   #define USE_PACKAGES openssl
// 
//   #define TARGET panda-sign1
// 
//   #define SOURCES \
//     panda_sign1.cxx
// 
// #end bin_target
