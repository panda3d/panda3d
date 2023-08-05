if(NOT HAVE_EGG)
  return()
endif()

set(P3DXFEGG_HEADERS
  dxfToEggConverter.h
  dxfToEggLayer.h
)

set(P3DXFEGG_SOURCES
  dxfToEggConverter.cxx
  dxfToEggLayer.cxx
)

add_library(p3dxfegg STATIC ${P3DXFEGG_HEADERS} ${P3DXFEGG_SOURCES})
target_link_libraries(p3dxfegg p3dxf p3eggbase)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
