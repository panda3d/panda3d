if(NOT HAVE_GLES2 OR NOT HAVE_EGL)
  return()
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "GLESDevel")
add_metalib(pandagles2 ${MODULE_TYPE}
  INCLUDE eglGraphicsPipe.h
  INIT init_libpandagles2 pandagles2.h
  EXPORT int get_pipe_type_pandagles2 "eglGraphicsPipe::get_class_type().get_index()"
  COMPONENTS p3egldisplay_gles2 p3gles2gsg p3glstuff p3x11display)
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

install(TARGETS pandagles2
  EXPORT GLES COMPONENT GLES
  DESTINATION ${MODULE_DESTINATION}
  ARCHIVE COMPONENT GLESDevel)

export_targets(GLES COMPONENT GLESDevel)
