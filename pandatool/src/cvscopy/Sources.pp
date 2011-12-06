#begin ss_lib_target
  #define TARGET p3cvscopy
  #define LOCAL_LIBS \
    p3progbase p3pandatoolbase

  #define OTHER_LIBS \
    p3pipeline:c p3event:c p3pstatclient:c panda:m \
    p3pandabase:c p3pnmimage:c p3mathutil:c p3linmath:c p3putil:c p3express:c \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],p3net:c p3downloader:c]

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
  #define LOCAL_LIBS p3cvscopy

  #define OTHER_LIBS \
    p3prc:c p3dconfig:c p3dtool:m p3pystub

  #define SOURCES \
    testCopy.cxx testCopy.h

#end test_bin_target
