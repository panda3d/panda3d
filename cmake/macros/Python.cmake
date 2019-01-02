# Filename: Python.cmake
#
# Description: This file provides support functions for building/installing
#   Python extension modules and/or pure-Python packages.
#
# Functions:
#   add_python_target(target [source1 [source2 ...]])
#   install_python_package(path [ARCH/LIB])
#   ensure_python_init(path [ARCH] [ROOT] [OVERWRITE])
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

  if(BUILD_SHARED_LIBS)
    set_target_properties(${target} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/${slash_namespace}"
      OUTPUT_NAME "${basename}"
      PREFIX ""
      SUFFIX "${PYTHON_EXTENSION_SUFFIX}")

    if(PYTHON_ARCH_INSTALL_DIR)
      install(TARGETS ${target} EXPORT "${export}" COMPONENT "${component}" DESTINATION "${PYTHON_ARCH_INSTALL_DIR}/${slash_namespace}")
    endif()
  else()
    set_target_properties(${target} PROPERTIES
      OUTPUT_NAME "${basename}"
      PREFIX "libpy.${namespace}.")

    install(TARGETS ${target} EXPORT "${export}" COMPONENT "${component}" DESTINATION lib)
  endif()

  set(keywords OVERWRITE ARCH)
  if(NOT slash_namespace MATCHES ".*/.*")
    list(APPEND keywords ROOT)
  endif()
  ensure_python_init("${PROJECT_BINARY_DIR}/${slash_namespace}" ${keywords})

endfunction(add_python_target)

#
# Function: install_python_package(path [ARCH/LIB] [COMPONENT component])
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
# The COMPONENT keyword overrides the install component (see CMake's
# documentation for more information on what this does).  The default is
# "Python".
#
function(install_python_package path)
  set(type "LIB")
  set(component "Python")
  set(component_keyword OFF)
  foreach(arg ${ARGN})
    if(arg STREQUAL "ARCH")
      set(type "ARCH")
    elseif(arg STREQUAL "LIB")
      set(type "LIB")
    elseif(arg STREQUAL "COMPONENT")
      set(component_keyword ON)
    elseif(component_keyword)
      set(component "${arg}")
      set(component_keyword OFF)
    else()
      message(FATAL_ERROR "install_python_package got unexpected argument: ${ARGN}")
    endif()
  endforeach(arg)

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

  ensure_python_init("${path}")

  set(dir ${PYTHON_${type}_INSTALL_DIR})
  if(dir)
    install(DIRECTORY "${path}" DESTINATION "${dir}"
      COMPONENT "${component}"
      FILES_MATCHING REGEX "\\.py[co]?$")
  endif()

endfunction(install_python_package)

#
# Function: ensure_python_init(path [ARCH] [ROOT] [OVERWRITE])
#
# Makes sure that the directory - at `path` - contains a file named
# '__init__.py', which is necessary for Python to recognize the directory as a
# package.
#
# ARCH, if specified, means that this is a binary package, and the build tree
# might contain configuration-specific subdirectories.  The __init__.py will be
# generated with a function that ensures that the appropriate configuration
# subdirectory is in the path.
#
# ROOT, if specified, means that the directory may sit directly adjacent to a
# 'bin' directory, which should be added to the DLL search path on Windows.
#
# OVERWRITE causes the __init__.py file to be overwritten if one is already
# present.
#
function(ensure_python_init path)
  set(arch OFF)
  set(root OFF)
  set(overwrite OFF)

  foreach(arg ${ARGN})
    if(arg STREQUAL "ARCH")
      set(arch ON)
    elseif(arg STREQUAL "ROOT")
      set(root ON)
    elseif(arg STREQUAL "OVERWRITE")
      set(overwrite ON)
    else()
      message(FATAL_ERROR "ensure_python_init got unexpected argument: ${arg}")
    endif()
  endforeach(arg)

  set(init_filename "${path}/__init__.py")
  if(EXISTS "${init_filename}" AND NOT overwrite)
    return()
  endif()

  file(WRITE "${init_filename}" "")

  if(arch AND NOT "${CMAKE_CFG_INTDIR}" STREQUAL ".")
    # ARCH set, and this is a multi-configuration generator

    set(configs "${CMAKE_CONFIGURATION_TYPES}")

    # Debug should be at the end (highest preference)
    list(REMOVE_ITEM configs "Debug")
    list(APPEND configs "Debug")

    string(REPLACE ";" "', '" configs "${configs}")

    file(APPEND "${init_filename}" "
def _fixup_path():
    try:
        path = __path__[0]
    except (NameError, IndexError):
        return # Not a package, or not on filesystem

    import os
    abspath = os.path.abspath(path)

    newpath = None
    for config in ['${configs}']:
        cfgpath = os.path.join(abspath, config)
        if not os.path.isdir(cfgpath):
            continue

        newpath = cfgpath

        if config.lower() == os.environ.get('CMAKE_CONFIGURATION', '').lower():
            break

    if newpath:
        __path__.insert(0, newpath)

_fixup_path()
del _fixup_path
")
  endif()

  if(root AND WIN32 AND NOT CYGWIN)
    # ROOT set, and this is Windows

    file(APPEND "${init_filename}" "
def _fixup_dlls():
    try:
        path = __path__[0]
    except (NameError, IndexError):
        return # Not a package, or not on filesystem

    import os

    relpath = os.path.relpath(path, __path__[-1])
    dll_path = os.path.abspath(os.path.join(__path__[-1], '../bin', relpath))
    if not os.path.isdir(dll_path):
        return

    os_path = os.environ.get('PATH', '')
    os_path = os_path.split(os.pathsep) if os_path else []
    os_path.insert(0, dll_path)
    os.environ['PATH'] = os.pathsep.join(os_path)

_fixup_dlls()
del _fixup_dlls
")
  endif()

endfunction(ensure_python_init)
