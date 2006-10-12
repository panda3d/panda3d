#begin ss_lib_target
  #define TARGET imagebase
  #define LOCAL_LIBS \
    progbase
  #define OTHER_LIBS \
    linmath:c putil:c pnmimage:c pipeline:c event:c \
    panda:m \
    pandabase:c express:c pandaexpress:m \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m

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

