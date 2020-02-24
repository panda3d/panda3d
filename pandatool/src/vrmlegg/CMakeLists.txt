if(NOT HAVE_EGG)
  return()
endif()

set(P3VRMLEGG_HEADERS
  indexedFaceSet.h
  vrmlAppearance.h
  vrmlToEggConverter.h
)

set(P3VRMLEGG_SOURCES
  indexedFaceSet.cxx
  vrmlAppearance.cxx
  vrmlToEggConverter.cxx
)

composite_sources(p3vrmlegg P3VRMLEGG_SOURCES)
add_library(p3vrmlegg STATIC ${P3VRMLEGG_HEADERS} ${P3VRMLEGG_SOURCES})
target_link_libraries(p3vrmlegg p3vrml p3eggbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
