#define BUILD_DIRECTORY $[HAVE_AWESOMIUM]
#define BUILDING_DLL BUILDING_PANDAAWESOMIUM

#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#begin lib_target
  //I don't understand why ode can have TARGET as just p3ode, here it needs pandaawesomium
  #define TARGET pandaawesomium
  #define LOCAL_LIBS \
    p3pgraph p3physics

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

