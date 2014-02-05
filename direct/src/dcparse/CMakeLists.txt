add_executable(dcparse dcparse.cxx)
set_target_properties(dcparse PROPERTIES COMPILE_DEFINITIONS WITHIN_PANDA)
target_link_libraries(dcparse p3dcparser)
install(TARGETS dcparse DESTINATION bin)
