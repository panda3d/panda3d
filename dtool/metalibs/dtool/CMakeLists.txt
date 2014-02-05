add_library(p3dtool dtool.cxx)
target_link_libraries(p3dtool p3dtoolutil p3dtoolbase)

install(TARGETS p3dtool DESTINATION lib)
