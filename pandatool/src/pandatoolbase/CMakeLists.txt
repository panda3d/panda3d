set(P3PANDATOOLBASE_HEADERS
    animationConvert.h
    config_pandatoolbase.h
    distanceUnit.h
    pandatoolbase.h pandatoolsymbols.h
    pathReplace.I pathReplace.h
    pathStore.h)

set(P3PANDATOOLBASE_SOURCES
    animationConvert.cxx
    config_pandatoolbase.cxx
    distanceUnit.cxx
    pandatoolbase.cxx
    pathReplace.cxx
    pathStore.cxx)

composite_sources(p3pandatoolbase P3PANDATOOLBASE_SOURCES)
add_library(p3pandatoolbase STATIC ${P3PANDATOOLBASE_HEADERS} ${P3PANDATOOLBASE_SOURCES})
target_link_libraries(p3pandatoolbase p3pandabase p3putil)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
