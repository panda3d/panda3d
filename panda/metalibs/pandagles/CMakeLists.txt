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

add_metalib(pandagles ${MODULE_TYPE}
  INCLUDE "${GLES1_PIPE_INCLUDE}"
  INIT init_libpandagles pandagles.h
  EXPORT int get_pipe_type_pandagles "${GLES1_PIPE_TYPE}::get_class_type().get_index()"
  COMPONENTS p3egldisplay_gles1 p3glesgsg)

install(TARGETS pandagles DESTINATION ${MODULE_DESTINATION})
