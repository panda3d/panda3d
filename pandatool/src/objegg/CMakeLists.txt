if(NOT HAVE_EGG)
  return()
endif()

set(P3OBJEGG_HEADERS
  config_objegg.h
  eggToObjConverter.h
  objToEggConverter.h
  objToEggConverter.I
)

set(P3OBJEGG_SOURCES
  config_objegg.cxx
  eggToObjConverter.cxx
  objToEggConverter.cxx
)

composite_sources(p3objegg P3OBJEGG_SOURCES)
add_library(p3objegg STATIC ${P3OBJEGG_HEADERS} ${P3OBJEGG_SOURCES})
target_link_libraries(p3objegg p3eggbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
