add_definitions(-DWITHIN_PANDA)
add_executable(autorestart autorestart.c)
target_link_libraries(autorestart p3dtool)
