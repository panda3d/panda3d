## Configure dtool ##
configure_file(dtool_config.h.cmake ${CMAKE_BINARY_DIR}/include/dtool_config.h)

## Include dtool subpackages ##
add_subdirectory(src/dtoolbase)
add_subdirectory(src/dtoolutil)
add_subdirectory(src/prc)
add_subdirectory(src/dconfig)
add_subdirectory(src/pystub)
add_subdirectory(src/cppparser)
add_subdirectory(src/interrogatedb)
add_subdirectory(src/interrogate)

## Include dtool metalibs ##
add_subdirectory(metalibs/dtool)
