# First, dtoolconfig:

set(DTOOLCONFIG_LINK_TARGETS p3prc p3dconfig p3interrogatedb)
add_metalib(p3dtoolconfig dtoolconfig.cxx COMPONENTS ${DTOOLCONFIG_LINK_TARGETS})
install(TARGETS p3dtoolconfig DESTINATION lib)

# Next, panda3d.interrogatedb:

if(HAVE_PYTHON AND INTERROGATE_PYTHON_INTERFACE)
  set(INTERROGATEDB_IGATE
    ../../src/interrogatedb/interrogate_interface.h
    ../../src/interrogatedb/interrogate_request.h
  )

  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/interrogatedb_module.cxx"
    COMMAND interrogate
      -D EXPCL_INTERROGATEDB=
      -nodb -python -promiscuous
      -module panda3d.interrogatedb
      -library interrogatedb
      -string -true-names -do-module
      -srcdir "${CMAKE_CURRENT_SOURCE_DIR}"
      -oc "${CMAKE_CURRENT_BINARY_DIR}/interrogatedb_module.cxx"
      ${INTERROGATEDB_IGATE}
    DEPENDS interrogate ${INTERROGATEDB_IGATE}
    COMMENT "Interrogating interrogatedb"
  )

  add_library(interrogatedb ${MODULE_TYPE}
    "${CMAKE_CURRENT_BINARY_DIR}/interrogatedb_module.cxx")
  target_use_packages(interrogatedb PYTHON)
  target_link_libraries(interrogatedb p3dtoolconfig)

  set_target_properties(interrogatedb PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/panda3d"
    PREFIX ""
    OUTPUT_NAME "interrogatedb"
  )

  install(TARGETS interrogatedb DESTINATION "${PYTHON_ARCH_INSTALL_DIR}/panda3d")
endif()
