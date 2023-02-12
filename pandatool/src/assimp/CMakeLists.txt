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
target_link_libraries(p3assimp PRIVATE p3pandatoolbase)
target_link_libraries(p3assimp PUBLIC PKG::ASSIMP)

if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  # Do not re-export symbols from these libraries.
  target_link_options(p3assimp PRIVATE "LINKER:--exclude-libs,libassimp.a")
  target_link_options(p3assimp PRIVATE "LINKER:--exclude-libs,libIrrXML.a")
endif()

if(BUILD_SHARED_LIBS)
  # We can't install this if we're doing a static build, because it depends on
  # a static library that isn't installed.
  install(TARGETS p3assimp EXPORT Assimp COMPONENT Assimp DESTINATION ${MODULE_DESTINATION})
endif()
