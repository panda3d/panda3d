#define DIRECTORY_IF_DX yes

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define WIN_SYS_LIBS Imm32.lib
#define USE_DX yes

#begin lib_target
  #define TARGET wdxdisplay8
  #define LOCAL_LIBS dxgsg8
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 
  
  // need to install these due to external projects that link directly with libpandadx (bartop)    
  #define INSTALL_HEADERS \
    config_wdxdisplay8.h wdxGraphicsPipe8.h wdxGraphicsWindow8.h
    
  #define INCLUDED_SOURCES \
    config_wdxdisplay8.cxx wdxGraphicsPipe8.cxx 
    
  // note SOURCES shoult NOT include INCLUDED_SOURCES, that would cause a double build
  // SOURCES should be headers and separately-built cxx files
  // build wdxGraphicsWindow.cxx separately since its big
  
  #define SOURCES wdxGraphicsWindow8.cxx $[INSTALL_HEADERS]
    
#end lib_target
