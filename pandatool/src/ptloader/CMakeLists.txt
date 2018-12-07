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
  p3dxfegg p3fltegg p3lwoegg p3objegg p3vrmlegg p3xfileegg
  p3converter)

install(TARGETS p3ptloader EXPORT Tools COMPONENT Tools DESTINATION ${MODULE_DESTINATION})
