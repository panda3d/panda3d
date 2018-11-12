if(NOT HAVE_ASSIMP)
  return()
endif()

set(P3ASSIMP_HEADERS
  assimpLoader.h assimpLoader.I
  config_assimp.h
  loaderFileTypeAssimp.h
  pandaIOStream.h
  pandaIOSystem.h
  pandaLogger.h
)

set(P3ASSIMP_SOURCES
  assimpLoader.cxx
  config_assimp.cxx
  loaderFileTypeAssimp.cxx
  pandaIOStream.cxx
  pandaIOSystem.cxx
  pandaLogger.cxx
)

composite_sources(p3assimp P3ASSIMP_SOURCES)
add_library(p3assimp ${MODULE_TYPE} ${P3ASSIMP_HEADERS} ${P3ASSIMP_SOURCES})
set_target_properties(p3assimp PROPERTIES DEFINE_SYMBOL BUILDING_ASSIMP)
target_link_libraries(p3assimp p3pandatoolbase
  PKG::ASSIMP)

install(TARGETS p3assimp DESTINATION ${MODULE_DESTINATION})
