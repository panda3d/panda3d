if(NOT HAVE_GLES1 OR NOT HAVE_EGL)
  return()
endif()

if(ANDROID)
  set(GLES1_PIPE_TYPE "AndroidGraphicsPipe")
  set(GLES1_PIPE_INCLUDE "androidGraphicsPipe.h")
else()
  set(GLES1_PIPE_TYPE "eglGraphicsPipe")
  set(GLES1_PIPE_INCLUDE "eglGraphicsPipe.h")
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "GLESDevel")
add_metalib(pandagles ${MODULE_TYPE}
  INCLUDE "${GLES1_PIPE_INCLUDE}"
  INIT init_libpandagles pandagles.h
  EXPORT int get_pipe_type_pandagles "${GLES1_PIPE_TYPE}::get_class_type().get_index()"
  COMPONENTS p3egldisplay_gles1 p3glesgsg p3glstuff p3x11display)
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

install(TARGETS pandagles
  EXPORT GLES COMPONENT GLES
  DESTINATION ${MODULE_DESTINATION}
  ARCHIVE COMPONENT GLESDevel)

if(NOT HAVE_GLES2)
  # If GLES2 won't export this, we should do so here.
  export_targets(GLES COMPONENT GLESDevel)
endif()
