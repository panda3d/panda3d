#begin lib_target
  #define TARGET dtoolutil
  #define LOCAL_LIBS dtoolbase
  #define UNIX_SYS_LIBS dl
  
  #define SOURCES \
    executionEnvironment.I executionEnvironment.cxx			\
    executionEnvironment.h filename.I filename.cxx filename.h		\
    load_dso.cxx load_dso.h dSearchPath.I dSearchPath.cxx		\
    dSearchPath.h pfstream.h vector_string.cxx vector_string.h	\
    gnu_getopt.c gnu_getopt.h gnu_getopt1.c

  #define INSTALL_HEADERS \
    executionEnvironment.I executionEnvironment.h filename.I	\
    filename.h load_dso.h dSearchPath.I dSearchPath.h	\
    pfstream.h vector_string.h gnu_getopt.h

#end lib_target
