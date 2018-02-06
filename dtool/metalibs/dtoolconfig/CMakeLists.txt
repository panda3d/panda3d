add_definitions(-DBUILDING_DTOOLCONFIG)

if(HAVE_PYTHON)
  add_library(p3dtoolconfig dtoolconfig.cxx pydtool.cxx)
else()
  add_library(p3dtoolconfig dtoolconfig.cxx)
endif()

target_use_packages(p3dtoolconfig PYTHON)
target_link_libraries(p3dtoolconfig p3prc p3dconfig p3interrogatedb)
set_target_properties(p3dtoolconfig PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/panda3d"
  PREFIX ""
  OUTPUT_NAME "interrogatedb"
)

if(HAVE_PYTHON)
  install(TARGETS p3dtoolconfig DESTINATION "${PYTHON_ARCH_INSTALL_DIR}/panda3d")
else()
  install(TARGETS p3dtoolconfig DESTINATION lib)
endif()
