if(NOT HAVE_GLES1 OR NOT HAVE_EGL)
  return()
endif()

add_metalib(pandagles ${MODULE_TYPE} pandagles.cxx
  COMPONENTS p3egldisplay_gles1 p3glesgsg)
set_target_properties(pandagles PROPERTIES DEFINE_SYMBOL BUILDING_PANDAGLES)

install(TARGETS pandagles DESTINATION ${MODULE_DESTINATION})
