#begin bin_target
  #define TARGET softcvs
  #define LOCAL_LIBS p3progbase
  #define USE_PACKAGES openssl

  #define OTHER_LIBS \ 
    p3egg:c pandaegg:m \
    p3pipeline:c p3event:c p3pstatclient:c panda:m \
    p3pandabase:c p3pnmimage:c p3mathutil:c p3linmath:c p3putil:c p3express:c \
    pandaexpress:m \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],p3net:c p3downloader:c] \
    p3pystub

  #define SOURCES \
    softCVS.cxx softCVS.h softFilename.cxx softFilename.h

#end bin_target
