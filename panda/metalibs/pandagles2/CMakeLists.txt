if(NOT HAVE_GLES2 OR NOT HAVE_EGL)
  return()
endif()

add_metalib(pandagles2 ${MODULE_TYPE} pandagles2.cxx
  COMPONENTS p3egldisplay_gles2 p3gles2gsg)
set_target_properties(pandagles2 PROPERTIES DEFINE_SYMBOL BUILDING_PANDAGLES2)

install(TARGETS pandagles2 DESTINATION ${MODULE_DESTINATION})
