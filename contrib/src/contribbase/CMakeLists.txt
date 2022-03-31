set(P3CONTRIBBASE_SOURCES
  contribbase.cxx
)

set(P3CONTRIBBASE_HEADERS
  contribbase.h contribsymbols.h
)

# Yes, INTERFACE: don't build it, there's no code!
add_library(p3contribbase INTERFACE)
target_link_libraries(p3contribbase INTERFACE panda)

install(TARGETS p3contribbase EXPORT Contrib COMPONENT Contrib)
install(FILES ${P3CONTRIBBASE_HEADERS} COMPONENT ContribDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
