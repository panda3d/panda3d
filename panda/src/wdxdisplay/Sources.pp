#define DIRECTORY_IF_DX yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_DX yes

#begin lib_target
  #define TARGET wdxdisplay
  #define LOCAL_LIBS \
    dxgsg
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 
    
  #define SOURCES \
    config_wdxdisplay.h wdxGraphicsPipe.h wdxGraphicsWindow.cxx wdxGraphicsWindow.h
    
  #define INCLUDED_SOURCES \
    config_wdxdisplay.cxx wdxGraphicsPipe.cxx 
    
  #define INSTALL_HEADERS \
    config_wdxdisplay.h wdxGraphicsPipe.h wdxGraphicsWindow.h
    
#end lib_target

