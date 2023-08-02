if(NOT HAVE_GL)
  return()
endif()

set(PANDAGL_LINK_TARGETS p3glgsg p3glstuff)

if(HAVE_GLX)
  list(APPEND PANDAGL_LINK_TARGETS p3glxdisplay p3x11display)
  set(PANDAGL_PIPE_TYPE "glxGraphicsPipe")

elseif(HAVE_WGL)
  list(APPEND PANDAGL_LINK_TARGETS p3wgldisplay p3windisplay)
  set(PANDAGL_PIPE_TYPE "wglGraphicsPipe")

elseif(HAVE_COCOA)
  list(APPEND PANDAGL_LINK_TARGETS p3cocoagldisplay p3cocoadisplay)
  set(PANDAGL_PIPE_TYPE "CocoaGLGraphicsPipe")
  set(PANDAGL_PIPE_INCLUDE "cocoaGLGraphicsPipe.h")

else()
  message("") # Add extra line before error
  message(SEND_ERROR
    "When compiling with OpenGL (HAVE_GL), at least one of:
  HAVE_WGL, HAVE_COCOA, or HAVE_GLX must be defined.")

endif()

if(NOT PANDAGL_PIPE_INCLUDE)
  set(PANDAGL_PIPE_INCLUDE "${PANDAGL_PIPE_TYPE}.h")
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "OpenGLDevel")
add_metalib(pandagl ${MODULE_TYPE}
  INCLUDE "${PANDAGL_PIPE_INCLUDE}"
  INIT init_libpandagl pandagl.h
  EXPORT int get_pipe_type_pandagl "${PANDAGL_PIPE_TYPE}::get_class_type().get_index()"
  COMPONENTS ${PANDAGL_LINK_TARGETS})
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

install(TARGETS pandagl
  EXPORT OpenGL COMPONENT OpenGL
  DESTINATION ${MODULE_DESTINATION}
  ARCHIVE COMPONENT OpenGLDevel)

export_targets(OpenGL NAMESPACE "Panda3D::OpenGL::" COMPONENT OpenGLDevel)
