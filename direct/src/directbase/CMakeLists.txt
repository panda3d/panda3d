set(P3DIRECTBASE_SOURCES
  directbase.cxx
)

set(P3DIRECTBASE_HEADERS
  directbase.h directsymbols.h
)

# Not worth compositing sources, there's really only one.
add_library(p3directbase ${P3DIRECTBASE_HEADERS} ${P3DIRECTBASE_SOURCES})
target_link_libraries(p3directbase panda)

install(TARGETS p3directbase DESTINATION lib)
install(FILES ${P3DIRECTBASE_HEADERS} DESTINATION include/panda3d)
