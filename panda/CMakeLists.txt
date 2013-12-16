if(NOT BUILD_DTOOL)
  message(FATAL_ERROR "Cannot build panda without dtool!  Please enable the BUILD_DTOOL option.")
endif()

# Include panda source directories
add_subdirectory(src/pandabase)
add_subdirectory(src/express)
add_subdirectory(src/downloader)
