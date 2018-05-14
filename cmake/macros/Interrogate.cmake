# Filename: Interrogate.cmake
#
# Description: This file contains macros and functions that are used to invoke
#   interrogate, to generate wrappers for Python and/or other languages.
#
# Functions:
#   target_interrogate(target [ALL] [source1 [source2 ...]])
#   add_python_module(module [lib1 [lib2 ...]])
#

set(IGATE_FLAGS -DCPPPARSER -D__cplusplus -Dvolatile -Dmutable)

# In addition, Interrogate needs to know if this is a 64-bit build:
include(CheckTypeSize)
check_type_size(long CMAKE_SIZEOF_LONG)
if(CMAKE_SIZEOF_LONG EQUAL 8)
  list(APPEND IGATE_FLAGS "-D_LP64")
endif()


# This is a list of regexes that are applied to every filename. If one of the
# regexes matches, that file will not be passed to Interrogate.
set(INTERROGATE_EXCLUDE_REGEXES
  ".*\\.I$"
  ".*\\.N$"
  ".*\\.lxx$"
  ".*\\.yxx$"
  ".*_src\\..*")

if(WIN32)
  list(APPEND IGATE_FLAGS -longlong __int64 -D_X86_ -D__STDC__=1 -DWIN32_VC -D "_declspec(param)=" -D "__declspec(param)=" -D_near -D_far -D__near -D__far -D_WIN32 -D__stdcall -DWIN32)
endif()
if(INTERROGATE_VERBOSE)
  list(APPEND IGATE_FLAGS "-v")
endif()

set(IMOD_FLAGS -python-native)

# This stores the names of every module added to the Interrogate system:
set(ALL_INTERROGATE_MODULES CACHE INTERNAL "Internal variable")

#
# Function: target_interrogate(target [ALL] [source1 [source2 ...]])
# NB. This doesn't actually invoke interrogate, but merely adds the
# sources to the list of scan sources associated with the target.
# Interrogate will be invoked when add_python_module is called.
# If ALL is specified, all of the sources from the associated
# target are added.
#
function(target_interrogate target)
  set(sources)
  set(extensions)
  set(want_all OFF)
  set(extensions_keyword OFF)
  foreach(arg ${ARGN})
    if(arg STREQUAL "ALL")
      set(want_all ON)
    elseif(arg STREQUAL "EXTENSIONS")
      set(extensions_keyword ON)
    elseif(extensions_keyword)
      list(APPEND extensions "${arg}")
    else()
      list(APPEND sources "${arg}")
    endif()
  endforeach()

  # If ALL was specified, pull in all sources from the target.
  if(want_all)
    get_target_property(target_sources "${target}" SOURCES)
    list(APPEND sources ${target_sources})
  endif()

  list(REMOVE_DUPLICATES sources)

  # Now let's get everything's absolute path, so that it can be passed
  # through a property while still preserving the reference.
  set(absolute_sources)
  set(absolute_extensions)
  foreach(source ${sources})
    get_source_file_property(exclude "${source}" WRAP_EXCLUDE)
    if(NOT exclude)
      get_source_file_property(location "${source}" LOCATION)
      list(APPEND absolute_sources ${location})
    endif()
  endforeach(source)
  foreach(extension ${extensions})
    get_source_file_property(location "${extension}" LOCATION)
    list(APPEND absolute_extensions ${location})
  endforeach(extension)

  set_target_properties("${target}" PROPERTIES IGATE_SOURCES
    "${absolute_sources}")
  set_target_properties("${target}" PROPERTIES IGATE_EXTENSIONS
    "${absolute_extensions}")

  # CMake has no property for determining the source directory where the
  # target was originally added. interrogate_sources makes use of this
  # property (if it is set) in order to make all paths on the command-line
  # relative to it, thereby shortening the command-line even more.
  # Since this is not an Interrogate-specific property, it is not named with
  # an IGATE_ prefix.
  set_target_properties("${target}" PROPERTIES TARGET_SRCDIR
    "${CMAKE_CURRENT_SOURCE_DIR}")
endfunction(target_interrogate)

#
# Function: interrogate_sources(target output database language_flags module)
#
# This function actually runs a component-level interrogation against 'target'.
# It generates the outfile.cxx (output) and dbfile.in (database) files, which
# can then be used during the interrogate_module step to produce language
# bindings.
#
# The target must first have had sources selected with target_interrogate.
# Failure to do so will result in an error.
#
function(interrogate_sources target output database language_flags)
  get_target_property(sources "${target}" IGATE_SOURCES)
  get_target_property(extensions "${target}" IGATE_EXTENSIONS)

  if(NOT sources)
    message(FATAL_ERROR
      "Cannot interrogate ${target} unless it's run through target_interrogate first!")
  endif()

  get_target_property(srcdir "${target}" TARGET_SRCDIR)
  if(NOT srcdir)
    # No TARGET_SRCDIR was set, so we'll do everything relative to our
    # current binary dir instead:
    set(srcdir "${CMAKE_CURRENT_BINARY_DIR}")
  endif()

  set(scan_sources)
  set(nfiles)
  foreach(source ${sources})
    get_filename_component(source_basename "${source}" NAME)

    # Only certain sources should actually be scanned by Interrogate. The
    # rest are merely dependencies. This uses the exclusion regex above in
    # order to determine what files are okay:
    set(exclude OFF)
    foreach(regex ${INTERROGATE_EXCLUDE_REGEXES})
      if("${source_basename}" MATCHES "${regex}")
        set(exclude ON)
      endif()
    endforeach(regex)

    if(NOT exclude)
      # This file is to be scanned by Interrogate. In order to avoid
      # cluttering up the command line, we should first make it relative:
      file(RELATIVE_PATH rel_source "${srcdir}" "${source}")
      list(APPEND scan_sources "${rel_source}")

      # Also see if this file has a .N counterpart, which has directives
      # specific for Interrogate. If there is a .N file, we add it as a dep,
      # so that CMake will rerun Interrogate if the .N files are modified:
      get_filename_component(source_path "${source}" PATH)
      get_filename_component(source_name_we "${source}" NAME_WE)
      set(nfile "${source_path}/${source_name_we}.N")
      if(EXISTS "${nfile}")
        list(APPEND nfiles "${nfile}")
      endif()
    endif()
  endforeach(source)

  # Interrogate also needs the include paths, so we'll extract them from the
  # target. These are available via a generator expression.

  # When we read the INTERFACE_INCLUDE_DIRECTORIES property, we need to read it
  # from a target that has the IS_INTERROGATE=1 property.
  # (See PackageConfig.cmake for an explanation why.)
  # The problem is, custom commands are not targets, so we can't put target
  # properties on them. And if you try to use the $<TARGET_PROPERTY:prop>
  # generator expression from the context of a custom command, it'll instead
  # read the property from the most recent actual target. As a workaround for
  # this, we create a fake target with the IS_INTERROGATE property set and pull
  # the INTERFACE_INCLUDE_DIRECTORIES property out through that.
  # I hate it, but such is CMake.
  add_custom_target(${target}_igate_internal)
  set_target_properties(${target}_igate_internal PROPERTIES
    IS_INTERROGATE 1
    INTERFACE_INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:${target},INTERFACE_INCLUDE_DIRECTORIES>")

  # Note, the \t is a workaround for a CMake bug where using a plain space in
  # a JOIN will cause it to be escaped. Tabs are not escaped and will
  # separate correctly.
  set(include_flags "-I$<JOIN:$<TARGET_PROPERTY:${target}_igate_internal,INTERFACE_INCLUDE_DIRECTORIES>,\t-I>")
  # The above must also be included when compiling the resulting _igate.cxx file:
  include_directories("$<TARGET_PROPERTY:${target},INTERFACE_INCLUDE_DIRECTORIES>")

  # Get the compiler definition flags. These must be passed to Interrogate
  # in the same way that they are passed to the compiler so that Interrogate
  # will preprocess each file in the same way.
  set(define_flags)
  get_target_property(target_defines "${target}" INTERFACE_COMPILE_DEFINITIONS)
  if(target_defines)
    foreach(target_define ${target_defines})
      list(APPEND define_flags "-D${target_define}")
      # And add the same definition when we compile the _igate.cxx file:
      add_definitions("-D${target_define}")
    endforeach(target_define)
  endif()
  # If this is a release build that has NDEBUG defined, we need that too:
  string(TOUPPER "${CMAKE_BUILD_TYPE}" build_type)
  if("${CMAKE_CXX_FLAGS_${build_type}}" MATCHES ".*NDEBUG.*")
    list(APPEND define_flags "-DNDEBUG")
  endif()

  add_custom_command(
    OUTPUT "${output}" "${database}"
    COMMAND interrogate
      -oc "${output}"
      -od "${database}"
      -srcdir "${srcdir}"
      -library ${target}
      ${INTERROGATE_OPTIONS}
      ${IGATE_FLAGS}
      ${language_flags}
      ${define_flags}
      -S "${PROJECT_BINARY_DIR}/include"
      -S "${PROJECT_SOURCE_DIR}/dtool/src/parser-inc"
      -S "${PYTHON_INCLUDE_DIRS}"
      ${include_flags}
      ${scan_sources}
      ${extensions}
    DEPENDS interrogate ${sources} ${extensions} ${nfiles}
    COMMENT "Interrogating ${target}"
  )
endfunction(interrogate_sources)

#
# Function: add_python_module(module [lib1 [lib2 ...]] [LINK lib1 ...]
#    [IMPORT mod1 ...])
# Uses interrogate to create a Python module. If the LINK keyword is specified,
# the Python module is linked against the specified libraries instead of those
# listed before. The IMPORT keyword makes the output module import another
# Python module when it's initialized.
#
function(add_python_module module)
  if(HAVE_PYTHON AND INTERROGATE_PYTHON_INTERFACE)
    set(targets)
    set(link_targets)
    set(import_flags)
    set(infiles)
    set(sources)

    set(link_keyword OFF)
    set(import_keyword OFF)
    foreach(arg ${ARGN})
      if(arg STREQUAL "LINK")
        set(link_keyword ON)
        set(import_keyword OFF)
      elseif(arg STREQUAL "IMPORT")
        set(link_keyword OFF)
        set(import_keyword ON)
      elseif(link_keyword)
        list(APPEND link_targets "${arg}")
      elseif(import_keyword)
        list(APPEND import_flags "-import" "${arg}")
      else()
        list(APPEND targets "${arg}")
      endif()
    endforeach(arg)

    if(NOT link_targets)
      set(link_targets ${targets})
    endif()

    foreach(target ${targets})
      interrogate_sources(${target} "${target}_igate.cxx" "${target}.in"
        "-python-native;-module;panda3d.${module}")
      get_target_property(target_extensions "${target}" IGATE_EXTENSIONS)
      list(APPEND infiles "${target}.in")
      list(APPEND sources "${target}_igate.cxx" ${target_extensions})
    endforeach(target)

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${module}_module.cxx"
      COMMAND interrogate_module
        -oc "${CMAKE_CURRENT_BINARY_DIR}/${module}_module.cxx"
        -module ${module} -library ${module}
        ${import_flags}
        ${INTERROGATE_MODULE_OPTIONS}
        ${IMOD_FLAGS} ${infiles}
      DEPENDS interrogate_module ${infiles}
      COMMENT "Generating module ${module}"
    )

    add_library(${module} ${MODULE_TYPE} "${module}_module.cxx" ${sources})
    target_link_libraries(${module}
      ${link_targets} ${PYTHON_LIBRARIES} p3dtool)

    set_target_properties(${module} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/panda3d"
      PREFIX ""
    )
    if(WIN32 AND NOT CYGWIN)
      set_target_properties(${module} PROPERTIES SUFFIX ".pyd")
    endif()

    install(TARGETS ${module} DESTINATION "${PYTHON_ARCH_INSTALL_DIR}/panda3d")

    list(APPEND ALL_INTERROGATE_MODULES "${module}")
    set(ALL_INTERROGATE_MODULES "${ALL_INTERROGATE_MODULES}" CACHE INTERNAL "Internal variable")
  endif()
endfunction(add_python_module)


if(HAVE_PYTHON)
  # We have to create an __init__.py so that Python 2.x can recognize 'panda3d'
  # as a package.
  file(WRITE "${PROJECT_BINARY_DIR}/panda3d/__init__.py" "")

  # The Interrogate path needs to be installed to the architecture-dependent
  # Python directory.
  install(FILES "${PROJECT_BINARY_DIR}/panda3d/__init__.py" DESTINATION "${PYTHON_ARCH_INSTALL_DIR}/panda3d")
endif()
