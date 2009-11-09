#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c movies:m

#define USE_PACKAGES opencv artoolkit

#define BUILDING_DLL BUILDING_VISION

#begin lib_target
  #define TARGET vision
  #define LOCAL_LIBS \
    display text pgraph gobj linmath putil movies audio
    
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx

  #define SOURCES \
    arToolKit.I arToolKit.h \
    config_vision.h \
    openCVTexture.I openCVTexture.h \
    webcamVideo.h webcamVideo.I
    
  #define INCLUDED_SOURCES \
    arToolKit.cxx \
    config_vision.cxx \
    openCVTexture.cxx \
    webcamVideo.cxx \
    webcamVideoDS.cxx
    
  #define INSTALL_HEADERS \
    arToolKit.I arToolKit.h \
    openCVTexture.I openCVTexture.h \
    webcamVideo.h webcamVideo.I

  #define IGATESCAN all

#end lib_target

