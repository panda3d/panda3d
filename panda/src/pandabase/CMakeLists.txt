set(P3PANDABASE_HEADERS
  pandabase.h pandasymbols.h
)

set(P3PANDABASE_SOURCES
  pandabase.cxx
)

# Yes, INTERFACE: don't build it, there's no code!
add_library(p3pandabase INTERFACE)
target_link_libraries(p3pandabase INTERFACE p3dtool)

install(FILES ${P3PANDABASE_HEADERS} DESTINATION include/panda3d)
