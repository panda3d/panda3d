#define OTHER_LIBS interrogatedb:c dconfig:c dtoolutil:c dtoolbase:c dtool:m pystub
#define LOCAL_LIBS downloader express event ipc

#begin bin_target
  #define TARGET apply_patch

  #define SOURCES \
    apply_patch.cxx

  #define INSTALL_HEADERS \

#end bin_target

#begin bin_target
  #define TARGET build_patch

  #define SOURCES \
    build_patch.cxx

#end bin_target

#begin bin_target
  #define TARGET check_adler

  #define SOURCES \
    check_adler.cxx

#end bin_target

#begin bin_target
  #define TARGET check_crc

  #define SOURCES \
    check_crc.cxx

#end bin_target

#begin bin_target
  #define TARGET multify

  #define SOURCES \
    multify.cxx

#end bin_target

#begin bin_target
  #define TARGET pcompress

  #define SOURCES \
    pcompress.cxx

#end bin_target

#begin bin_target
  #define TARGET pdecompress

  #define SOURCES \
    pdecompress.cxx

#end bin_target

