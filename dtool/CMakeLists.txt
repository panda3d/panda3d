# Generate dtool_config.h
include(LocalSetup.cmake)

# Include dtool source directories
add_subdirectory(src/dconfig)
add_subdirectory(src/dtoolbase)
add_subdirectory(src/dtoolutil)
add_subdirectory(src/interrogatedb)
add_subdirectory(src/prc)

# Include dtool metalib
add_subdirectory(metalibs/dtool)
