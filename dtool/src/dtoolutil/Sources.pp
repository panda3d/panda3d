#begin lib_target
  #define TARGET dtoolutil
  #define LOCAL_LIBS dtoolbase
  #define UNIX_SYS_LIBS dl
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx  $[TARGET]_composite2.cxx
  
  #define SOURCES \
     executionEnvironment.I executionEnvironment.h filename.I  \
     filename.h load_dso.h dSearchPath.I dSearchPath.h pfstream.h  \
     vector_string.h gnu_getopt.c gnu_getopt.h gnu_getopt1.c  \
     pfstreamBuf.h vector_src.h 

  #define INCLUDED_SOURCES \
     executionEnvironment.cxx filename.cxx load_dso.cxx  \
     dSearchPath.cxx vector_string.cxx \
     pfstreamBuf.cxx pfstream.cxx 

  #define INSTALL_HEADERS \
    executionEnvironment.I executionEnvironment.h filename.I    \
    filename.h load_dso.h dSearchPath.I dSearchPath.h   \
    pfstream.h pfstream.I vector_string.h gnu_getopt.h \
    pfstreamBuf.h vector_src.cxx vector_src.h

#end lib_target
