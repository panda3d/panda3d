# First, dtoolconfig:

set(DTOOLCONFIG_LINK_TARGETS p3prc p3dconfig p3interrogatedb)
add_metalib(p3dtoolconfig dtoolconfig.cxx COMPONENTS ${DTOOLCONFIG_LINK_TARGETS})
install(TARGETS p3dtoolconfig DESTINATION lib)

# Next, panda3d.interrogatedb:

if(BUILD_SHARED_LIBS)
  set(libtype MODULE)
else()
  set(libtype STATIC)
endif()

if(HAVE_PYTHON)
  add_library(interrogatedb ${libtype} pydtool.cxx)
  target_use_packages(interrogatedb PYTHON)
  target_link_libraries(interrogatedb p3dtoolconfig)

  set_target_properties(interrogatedb PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/panda3d"
    PREFIX ""
    OUTPUT_NAME "interrogatedb"
  )

  install(TARGETS interrogatedb DESTINATION "${PYTHON_ARCH_INSTALL_DIR}/panda3d")
endif()
