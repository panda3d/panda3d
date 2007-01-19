#define BUILD_DIRECTORY $[WINDOWS_PLATFORM]

#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
#define WIN_SYS_LIBS Imm32.lib

#define BUILDING_DLL BUILDING_PANDAWIN

#define USE_PACKAGES dx

#begin lib_target
  #define TARGET windisplay
  #define LOCAL_LIBS \
    display putil
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx winDetectDx9.cxx winDetectDx8.cxx winDetectDx7.cxx

  #define SOURCES \
     config_windisplay.h \
     winGraphicsPipe.I winGraphicsPipe.h \
     winGraphicsWindow.I winGraphicsWindow.h \
     winDetectDx.h
   
  #define INSTALL_HEADERS \
     config_windisplay.h \
     winGraphicsPipe.I winGraphicsPipe.h \
     winGraphicsWindow.I winGraphicsWindow.h
//     Win32Defs.h  
    
  #define INCLUDED_SOURCES \
     config_windisplay.cxx winGraphicsPipe.cxx \
     winGraphicsWindow.cxx \
     winDetectDx9.cxx winDetectDx8.cxx winDetectDx7.cxx

  #define WIN_SYS_LIBS Imm32.lib winmm.lib kernel32.lib oldnames.lib user32.lib gdi32.lib

#end lib_target
