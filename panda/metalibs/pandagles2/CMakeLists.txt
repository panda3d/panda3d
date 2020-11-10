if(ANDROID AND NOT HAVE_EGL)
    if(HAVE_GLES2)
        set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "OpenGLES2Devel")
        add_metalib(pandagles2 ${MODULE_TYPE}
          INCLUDE androidGraphicsPipe.h
          INIT init_libpandagles2 pandagles2.h
          EXPORT int get_pipe_type_pandagles2 "AndroidGraphicsPipe::get_class_type().get_index()"
          COMPONENTS p3androiddisplay_gles2 p3glstuff p3gles2gsg)
        unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)

        install(TARGETS pandagles2
          EXPORT OpenGLES2 COMPONENT OpenGLES2
          DESTINATION ${MODULE_DESTINATION}
          ARCHIVE COMPONENT OpenGLES2Devel)

        export_targets(OpenGLES2 COMPONENT OpenGLES2Devel)
    endif()

    #discarded GLES1

  return()
endif()

if(NOT HAVE_GLES2 OR NOT HAVE_EGL)
  return()
endif()

set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "OpenGLES2Devel")
add_metalib(pandagles2 ${MODULE_TYPE}
  INCLUDE eglGraphicsPipe.h
  INIT init_libpandagles2 pandagles2.h
  EXPORT int get_pipe_type_pandagles2 "eglGraphicsPipe::get_class_type().get_index()"
  COMPONENTS p3egldisplay_gles2 p3gles2gsg p3glstuff p3x11display)
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)



install(TARGETS pandagles2
  EXPORT OpenGLES2 COMPONENT OpenGLES2
  DESTINATION ${MODULE_DESTINATION}
  ARCHIVE COMPONENT OpenGLES2Devel)

export_targets(OpenGLES2 COMPONENT OpenGLES2Devel)
