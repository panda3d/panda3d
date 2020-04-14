set(P3DXF_HEADERS
  dxfFile.h
  dxfLayer.h
  dxfLayerMap.h
  dxfVertex.h
)

set(P3DXF_SOURCES
  dxfFile.cxx
  dxfLayer.cxx
  dxfLayerMap.cxx
  dxfVertex.cxx
)

composite_sources(p3dxf P3DXF_SOURCES)
add_library(p3dxf STATIC ${P3DXF_HEADERS} ${P3DXF_SOURCES})
target_link_libraries(p3dxf p3pandatoolbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
