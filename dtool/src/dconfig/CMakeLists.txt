# Yes, INTERFACE: don't build it, there's no code!
# We're only doing this to put the .h file on the path.
add_library(p3dconfig INTERFACE)

install(FILES dconfig.h COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
install(TARGETS p3dconfig EXPORT Core COMPONENT Core)
