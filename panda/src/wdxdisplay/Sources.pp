#define BUILD_DIRECTORY $[HAVE_DX]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define WIN_SYS_LIBS Imm32.lib
#define USE_DX yes

#begin lib_target
  #define TARGET wdxdisplay
  #define LOCAL_LIBS \
    dxgsg
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 
  
  // need to install these due to external projects that link directly with libpandadx (bartop)    
  #define INSTALL_HEADERS \
    config_wdxdisplay.h wdxGraphicsPipe.h wdxGraphicsWindow.h
    
  #define INCLUDED_SOURCES \
    config_wdxdisplay.cxx wdxGraphicsPipe.cxx 
    
  // note SOURCES shoult NOT include INCLUDED_SOURCES, that would cause a double build
  // SOURCES should be headers and separately-built cxx files
  // build wdxGraphicsWindow.cxx separately since its big
  
  #define SOURCES wdxGraphicsWindow.cxx $[INSTALL_HEADERS]
    
#end lib_target

