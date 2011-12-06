#define USE_PACKAGES zlib

#begin ss_lib_target
  #define TARGET p3progbase
  #define LOCAL_LIBS \
    p3pandatoolbase
  #define OTHER_LIBS \
    p3pipeline:c p3event:c p3pstatclient:c panda:m \
    p3pandabase:c p3pnmimage:c p3mathutil:c p3linmath:c p3putil:c p3express:c \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],p3net:c p3downloader:c]
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    programBase.I programBase.h \
    withOutputFile.I withOutputFile.h \
    wordWrapStream.h wordWrapStreamBuf.I \
    wordWrapStreamBuf.h
    
  #define INCLUDED_SOURCES \
    programBase.cxx withOutputFile.cxx wordWrapStream.cxx \
    wordWrapStreamBuf.cxx   

  #define INSTALL_HEADERS \
    programBase.I programBase.h \
    withOutputFile.I withOutputFile.h \
    wordWrapStream.h wordWrapStreamBuf.I \
    wordWrapStreamBuf.h

#end ss_lib_target

#begin test_bin_target
  #define TARGET test_prog
  #define LOCAL_LIBS \
    p3progbase
  #define OTHER_LIBS p3pystub

  #define SOURCES \
    test_prog.cxx

#end test_bin_target

