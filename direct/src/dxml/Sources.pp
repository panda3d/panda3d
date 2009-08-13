#begin lib_target
  #define BUILD_TARGET $[HAVE_TINYXML]
  #define USE_PACKAGES tinyxml

  #define TARGET dxml
  #define LOCAL_LIBS directbase
  #define OTHER_LIBS \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c prc:c dtool:m

  #define SOURCES \
    config_dxml.h config_dxml.cxx

  #define IGATESCAN all
#end lib_target
