#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c

#define USE_PACKAGES opencv artoolkit jpeg

#define BUILDING_DLL BUILDING_VISION

#begin lib_target
  #define TARGET p3vision
  #define LOCAL_LIBS \
    p3display p3text p3pgraph p3gobj p3linmath p3putil p3audio p3movies

  #define COMBINED_SOURCES p3vision_composite1.cxx

  #define SOURCES \
    arToolKit.I arToolKit.h \
    config_vision.h \
    openCVTexture.I openCVTexture.h \
    webcamVideo.h webcamVideo.I \
    webcamVideoCursorOpenCV.h webcamVideoOpenCV.h \
    webcamVideoCursorV4L.h webcamVideoV4L.h

  #define INCLUDED_SOURCES \
    arToolKit.cxx \
    config_vision.cxx \
    openCVTexture.cxx \
    webcamVideo.cxx \
    webcamVideoDS.cxx \
    webcamVideoCursorOpenCV.cxx \
    webcamVideoOpenCV.cxx \
    webcamVideoCursorV4L.cxx \
    webcamVideoV4L.cxx

  #define INSTALL_HEADERS \
    arToolKit.I arToolKit.h \
    openCVTexture.I openCVTexture.h \
    webcamVideo.h webcamVideo.I \
    webcamVideoCursorOpenCV.h webcamVideoOpenCV.h \
    webcamVideoCursorV4L.h webcamVideoV4L.h

  #define IGATESCAN all

#end lib_target

