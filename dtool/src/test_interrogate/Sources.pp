#define LOCAL_LIBS pystub interrogatedb dconfig dtoolutil dtoolbase
#define USE_PACKAGES ssl

#begin bin_target
  #define TARGET test_interrogate

  #define SOURCES \
    test_interrogate.cxx
#end bin_target

#begin test_bin_target
  #define TARGET test_lib
  #define SOURCES test_lib.cxx test_lib.h
#end test_bin_target
