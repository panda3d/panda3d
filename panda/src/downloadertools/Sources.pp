#define OTHER_LIBS interrogatedb:c prc:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m pystub prc:c
#define LOCAL_LIBS downloader express
#define USE_PACKAGES openssl zlib

#begin bin_target
  #define TARGET apply_patch
  #define BUILD_TARGET $[HAVE_OPENSSL]

  #define SOURCES \
    apply_patch.cxx

  #define INSTALL_HEADERS \

#end bin_target

#begin bin_target
  #define TARGET build_patch
  #define BUILD_TARGET $[HAVE_OPENSSL]

  #define SOURCES \
    build_patch.cxx

#end bin_target

#begin bin_target
  #define TARGET show_ddb
  #define BUILD_TARGET $[HAVE_OPENSSL]

  #define SOURCES \
    show_ddb.cxx

#end bin_target

#begin bin_target
  #define TARGET check_adler
  #define BUILD_TARGET $[HAVE_ZLIB]

  #define SOURCES \
    check_adler.cxx

#end bin_target

#begin bin_target
  #define TARGET check_crc
  #define BUILD_TARGET $[HAVE_ZLIB]

  #define SOURCES \
    check_crc.cxx

#end bin_target

#begin bin_target
  #define TARGET check_md5
  #define BUILD_TARGET $[HAVE_OPENSSL]

  #define SOURCES \
    check_md5.cxx

#end bin_target

#begin bin_target
  #define TARGET multify

  #define SOURCES \
    multify.cxx

#end bin_target

#begin bin_target
  #define TARGET pzip
  #define BUILD_TARGET $[HAVE_ZLIB]

  #define SOURCES \
    pzip.cxx

#end bin_target

#begin bin_target
  #define TARGET punzip
  #define BUILD_TARGET $[HAVE_ZLIB]

  #define SOURCES \
    punzip.cxx

#end bin_target

#begin bin_target
  #define TARGET pencrypt
  #define BUILD_TARGET $[HAVE_OPENSSL]

  #define SOURCES \
    pencrypt.cxx

#end bin_target

#begin bin_target
  #define TARGET pdecrypt
  #define BUILD_TARGET $[HAVE_OPENSSL]

  #define SOURCES \
    pdecrypt.cxx

#end bin_target
