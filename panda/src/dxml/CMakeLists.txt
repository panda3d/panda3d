set(P3DXML_HEADERS
  config_dxml.h
  tinyxml.h
)

set(P3DXML_SOURCES
  config_dxml.cxx
  tinyxml.cpp
  tinyxmlerror.cpp
  tinyxmlparser.cpp
)

composite_sources(p3dxml P3DXML_SOURCES)
add_definitions(-DTIXML_USE_STL)
add_library(p3dxml ${P3DXML_HEADERS} ${P3DXML_SOURCES})
set_target_properties(p3dxml PROPERTIES DEFINE_SYMBOL BUILDING_PANDA_DXML)
target_link_libraries(p3dxml p3express)
target_interrogate(p3dxml ALL)

install(TARGETS p3dxml DESTINATION lib)
install(FILES ${P3DXML_HEADERS} DESTINATION include/panda3d)
