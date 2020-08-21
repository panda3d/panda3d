if(NOT HAVE_EGG)
  return()
endif()

set(P3XFILEEGG_HEADERS
  xFileAnimationSet.h xFileAnimationSet.I
  xFileFace.h xFileMaker.h
  xFileMaterial.h xFileMesh.h xFileNormal.h
  xFileToEggConverter.h
  xFileVertex.h xFileVertexPool.h
)

set(P3XFILEEGG_SOURCES
  xFileAnimationSet.cxx
  xFileFace.cxx xFileMaker.cxx
  xFileMaterial.cxx xFileMesh.cxx xFileNormal.cxx
  xFileToEggConverter.cxx
  xFileVertex.cxx
)

composite_sources(p3xfileegg P3XFILEEGG_SOURCES)
add_library(p3xfileegg STATIC ${P3XFILEEGG_HEADERS} ${P3XFILEEGG_SOURCES})
target_link_libraries(p3xfileegg p3xfile p3eggbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
