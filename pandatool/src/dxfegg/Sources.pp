#begin ss_lib_target
  #define TARGET p3dxfegg
  #define LOCAL_LIBS p3converter p3dxf p3pandatoolbase
  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3pipeline:c p3event:c p3pstatclient:c panda:m \
    p3pandabase:c p3pnmimage:c p3mathutil:c p3linmath:c p3putil:c p3express:c \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],p3net:c p3downloader:c]

  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    dxfToEggConverter.cxx dxfToEggConverter.h \
    dxfToEggLayer.cxx dxfToEggLayer.h

  #define INSTALL_HEADERS \
    dxfToEggConverter.h dxfToEggLayer.h

#end ss_lib_target
