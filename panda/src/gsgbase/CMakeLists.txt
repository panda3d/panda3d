set(P3GSGBASE_HEADERS
  config_gsgbase.h
  displayRegionBase.I displayRegionBase.h
  graphicsOutputBase.I graphicsOutputBase.h
  graphicsStateGuardianBase.h
)

set(P3GSGBASE_SOURCES
  config_gsgbase.cxx
  displayRegionBase.cxx
  graphicsOutputBase.cxx
  graphicsStateGuardianBase.cxx
)

composite_sources(p3gsgbase P3GSGBASE_SOURCES)
add_library(p3gsgbase ${P3GSGBASE_HEADERS} ${P3GSGBASE_SOURCES})
target_link_libraries(p3gsgbase p3linmath)
target_interrogate(p3gsgbase ALL)

install(TARGETS p3gsgbase DESTINATION lib)

#begin test_bin_target
  #define TARGET test_gsgbase
  #define LOCAL_LIBS
#    p3gsgbase

  #define SOURCES
#    test_gsgbase.cxx

#end test_bin_target
