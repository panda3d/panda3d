function(add_library target_name)
  _add_library("${target_name}" ${ARGN})
  set_target_properties("${target_name}" PROPERTIES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
endfunction(add_library)
