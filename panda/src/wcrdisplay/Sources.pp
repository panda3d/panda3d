#define BUILD_DIRECTORY $[HAVE_WCR]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define WIN_SYS_LIBS Imm32.lib
#define USE_CHROMIUM yes

#begin lib_target
  #define TARGET wcrdisplay
  #define LOCAL_LIBS \
    crgsg display putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 
  
  #define INSTALL_HEADERS \
     config_wcrdisplay.h wcrGraphicsPipe.h wcrGraphicsWindow.h

  #define SOURCES \
    wcrGraphicsWindow.cxx wcrext.h $[INSTALL_HEADERS]
    
  #define INCLUDED_SOURCES \
    config_wcrdisplay.cxx wcrGraphicsPipe.cxx

  #define IGATESCAN wcrGraphicsPipe.h

#end lib_target

#begin test_bin_target
  #define TARGET test_wcr
  #define LOCAL_LIBS \
    putil graph display mathutil gobj sgraph wcrdisplay crgsg

  #define SOURCES \
    test_wcr.cxx

#end test_bin_target

