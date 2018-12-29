if(NOT HAVE_EGG)
  return()
endif()

set(P3LWOEGG_HEADERS
  cLwoClip.h cLwoClip.I
  cLwoLayer.h cLwoLayer.I
  cLwoPoints.h cLwoPoints.I
  cLwoPolygons.h cLwoPolygons.I
  cLwoSurfaceBlock.h cLwoSurfaceBlock.I
  cLwoSurfaceBlockTMap.h cLwoSurfaceBlockTMap.I
  cLwoSurface.h cLwoSurface.I
  lwoToEggConverter.h lwoToEggConverter.I
)

set(P3LWOEGG_SOURCES
  cLwoClip.cxx
  cLwoLayer.cxx
  cLwoPoints.cxx
  cLwoPolygons.cxx
  cLwoSurfaceBlock.cxx
  cLwoSurfaceBlockTMap.cxx
  cLwoSurface.cxx
  lwoToEggConverter.cxx
)

add_library(p3lwoegg STATIC ${P3LWOEGG_HEADERS} ${P3LWOEGG_SOURCES})
target_link_libraries(p3lwoegg p3lwo p3eggbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
