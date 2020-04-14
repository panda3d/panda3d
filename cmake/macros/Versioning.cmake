# Filename: Versioning.cmake
#
# Description: Contains an override for add_library to set the
#   VERSION and SOVERSION properties on all shared libraries, automatically, to
#   the project version.
#
# Functions:
#   add_library(...)
#

function(add_library target_name)
  _add_library("${target_name}" ${ARGN})

  get_target_property(type "${target_name}" TYPE)
  if(type STREQUAL "SHARED_LIBRARY")
    set_target_properties("${target_name}" PROPERTIES
      VERSION "${PROJECT_VERSION}"
      SOVERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")
  endif()

endfunction(add_library)
