if(NOT IS_IOS OR NOT HAVE_GLES2 OR NOT HAVE_EAGL)
  return()
endif()

set(P3EAGLDISPLAY_HEADERS
  config_eagldisplay.h
  eaglGraphicsPipe.h eaglGraphicsPipe.I
  eaglGraphicsStateGuardian.h eaglGraphicsStateGuardian.I
  eaglGraphicsWindow.h eaglGraphicsWindow.I
  pandaEAGLView.h
  PandaViewController.h
)

set(P3EAGLDISPLAY_SOURCES
  config_eagldisplay.mm
  eaglGraphicsPipe.mm
  eaglGraphicsStateGuardian.mm
  eaglGraphicsWindow.mm
  pandaEAGLView.mm
  PandaViewController.mm
)

composite_sources(p3eagldisplay P3EAGLDISPLAY_SOURCES)
add_component_library(p3eagldisplay SYMBOL BUILDING_PANDA_EAGLDISPLAY
  ${P3EAGLDISPLAY_HEADERS} ${P3EAGLDISPLAY_SOURCES})
target_link_libraries(p3eagldisplay p3gles2gsg panda)
target_compile_options(p3eagldisplay PRIVATE -fobjc-arc)

# Frameworks:
find_library(UIKIT_LIBRARY UIKit)
find_library(QUARTZCORE_LIBRARY QuartzCore)
target_link_libraries(p3eagldisplay ${UIKIT_LIBRARY} ${QUARTZCORE_LIBRARY})

if(NOT BUILD_METALIBS)
  install(TARGETS p3eagldisplay EXPORT GLES COMPONENT GLES DESTINATION lib)
endif()
install(FILES ${P3EAGLDISPLAY_HEADERS} COMPONENT GLES DESTINATION include/panda3d)