#define BUILD_DIRECTORY $[WINDOWS_PLATFORM]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define WIN_SYS_LIBS Imm32.lib
#define BUILDING_DLL BUILDING_PANDAWIN

#begin lib_target
  #define TARGET windisplay
  #define LOCAL_LIBS \
    display putil
    
//  #define COMBINED_SOURCES $[TARGET]_composite1.cxx 
  
  #define INSTALL_HEADERS \
     config_windisplay.h \
     winGraphicsPipe.I winGraphicsPipe.h \
     winGraphicsWindow.I winGraphicsWindow.h
//     Win32Defs.h  
    
  #define INCLUDED_SOURCES \
    config_windisplay.cxx winGraphicsPipe.cxx

  #define SOURCES \
    winGraphicsWindow.cxx $[INSTALL_HEADERS]

  #define WIN_SYS_LIBS Imm32.lib winmm.lib kernel32.lib oldnames.lib user32.lib gdi32.lib

#end lib_target
