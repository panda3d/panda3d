#begin ss_lib_target
  #define TARGET cvscopy
  #define LOCAL_LIBS \
    progbase pandatoolbase

  #define OTHER_LIBS \
    express:c dtoolutil:c dtoolbase:c pnmimagetypes:c pnmimage:c putil:c \
    linmath:c panda:m prc:c dconfig:c dtool:m pystub

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 
    
  #define SOURCES \
    cvsCopy.h cvsSourceDirectory.h cvsSourceTree.h  

  #define INCLUDED_SOURCES \
    cvsCopy.cxx cvsSourceDirectory.cxx cvsSourceTree.cxx

  #define INSTALL_HEADERS \
    cvsCopy.h

#end ss_lib_target

#begin test_bin_target
  #define TARGET testcopy
  #define LOCAL_LIBS cvscopy

  #define OTHER_LIBS \
    prc:c dconfig:c dtool:m pystub

  #define SOURCES \
    testCopy.cxx testCopy.h

#end test_bin_target
