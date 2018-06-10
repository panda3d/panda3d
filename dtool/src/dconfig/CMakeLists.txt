set(P3DCONFIG_HEADERS
  config_dconfig.h dconfig.I dconfig.h)

set(P3DCONFIG_SOURCES
  config_dconfig.cxx dconfig.cxx)

composite_sources(p3dconfig P3DCONFIG_SOURCES)

add_component_library(p3dconfig NOINIT SYMBOL BUILDING_DTOOL_DCONFIG
  ${P3DCONFIG_HEADERS} ${P3DCONFIG_SOURCES})
target_link_libraries(p3dconfig p3prc)

install(TARGETS p3dconfig DESTINATION lib)
install(FILES ${P3DCONFIG_HEADERS} DESTINATION include/panda3d)
