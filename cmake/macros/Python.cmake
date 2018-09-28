# Filename: Python.cmake
#
# Description: This file provides support functions for building/installing
#   Python extension modules and/or pure-Python packages.
#
# Functions:
#   add_python_target(target [source1 [source2 ...]])
#   install_python_package(path [ARCH/LIB])
#

#
# Function: add_python_target(target [source1 [source2 ...]])
# Build the provided source(s) as a Python extension module, linked against the
# Python runtime library.
#
# Note that this also takes care of installation, unlike other target creation
# commands in CMake.
#
function(add_python_target target)
  if(NOT HAVE_PYTHON)
    return()
  endif()

  string(REGEX REPLACE "^.*\\." "" basename "${target}")
  set(sources ${ARGN})

  add_library(${target} ${MODULE_TYPE} ${sources})
  target_link_libraries(${target} PKG::PYTHON)

  if(BUILD_SHARED_LIBS)
    set_target_properties(${target} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/panda3d"
      OUTPUT_NAME "${basename}"
      PREFIX ""
      SUFFIX "${PYTHON_EXTENSION_SUFFIX}")

    install(TARGETS ${target} DESTINATION "${PYTHON_ARCH_INSTALL_DIR}/panda3d")
  else()
    set_target_properties(${target} PROPERTIES
      OUTPUT_NAME "${basename}"
      PREFIX "libpython_panda3d_")

    install(TARGETS ${target} DESTINATION lib)
  endif()

endfunction(add_python_target)

#
# Function: install_python_package(path [ARCH/LIB])
#
# Installs the Python package which was built at `path`.
#
# Note that this handles more than just installation; it will also invoke
# Python's compileall utility to pregenerate .pyc/.pyo files.  This will only
# happen if the Python interpreter is found.
#
# The ARCH or LIB keyword may be used to specify whether this package should be
# installed into Python's architecture-dependent or architecture-independent
# package path.  The default, if unspecified, is LIB.
#
function(install_python_package path)
  if(ARGN STREQUAL "ARCH")
    set(type "ARCH")
  elseif(ARGN STREQUAL "LIB")
    set(type "LIB")
  elseif(ARGN STREQUAL "")
    set(type "LIB")
  else()
    message(FATAL_ERROR "install_python_package got unexpected argument: ${ARGN}")
  endif()

  get_filename_component(package_name "${path}" NAME)
  set(custom_target "bytecompile_${package_name}")

  file(RELATIVE_PATH relpath "${PROJECT_BINARY_DIR}" "${path}")

  if(PYTHON_EXECUTABLE)
    add_custom_target(${custom_target} ALL)
    add_custom_command(
      TARGET ${custom_target}
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
      COMMAND "${PYTHON_EXECUTABLE}" -m compileall -q "${relpath}")
    add_custom_command(
      TARGET ${custom_target}
      WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
      COMMAND "${PYTHON_EXECUTABLE}" -OO -m compileall -q "${relpath}")
  endif()

  set(dir ${PYTHON_${type}_INSTALL_DIR})
  if(dir)
    install(DIRECTORY "${path}" DESTINATION "${dir}"
      FILES_MATCHING REGEX "\\.py[co]?$")
  endif()

endfunction(install_python_package)
