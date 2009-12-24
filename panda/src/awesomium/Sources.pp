#define BUILD_DIRECTORY $[HAVE_AWESOMIUM]
#define BUILDING_DLL BUILDING_PANDAAWESOMIUM

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c

#begin lib_target
  //I don't understand why ode can have TARGET as just pode, here it needs pandaawesomium
  #define TARGET pandaawesomium
  #define LOCAL_LIBS \
    pgraph physics

  #define USE_PACKAGES awesomium
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 

  #define SOURCES \
    awesomium_includes.h config_awesomium.h \
    awWebCore.I awWebCore.h \
    awWebView.I awWebView.h \
    awWebViewListener.I awWebViewListener.h

  #define INCLUDED_SOURCES \
    config_awesomium.cxx \
    awWebCore.cxx \
    awWebView.cxx \
    awWebViewListener.cxx

  #define INSTALL_HEADERS \
    awesomium_includes.h config_awesomium.h \
    awWebCore.h awWebCore.I \
    awWebView.h awWebView.I \
    awWebViewListener.I awWebViewListener.h    

  #define IGATESCAN all

#end lib_target

