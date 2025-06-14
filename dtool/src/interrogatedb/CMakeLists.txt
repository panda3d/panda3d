set(P3INTERROGATEDB_HEADERS
  config_interrogatedb.h indexRemapper.h interrogateComponent.I
  interrogateComponent.h interrogateDatabase.I
  interrogateDatabase.h interrogateElement.I
  interrogateElement.h interrogateFunction.I
  interrogateFunction.h interrogateFunctionWrapper.I
  interrogateFunctionWrapper.h
  interrogateMakeSeq.I interrogateMakeSeq.h
  interrogateManifest.I interrogateManifest.h
  interrogateType.I interrogateType.h
  interrogate_datafile.I interrogate_datafile.h
  interrogate_interface.h interrogate_request.h
)

set(P3INTERROGATEDB_SOURCES
  config_interrogatedb.cxx
  indexRemapper.cxx
  interrogateComponent.cxx interrogateDatabase.cxx
  interrogateElement.cxx interrogateFunction.cxx
  interrogateFunctionWrapper.cxx
  interrogateMakeSeq.cxx
  interrogateManifest.cxx
  interrogateType.cxx interrogate_datafile.cxx
  interrogate_interface.cxx interrogate_request.cxx
)

set(P3INTERROGATEDB_IGATE
  interrogate_interface.h
  interrogate_request.h
)

set(P3IGATERUNTIME_HEADERS
  extension.h py_compat.h py_panda.h py_panda.I py_wrappers.h
)

composite_sources(p3interrogatedb P3INTERROGATEDB_SOURCES)
add_library(p3interrogatedb
  ${P3INTERROGATEDB_HEADERS} ${P3INTERROGATEDB_SOURCES})
set_target_properties(p3interrogatedb PROPERTIES DEFINE_SYMBOL BUILDING_INTERROGATEDB)
target_link_libraries(p3interrogatedb p3dconfig p3prc)

install(TARGETS p3interrogatedb
  EXPORT Core COMPONENT Core
  DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
  ARCHIVE COMPONENT CoreDevel)
install(FILES ${P3INTERROGATEDB_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
install(FILES ${P3IGATERUNTIME_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

# ALSO: This has an Interrogate binding! Take care of that if we want it.
# Note we don't use the regular Interrogate macros; this has some custom flags
# that would make it not worthwhile.

if(NOT INTERROGATE_PYTHON_INTERFACE)
  return()
endif()

add_custom_command(
  OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/interrogatedb_module.cxx"
  COMMAND host_interrogate
    -D EXPCL_INTERROGATEDB=
    -nodb -python -promiscuous
    -module panda3d.interrogatedb
    -library interrogatedb
    -string -true-names -do-module
    -srcdir "${CMAKE_CURRENT_SOURCE_DIR}"
    -oc "${CMAKE_CURRENT_BINARY_DIR}/interrogatedb_module.cxx"
    ${P3INTERROGATEDB_IGATE}

  DEPENDS host_interrogate ${P3INTERROGATEDB_IGATE}
  COMMENT "Interrogating interrogatedb")

add_python_target(panda3d.interrogatedb
  "${CMAKE_CURRENT_BINARY_DIR}/interrogatedb_module.cxx")
target_link_libraries(panda3d.interrogatedb p3interrogatedb)
