if(UNIX)
  add_executable(autorestart autorestart.c)
  set_target_properties(autorestart PROPERTIES COMPILE_DEFINITIONS WITHIN_PANDA)
  target_link_libraries(autorestart p3dtool)

  install(TARGETS autorestart DESTINATION bin)
endif()
