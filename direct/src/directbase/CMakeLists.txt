set(P3DIRECTBASE_SOURCES
  directbase.cxx
)

set(P3DIRECTBASE_HEADERS
  directbase.h directsymbols.h
)

# Yes, INTERFACE: don't build it, there's no code!
add_library(p3directbase INTERFACE)
target_link_libraries(p3directbase INTERFACE panda)

install(TARGETS p3directbase EXPORT Direct COMPONENT Direct)
install(FILES ${P3DIRECTBASE_HEADERS} COMPONENT DirectDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
