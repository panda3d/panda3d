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

elseif(HAVE_EGL)
  list(APPEND PANDAGL_LINK_TARGETS p3egldisplay)
  set(PANDAGL_PIPE_TYPE "eglGraphicsPipe")

  if(HAVE_X11)
    list(APPEND PANDAGL_LINK_TARGETS p3x11display)
  endif()

else()
  message("") # Add extra line before error
  message(SEND_ERROR
    "When compiling with OpenGL (HAVE_GL), at least one of:
  HAVE_WGL, HAVE_COCOA, HAVE_GLX, or HAVE_EGL must be defined.")

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

if(HAVE_EGL AND HAVE_GLX AND NOT WIN32 AND NOT APPLE)
  # As a temporary solution for #1086, build this module, which we can use as a
  # fallback to OpenGL for headless systems.
  set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "OpenGLDevel")
  add_metalib(p3headlessgl ${MODULE_TYPE}
    INCLUDE "eglGraphicsPipe.h"
    INIT init_libpandagl pandagl.h
    EXPORT int get_pipe_type_p3headlessgl "eglGraphicsPipe::get_class_type().get_index()"
    COMPONENTS p3egldisplay_gl p3glgsg p3glstuff)
  unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

  install(TARGETS p3headlessgl
    EXPORT OpenGL COMPONENT OpenGL
    DESTINATION ${MODULE_DESTINATION}
    ARCHIVE COMPONENT OpenGLDevel)
endif()

export_targets(OpenGL NAMESPACE "Panda3D::OpenGL::" COMPONENT OpenGLDevel)
