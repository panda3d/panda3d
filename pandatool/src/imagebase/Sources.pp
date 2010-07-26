#begin ss_lib_target
  #define TARGET imagebase
  #define LOCAL_LIBS \
    progbase

  #define OTHER_LIBS \
    pipeline:c event:c pstatclient:c panda:m \
    pandabase:c pnmimage:c mathutil:c linmath:c putil:c express:c \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m \
    $[if $[WANT_NATIVE_NET],nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],net:c downloader:c]

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    imageReader.h imageWriter.I imageWriter.h \
    imageBase.h imageFilter.h
    
  #define INCLUDED_SOURCES \
    imageBase.cxx imageFilter.cxx \
    imageReader.cxx imageWriter.cxx

  #define INSTALL_HEADERS \
    imageBase.h imageFilter.h imageReader.h imageWriter.I imageWriter.h

#end ss_lib_target

