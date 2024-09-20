if(NOT BUILD_TOOLS)
  return()
endif()

if(NOT HAVE_EGG)
  return()
endif()

set(P3EGG_OPTCHAR_HEADERS
  config_egg_optchar.h
  eggOptchar.h
  eggOptcharUserData.h eggOptcharUserData.I
  vertexMembership.h vertexMembership.I
)

set(P3EGG_OPTCHAR_SOURCES
  config_egg_optchar.cxx
  eggOptchar.cxx
  eggOptcharUserData.cxx
  vertexMembership.cxx
)

composite_sources(egg-optchar P3EGG_OPTCHAR_SOURCES)
add_executable(egg-optchar ${P3EGG_OPTCHAR_HEADERS} ${P3EGG_OPTCHAR_SOURCES})
target_link_libraries(egg-optchar p3eggcharbase)

install(TARGETS egg-optchar EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})
