if(NOT HAVE_EGG)
  return()
endif()

set(P3PTLOADER_HEADERS
  config_ptloader.h
  loaderFileTypePandatool.h
)

set(P3PTLOADER_SOURCES
  config_ptloader.cxx
  loaderFileTypePandatool.cxx
)

composite_sources(p3ptloader P3PTLOADER_SOURCES)
add_library(p3ptloader ${MODULE_TYPE} ${P3PTLOADER_HEADERS} ${P3PTLOADER_SOURCES})
set_target_properties(p3ptloader PROPERTIES DEFINE_SYMBOL BUILDING_PTLOADER)
target_link_libraries(p3ptloader PRIVATE
  p3dxfegg p3fltegg p3lwoegg p3vrmlegg p3xfileegg
  p3converter)

if(HAVE_FCOLLADA)
  target_link_libraries(p3ptloader PRIVATE p3daeegg)
  target_compile_definitions(p3ptloader PRIVATE HAVE_FCOLLADA)
endif()

if(BUILD_SHARED_LIBS)
  # We can't install this if we're doing a static build, because it depends on
  # a bunch of static libraries that aren't installed.
  install(TARGETS p3ptloader EXPORT Tools COMPONENT Tools DESTINATION ${MODULE_DESTINATION})
endif()
