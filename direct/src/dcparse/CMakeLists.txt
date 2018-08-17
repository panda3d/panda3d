add_executable(p3dcparse dcparse.cxx)
target_compile_definitions(p3dcparse PUBLIC WITHIN_PANDA)
target_link_libraries(p3dcparse p3direct)
install(TARGETS p3dcparse DESTINATION bin)
