add_executable(dcparse dcparse.cxx)
target_compile_definitions(dcparse PUBLIC WITHIN_PANDA)
target_link_libraries(dcparse p3dcparser)
install(TARGETS dcparse DESTINATION bin)
