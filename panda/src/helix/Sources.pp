#define BUILD_DIRECTORY $[HAVE_HELIX]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m

#begin lib_target
  #define TARGET helix
  #define LOCAL_LIBS \
   dgraph
  #define WIN_SYS_LIBS $[WIN_SYS_LIBS] user32.lib advapi32.lib winmm.lib
  #define USE_PACKAGES helix

  #define SOURCES \
    config_helix.cxx config_helix.h fivemmap.cxx fivemmap.h \
    HelixClient.cxx HelixClient.h HxAdviseSink.cxx HxAdviseSink.h \ 
    HxAuthenticationManager.cxx HxAuthenticationManager.h \
    HxClientContext.cxx HxClientContext.h HxErrorSink.cxx HxErrorSink.h \
    HxSiteSupplier.cxx HxSiteSupplier.h iids.cxx MainHelix.h print.cxx print.h
    
   
  #define INSTALL_HEADERS \
    config_helix.h \
    HelixClient.h

  #define IGATESCAN \
    HelixClient.cxx

#end lib_target
