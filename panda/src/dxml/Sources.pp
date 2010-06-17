#begin static_lib_target
  #define TARGET p3tinyxml

  #define COMBINED_SOURCES tinyxml_composite1.cxx

  #define SOURCES \
     tinyxml.h

  #define INCLUDED_SOURCES  \
     tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp

  #define INSTALL_HEADERS \
    tinyxml.h

  #define EXTRA_CDEFS TIXML_USE_STL
  #define C++FLAGS $[CFLAGS_SHARED]

#end static_lib_target

#if $[P3D_PLUGIN_MT]
#begin static_lib_target
//
// libp3tinyxml_mt.lib, the same as above, with /MT compilation.  This
// is needed when building the /MT plugin.
//
  #define TARGET p3tinyxml_mt
  #define LINK_FORCE_STATIC_RELEASE_C_RUNTIME 1

  #define COMBINED_SOURCES tinyxml_composite1.cxx

  #define SOURCES \
     tinyxml.h

  #define INCLUDED_SOURCES  \
     tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp

  #define INSTALL_HEADERS \
    tinyxml.h

  #define EXTRA_CDEFS TIXML_USE_STL
  #define C++FLAGS $[CFLAGS_SHARED]

#end static_lib_target
#endif  // $[P3D_PLUGIN_MT]

#begin lib_target
  #define TARGET dxml

  #define LOCAL_LIBS pandabase
  #define OTHER_LIBS \
    interrogatedb:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c prc:c dtool:m

  #define COMBINED_SOURCES dxml_composite1.cxx

  #define SOURCES \
    config_dxml.h tinyxml.h

  #define INCLUDED_SOURCES \
    config_dxml.cxx \
    tinyxml.cpp tinyxmlparser.cpp tinyxmlerror.cpp

  // It's important not to include tinyxml.h on the IGATESCAN list,
  // because that file has to be bracketed by BEGIN_PUBLISH
  // .. END_PUBLISH (which is handled by config_dxml.cxx).
  #define IGATESCAN config_dxml.h $[INCLUDED_SOURCES]
#end lib_target
