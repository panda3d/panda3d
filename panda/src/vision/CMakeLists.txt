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

if(OPENCV_VER_3)
  set_target_properties(p3vision PROPERTIES COMPILE_DEFINITIONS "OPENCV_VER_3")
elseif(OPENCV_VER_23)
  set_target_properties(p3vision PROPERTIES COMPILE_DEFINITIONS "OPENCV_VER_23")
endif()

if(HAVE_FFMPEG)
  target_compile_definitions(p3vision PRIVATE HAVE_FFMPEG)
endif()

if(HAVE_ARTOOLKIT AND CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  # Do not re-export symbols from these libraries.
  target_link_options(p3vision PRIVATE "LINKER:--exclude-libs,libAR.a")
  target_link_options(p3vision PRIVATE "LINKER:--exclude-libs,libARMulti.a")
endif()

install(TARGETS p3vision
  EXPORT Vision COMPONENT Vision
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT VisionDevel)
install(FILES ${P3VISION_HEADERS} COMPONENT VisionDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

export_targets(Vision COMPONENT VisionDevel)
