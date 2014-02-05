add_definitions(-DBUILDING_DTOOLCONFIG)

if(HAVE_PYTHON)
  add_library(p3dtoolconfig dtoolconfig.cxx pydtool.cxx)
  target_link_libraries(p3dtoolconfig ${_PYTHON_LIBRARIES})
else()
  add_library(p3dtoolconfig dtoolconfig.cxx)
endif()

target_link_libraries(p3dtoolconfig p3prc p3dconfig p3interrogatedb)

install(TARGETS p3dtoolconfig DESTINATION lib)
