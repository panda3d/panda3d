set(P3GSGBASE_HEADERS
  config_gsgbase.h
  graphicsOutputBase.I graphicsOutputBase.h
  graphicsStateGuardianBase.h
)

set(P3GSGBASE_SOURCES
  config_gsgbase.cxx
  graphicsOutputBase.cxx
  graphicsStateGuardianBase.cxx
)

composite_sources(p3gsgbase P3GSGBASE_SOURCES)
add_component_library(p3gsgbase NOINIT SYMBOL BUILDING_PANDA_GSGBASE
  ${P3GSGBASE_HEADERS} ${P3GSGBASE_SOURCES})
target_link_libraries(p3gsgbase p3putil p3linmath)
target_interrogate(p3gsgbase ALL)

if(NOT BUILD_METALIBS)
  install(TARGETS p3gsgbase DESTINATION lib RUNTIME DESTINATION bin)
endif()
install(FILES ${P3GSGBASE_HEADERS} DESTINATION include/panda3d)

#begin test_bin_target
  #define TARGET test_gsgbase
  #define LOCAL_LIBS
#    p3gsgbase

  #define SOURCES
#    test_gsgbase.cxx

#end test_bin_target
