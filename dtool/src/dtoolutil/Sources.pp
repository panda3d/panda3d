#begin lib_target
  #define TARGET dtoolutil
  #define LOCAL_LIBS dtoolbase
  #if $[ne $[PLATFORM], FreeBSD]
    #define UNIX_SYS_LIBS dl
  #endif
  #define WIN_SYS_LIBS shell32.lib
  #define OSX_SYS_FRAMEWORKS Foundation $[if $[not $[BUILD_IPHONE]],AppKit]
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  $[TARGET]_composite2.cxx
  
  #define SOURCES \
    checkPandaVersion.h \
    config_dtoolutil.h \
    executionEnvironment.I executionEnvironment.h filename.I  \
    filename.h \
    $[if $[IS_OSX],filename_assist.mm filename_assist.h,] \
    load_dso.h dSearchPath.I dSearchPath.h \
    pandaFileStream.h pandaFileStream.I \
    pandaFileStreamBuf.h \
    pandaSystem.h pandaVersion.h \
    panda_getopt.h panda_getopt_long.h panda_getopt_impl.h \
    pfstream.h pfstream.I pfstreamBuf.h \
    vector_string.h \
    vector_src.h 

  #define INCLUDED_SOURCES \
    checkPandaVersion.cxx \
    config_dtoolutil.cxx \
    executionEnvironment.cxx filename.cxx load_dso.cxx  \
    dSearchPath.cxx \
    pandaFileStream.cxx pandaFileStreamBuf.cxx \
    pandaSystem.cxx \
    panda_getopt_impl.cxx \
    pfstreamBuf.cxx pfstream.cxx \
    vector_string.cxx

  #define INSTALL_HEADERS \
    checkPandaVersion.h \
    config_dtoolutil.h \
    executionEnvironment.I executionEnvironment.h filename.I    \
    filename.h load_dso.h dSearchPath.I dSearchPath.h   \
    pandaFileStream.h pandaFileStream.I \
    pandaFileStreamBuf.h \
    pandaSystem.h pandaVersion.h \
    panda_getopt.h panda_getopt_long.h panda_getopt_impl.h \
    pfstream.h pfstream.I pfstreamBuf.h \
    vector_string.h \
    vector_src.cxx vector_src.h
#end lib_target

#begin test_bin_target
  #define TARGET test_pfstream
  #define LOCAL_LIBS dtoolbase dtoolutil

  #define SOURCES test_pfstream.cxx
#end test_bin_target

#begin test_bin_target
  #define TARGET test_touch
  #define LOCAL_LIBS dtoolbase dtoolutil

  #define SOURCES test_touch.cxx
#end test_bin_target

#include $[THISDIRPREFIX]pandaVersion.h.pp
