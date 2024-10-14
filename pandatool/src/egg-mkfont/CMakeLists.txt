if(NOT BUILD_TOOLS)
  return()
endif()

if(NOT HAVE_EGG OR NOT HAVE_FREETYPE)
  return()
endif()

set(P3EGG_MKFONT_HEADERS
  eggMakeFont.h
  rangeDescription.h rangeDescription.I
  rangeIterator.h rangeIterator.I
)

set(P3EGG_MKFONT_SOURCES
  eggMakeFont.cxx
  rangeDescription.cxx
  rangeIterator.cxx
)

composite_sources(egg-mkfont P3EGG_MKFONT_SOURCES)
add_executable(egg-mkfont ${P3EGG_MKFONT_HEADERS} ${P3EGG_MKFONT_SOURCES})
target_link_libraries(egg-mkfont p3palettizer p3eggbase p3pandatoolbase)

install(TARGETS egg-mkfont EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})
