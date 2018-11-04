if(NOT HAVE_OPENCV AND NOT HAVE_ARTOOLKIT)
  return()
endif()

set(P3VISION_HEADERS
  arToolKit.h arToolKit.I
  config_vision.h
  openCVTexture.h openCVTexture.I
  webcamVideoCursorOpenCV.h
  webcamVideoCursorV4L.h
  webcamVideo.h webcamVideo.I
  webcamVideoOpenCV.h
  webcamVideoV4L.h
)

set(P3VISION_SOURCES
  arToolKit.cxx
  config_vision.cxx
  openCVTexture.cxx
  webcamVideoCursorOpenCV.cxx
  webcamVideoCursorV4L.cxx
  webcamVideo.cxx
  webcamVideoDS.cxx
  webcamVideoOpenCV.cxx
  webcamVideoV4L.cxx
)

composite_sources(p3vision P3VISION_SOURCES)
add_library(p3vision ${P3VISION_HEADERS} ${P3VISION_SOURCES})
set_target_properties(p3vision PROPERTIES DEFINE_SYMBOL BUILDING_VISION)
target_link_libraries(p3vision panda
  PKG::OPENCV PKG::ARTOOLKIT)
set_target_properties(p3vision PROPERTIES CXX_EXCEPTIONS ON)
target_interrogate(p3vision ALL)

install(TARGETS p3vision DESTINATION lib RUNTIME DESTINATION bin)
install(FILES ${P3VISION_HEADERS} DESTINATION include/panda3d)
