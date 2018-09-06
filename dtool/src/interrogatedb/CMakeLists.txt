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
  interrogate_interface.h interrogate_request.h)

set(P3IGATERUNTIME_HEADERS
  extension.h py_compat.h py_panda.h py_wrappers.h)

set(P3INTERROGATEDB_SOURCES
  config_interrogatedb.cxx
  indexRemapper.cxx
  interrogateComponent.cxx interrogateDatabase.cxx
  interrogateElement.cxx interrogateFunction.cxx
  interrogateFunctionWrapper.cxx
  interrogateMakeSeq.cxx
  interrogateManifest.cxx
  interrogateType.cxx interrogate_datafile.cxx
  interrogate_interface.cxx interrogate_request.cxx)

set(P3IGATERUNTIME_SOURCES
  dtool_super_base.cxx
  py_compat.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/py_panda.cxx
  py_wrappers.cxx)

set(P3INTERROGATEDB_IGATE
  interrogate_interface.h
  interrogate_request.h)

composite_sources(p3interrogatedb P3INTERROGATEDB_SOURCES)

add_component_library(p3interrogatedb NOINIT SYMBOL BUILDING_INTERROGATEDB
  ${P3INTERROGATEDB_HEADERS} ${P3INTERROGATEDB_SOURCES})
target_link_libraries(p3interrogatedb p3dconfig)

if(NOT BUILD_METALIBS)
  install(TARGETS p3interrogatedb DESTINATION lib RUNTIME DESTINATION bin)
endif()
install(FILES ${P3INTERROGATEDB_HEADERS} DESTINATION include/panda3d)

# Now we need to build the Interrogate runtime code; this should eventually be
# split out into its own subdirectory.

# HACK: igateruntime doesn't have its own BUILDING_IGATERUNTIME symbol yet;
# it's still using BUILDING_INTERROGATEDB.

# py_panda.cxx can't rely on anything from interrogatedb, so:
file(READ ${CMAKE_CURRENT_SOURCE_DIR}/py_panda.cxx _py_panda)
string(REGEX REPLACE "interrogatedb" "prc" _py_panda "${_py_panda}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/py_panda.cxx "${_py_panda}")

add_library(p3igateruntime
  ${P3IGATERUNTIME_HEADERS} ${P3IGATERUNTIME_SOURCES})
set_target_properties(p3igateruntime PROPERTIES
  DEFINE_SYMBOL BUILDING_INTERROGATEDB) # HACK
target_link_libraries(p3igateruntime p3dtoolconfig)
target_use_packages(p3igateruntime PYTHON)

install(TARGETS p3igateruntime DESTINATION lib RUNTIME DESTINATION bin)
install(FILES ${P3IGATERUNTIME_HEADERS} DESTINATION include/panda3d)

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
  COMMENT "Interrogating interrogatedb"
)

add_python_target(panda3d.interrogatedb
  "${CMAKE_CURRENT_BINARY_DIR}/interrogatedb_module.cxx")
target_link_libraries(panda3d.interrogatedb p3igateruntime)
