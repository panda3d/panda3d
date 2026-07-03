if(NOT HAVE_RMLUI)
  return()
endif()

set(P3RMLUI_HEADERS
  config_rmlui.h
  rmlContext.h
  rmlDataModel.h
  rmlDocument.h
  rmlElement.h
  rmlEvent.h
  rmlFileInterface.h
  rmlInputHandler.h
  rmlPythonUtil.h
  rmlRegion.I rmlRegion.h
  rmlRenderInterface.h
  rmlSystemInterface.h
)

set(P3RMLUI_SOURCES
  config_rmlui.cxx
  rmlContext.cxx
  rmlDataModel.cxx
  rmlDocument.cxx
  rmlElement.cxx
  rmlEvent.cxx
  rmlFileInterface.cxx
  rmlInputHandler.cxx
  rmlRegion.cxx
  rmlRenderInterface.cxx
  rmlSystemInterface.cxx
)

composite_sources(p3rmlui P3RMLUI_SOURCES)
add_library(p3rmlui ${P3RMLUI_SOURCES} ${P3RMLUI_HEADERS})
set_target_properties(p3rmlui PROPERTIES DEFINE_SYMBOL BUILDING_PANDARMLUI)
target_link_libraries(p3rmlui panda PKG::RMLUI)
if(HAVE_PYTHON)
  target_include_directories(p3rmlui PRIVATE "${PROJECT_SOURCE_DIR}/dtool/src/interrogatedb")
  target_link_libraries(p3rmlui PKG::PYTHON)
endif()
# Only interrogate the files with PUBLISHED: annotations; the interface
# adapter files (rmlFileInterface, rmlRenderInterface, rmlSystemInterface)
# contain Rml:: types that the CPPPARSER cannot resolve.
target_interrogate(p3rmlui config_rmlui.h rmlInputHandler.h rmlRegion.h rmlContext.h rmlDocument.h rmlElement.h rmlEvent.h rmlDataModel.h)

if(HAVE_RMLUI_DEBUGGER)
  target_compile_definitions(p3rmlui PRIVATE HAVE_RMLUI_DEBUGGER)
  target_link_libraries(p3rmlui RmlUi::Debugger)
endif()

install(TARGETS p3rmlui
  EXPORT RmlUi COMPONENT RmlUi
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT RmlUiDevel)
install(FILES ${P3RMLUI_HEADERS}
  COMPONENT RmlUiDevel
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

export_targets(RmlUi COMPONENT RmlUiDevel)
