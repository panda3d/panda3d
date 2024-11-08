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
# Function: add_python_target(target [EXPORT exp] [COMPONENT comp]
#                                    [source1 [source2 ...]])
# Build the provided source(s) as a Python extension module, linked against the
# Python runtime library.
#
# Note that this also takes care of installation, unlike other target creation
# commands in CMake.  The EXPORT and COMPONENT keywords allow passing the
# corresponding options to install(), but default to "Python" otherwise.
#
function(add_python_target target)
  if(NOT HAVE_PYTHON)
    return()
  endif()

  string(REGEX REPLACE "^.*\\." "" basename "${target}")
  set(sources)
  set(component "Python")
  set(export "Python")
  foreach(arg ${ARGN})
    if(arg STREQUAL "COMPONENT")
      set(keyword "component")

    elseif(arg STREQUAL "EXPORT")
      set(keyword "export")

    elseif(keyword)
      set(${keyword} "${arg}")
      unset(keyword)

    else()
      list(APPEND sources "${arg}")

    endif()
  endforeach(arg)

  string(REGEX REPLACE "\\.[^.]+$" "" namespace "${target}")
  string(REPLACE "." "/" slash_namespace "${namespace}")

  add_library(${target} ${MODULE_TYPE} ${sources})
  target_link_libraries(${target} PKG::PYTHON)
  target_include_directories(${target} PRIVATE "${PROJECT_SOURCE_DIR}/dtool/src/interrogatedb")

  if(BUILD_SHARED_LIBS)
    set(_outdir "${PANDA_OUTPUT_DIR}/${slash_namespace}")

    set_target_properties(${target} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${_outdir}"
      OUTPUT_NAME "${basename}"
      PREFIX ""
      SUFFIX "${PYTHON_EXTENSION_SUFFIX}")

    # This is explained over in CompilerFlags.cmake
    foreach(_config ${CMAKE_CONFIGURATION_TYPES})
      string(TOUPPER "${_config}" _config)
      set_target_properties(${target} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY_${_config} "${_outdir}")
    endforeach(_config)

    if(PYTHON_ARCH_INSTALL_DIR)
      install(TARGETS ${target} EXPORT "${export}" COMPONENT "${component}" DESTINATION "${PYTHON_ARCH_INSTALL_DIR}/${slash_namespace}")
    endif()

  else()
    set_target_properties(${target} PROPERTIES
      OUTPUT_NAME "${basename}"
      PREFIX "libpy.${namespace}.")

    install(TARGETS ${target} EXPORT "${export}" COMPONENT "${component}" DESTINATION ${CMAKE_INSTALL_LIBDIR})

  endif()

endfunction(add_python_target)

#
# Function: install_python_package(name [SOURCE path] [ARCH/LIB] [COMPONENT component])
#
# Installs the Python package `name` (which may have its source at `path`).
#
# The package is copied to (or created in) the build directory so that the user
# may import it before the install step.
#
# Note that this handles more than just installation; it will also invoke
# Python's compileall utility to pregenerate .pyc/.pyo files.  This will only
# happen if the Python interpreter is found.
#
# The ARCH or LIB keyword may be used to specify whether this package should be
# installed into Python's architecture-dependent or architecture-independent
# package path.  The default, if unspecified, is LIB.
#
# The COMPONENT keyword overrides the install component (see CMake's
# documentation for more information on what this does).  The default is
# "Python".
#
function(install_python_package package_name)
  set(type "LIB")
  unset(keyword)
  set(component "Python")
  unset(src_path)
  foreach(arg ${ARGN})
    if(arg STREQUAL "ARCH")
      set(type "ARCH")

    elseif(arg STREQUAL "LIB")
      set(type "LIB")

    elseif(arg STREQUAL "COMPONENT")
      set(keyword "${arg}")

    elseif(keyword STREQUAL "COMPONENT")
      set(component "${arg}")
      unset(keyword)

    elseif(arg STREQUAL "SOURCE")
      set(keyword "${arg}")

    elseif(keyword STREQUAL "SOURCE")
      set(src_path "${arg}")
      unset(keyword)

    else()
      message(FATAL_ERROR "install_python_package got unexpected argument: ${ARGN}")

    endif()
  endforeach(arg)

  if(NOT DEFINED src_path AND type STREQUAL "ARCH" AND WIN32 AND NOT CYGWIN)
    # Win32 needs a special fixup so the DLLs in "bin" can be on the path;
    # let's set src_path to the directory containing our fixup __init__.py
    set(src_path "${CMAKE_SOURCE_DIR}/cmake/templates/win32_python")
  endif()

  set(path "${PANDA_OUTPUT_DIR}/${package_name}")

  set(args -D "OUTPUT_DIR=${path}")
  if(src_path)
    list(APPEND args -D "SOURCE_DIR=${src_path}")
  endif()
  if(PYTHON_EXECUTABLE)
    list(APPEND args -D "PYTHON_EXECUTABLES=${PYTHON_EXECUTABLE}")
  endif()
  add_custom_target(${package_name} ALL
    COMMAND ${CMAKE_COMMAND}
      ${args}
      -P "${CMAKE_SOURCE_DIR}/cmake/scripts/CopyPython.cmake")

  set(dir "${PYTHON_${type}_INSTALL_DIR}")
  if(dir)
    install(DIRECTORY "${path}" DESTINATION "${dir}"
      COMPONENT "${component}"
      FILES_MATCHING REGEX "\\.py[co]?$")
  endif()

endfunction(install_python_package)
