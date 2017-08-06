set(P3FLTEGG_HEADERS
    fltToEggConverter.I fltToEggConverter.h
    fltToEggLevelState.I fltToEggLevelState.h)
  
set(P3FLTEGG_SOURCES
    fltToEggConverter.cxx
    fltToEggLevelState.cxx)

add_library(p3fltegg STATIC ${P3FLTEGG_HEADERS} ${P3FLTEGG_SOURCES})
target_link_libraries(p3fltegg p3flt p3converter)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
