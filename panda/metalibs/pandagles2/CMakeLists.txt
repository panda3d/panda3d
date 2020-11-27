if(NOT HAVE_GLES2 OR NOT HAVE_EAGL)
  return()
endif()

set(PANDAGLES2_LINK_TARGETS p3gles2gsg p3glstuff)

if(HAVE_EGL)
  list(APPEND PANDAGLES2_LINK_TARGETS p3egldisplay_gles2 p3x11display)
  set(PANDAGLES2_PIPE_TYPE "eglGraphicsPipe")
  set(PANDAGLES2_PIPE_INCLUDE "eglGraphicsPipe.h")
elseif(HAVE_EAGL)
  list(APPEND PANDAGLES2_LINK_TARGETS p3eagldisplay)
  set(PANDAGLES2_PIPE_TYPE "EAGLGraphicsPipe")
  set(PANDAGLES2_PIPE_INCLUDE "eaglGraphicsPipe.h")
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "OpenGLES2Devel")
add_metalib(pandagles2 ${MODULE_TYPE}
  INCLUDE ${PANDAGLES2_PIPE_INCLUDE}
  INIT init_libpandagles2 pandagles2.h
  EXPORT int get_pipe_type_pandagles2 "${PANDAGLES2_PIPE_TYPE}::get_class_type().get_index()"
  COMPONENTS ${PANDAGLES2_LINK_TARGETS})
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

install(TARGETS pandagles2
  EXPORT OpenGLES2 COMPONENT OpenGLES2
  DESTINATION ${MODULE_DESTINATION}
  ARCHIVE COMPONENT OpenGLES2Devel)

export_targets(OpenGLES2 COMPONENT OpenGLES2Devel)
