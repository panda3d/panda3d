#define DIRECTORY_IF_DX yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define USE_DX yes

#if $[BUILD_DX8]
#begin lib_target
  #define TARGET wdxdisplay8
  #define LOCAL_LIBS dxgsg8
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 
    
  #define SOURCES \
    config_wdxdisplay8.h wdxGraphicsPipe8.h wdxGraphicsWindow8.cxx wdxGraphicsWindow8.h
    
  #define INCLUDED_SOURCES config_wdxdisplay8.cxx wdxGraphicsPipe8.cxx 
    
#end lib_target
#endif
