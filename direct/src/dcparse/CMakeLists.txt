add_executable(p3dcparse dcparse.cxx)
target_link_libraries(p3dcparse p3direct p3pystub)
install(TARGETS p3dcparse EXPORT Direct COMPONENT Direct DESTINATION bin)
