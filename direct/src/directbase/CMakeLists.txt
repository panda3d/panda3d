set(P3DIRECTBASE_SOURCES
  directbase.cxx
)

set(P3DIRECTBASE_HEADERS
  directbase.h directsymbols.h
)

# Not worth compositing sources, there's really only one.
add_library(p3directbase ${P3DIRECTBASE_SOURCES} ${P3DIRECTBASE_HEADERS})
target_link_libraries(p3directbase p3pandabase)

install(TARGETS p3directbase DESTINATION lib)
