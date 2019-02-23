set(P3FRAMEWORK_HEADERS
  config_framework.h
  pandaFramework.I pandaFramework.h
  windowFramework.I windowFramework.h
)

set(P3FRAMEWORK_SOURCES
  config_framework.cxx
  pandaFramework.cxx
  windowFramework.cxx
)


set(P3FRAMEWORK_LINK_TARGETS panda)
if(NOT BUILD_SHARED_LIBS)
  # If we're statically linking, we need to explicitly link with
  # at least one of the available renderers.
  if(HAVE_GL)
    set(P3FRAMEWORK_LINK_TARGETS
      ${P3FRAMEWORK_LINK_TARGETS}
      pandagl
    )
  elseif(HAVE_DX9)
    set(P3FRAMEWORK_LINK_TARGETS
      ${P3FRAMEWORK_LINK_TARGETS}
      pandadx9
    )
  elseif(HAVE_TINYDISPLAY)
    set(P3FRAMEWORK_LINK_TARGETS
      ${P3FRAMEWORK_LINK_TARGETS}
      p3tinydisplay
    )
  else()
    message(WARNING
      "No renderer library available to link to p3framework."
    )
  endif()

  # And we might like to have the p3egg loader available.
  if(HAVE_EGG)
    set(P3FRAMEWORK_LINK_TARGETS
      ${P3FRAMEWORK_LINK_TARGETS}
      pandaegg
    )
  endif()
endif()

composite_sources(p3framework P3FRAMEWORK_SOURCES)
# This one isn't a component library
add_library(p3framework ${P3FRAMEWORK_HEADERS} ${P3FRAMEWORK_SOURCES})
set_target_properties(p3framework PROPERTIES DEFINE_SYMBOL BUILDING_FRAMEWORK)
target_link_libraries(p3framework ${P3FRAMEWORK_LINK_TARGETS})

install(TARGETS p3framework
  EXPORT Framework COMPONENT Framework
  DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE COMPONENT FrameworkDevel)
install(FILES ${P3FRAMEWORK_HEADERS} COMPONENT FrameworkDevel DESTINATION include/panda3d)

export_targets(Framework COMPONENT FrameworkDevel)
