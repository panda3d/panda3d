if(NOT HAVE_EGG)
  return()
endif()

set(P3FLTEGG_HEADERS
  fltToEggConverter.h fltToEggConverter.I
  fltToEggLevelState.h fltToEggLevelState.I
)

set(P3FLTEGG_SOURCES
  fltToEggConverter.cxx
  fltToEggLevelState.cxx
)

add_library(p3fltegg STATIC ${P3FLTEGG_HEADERS} ${P3FLTEGG_SOURCES})
target_link_libraries(p3fltegg p3flt p3eggbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
