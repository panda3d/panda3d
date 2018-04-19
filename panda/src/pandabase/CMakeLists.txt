set(P3PANDABASE_HEADERS
  pandabase.h pandasymbols.h
)

set(P3PANDABASE_SOURCES
  pandabase.cxx
)

add_component_library(p3pandabase ${P3PANDABASE_HEADERS} ${P3PANDABASE_SOURCES})
target_link_libraries(p3pandabase p3dtool)

install(TARGETS p3pandabase DESTINATION lib)
install(FILES ${P3PANDABASE_HEADERS} DESTINATION include/panda3d)
