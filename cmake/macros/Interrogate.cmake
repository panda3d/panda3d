# Filename: Interrogate.cmake
#
# Description: This file contains macros and functions that are used to invoke
#   interrogate, to generate wrappers for Python and/or other languages.
#
# Functions:
#   target_interrogate(target [ALL] [source1 [source2 ...]])
#   add_python_module(module [lib1 [lib2 ...]])
#   add_python_target(target [source1 [source2 ...]])
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
  ".*\\.c$"
  ".*\\.lxx$"
  ".*\\.yxx$"
  ".*_src\\..*"
)

if(WIN32)
  list(APPEND IGATE_FLAGS -D_X86_ -D__STDC__=1 -D "_declspec(param)=" -D "__declspec(param)=" -D_near -D_far -D__near -D__far -D_WIN32 -D__stdcall)
endif()
if(MSVC_VERSION)
  list(APPEND IGATE_FLAGS "-D_MSC_VER=${MSVC_VERSION}")
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
  foreach(source ${sources})
    get_source_file_property(exclude "${source}" WRAP_EXCLUDE)
    if(NOT exclude)
      get_source_file_property(location "${source}" LOCATION)
      list(APPEND absolute_sources ${location})
    endif()
  endforeach(source)

  set(absolute_extensions)
  foreach(extension ${extensions})
    get_source_file_property(location "${extension}" LOCATION)
    list(APPEND absolute_extensions ${location})
  endforeach(extension)

  set_target_properties("${target}" PROPERTIES
    IGATE_SOURCES "${absolute_sources}")
  set_target_properties("${target}" PROPERTIES
    IGATE_EXTENSIONS "${absolute_extensions}")

  # CMake has no property for determining the source directory where the
  # target was originally added. interrogate_sources makes use of this
  # property (if it is set) in order to make all paths on the command-line
  # relative to it, thereby shortening the command-line even more.
  # Since this is not an Interrogate-specific property, it is not named with
  # an IGATE_ prefix.
  set_target_properties("${target}" PROPERTIES
    TARGET_SRCDIR "${CMAKE_CURRENT_SOURCE_DIR}")

  # Also store where the build files are kept, so the Interrogate output can go
  # there as well.
  set_target_properties("${target}" PROPERTIES
    TARGET_BINDIR "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_CFG_INTDIR}")

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

  # Also add extensions, in relative-path form
  foreach(extension ${extensions})
    file(RELATIVE_PATH rel_extension "${srcdir}" "${extension}")
    list(APPEND scan_sources "${rel_extension}")
  endforeach(extension)

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

  # Get the compiler definition flags. These must be passed to Interrogate
  # in the same way that they are passed to the compiler so that Interrogate
  # will preprocess each file in the same way.
  set(_compile_defs "$<TARGET_PROPERTY:${target},COMPILE_DEFINITIONS>")
  if(NOT CMAKE_HOST_WIN32)
    # Win32's command-line parser doesn't understand "'"
    # that's fine, it also ignores '"'
    set(_q "'")
  endif()
  set(_compile_defs_flags "-D${_q}$<JOIN:${_compile_defs},${_q}\t-D${_q}>${_q}")
  # We may have just ended up with -D'' if there are no flags; filter that
  set(define_flags
    "$<$<NOT:$<STREQUAL:${_compile_defs_flags},-D${_q}${_q}>>:${_compile_defs_flags}>")

  # Some of the definitions may be specified using -D flags in the global
  # CXX_FLAGS variables; parse those out (this also picks up NDEBUG)
  set(_configs ${CMAKE_CONFIGURATION_TYPES} ${CMAKE_BUILD_TYPE} "<ALL>")
  list(REMOVE_DUPLICATES _configs)
  foreach(_config ${_configs})
    if(_config STREQUAL "<ALL>")
      set(flags "${CMAKE_CXX_FLAGS}")
    else()
      string(TOUPPER "${_config}" _CONFIG)
      set(flags "${CMAKE_CXX_FLAGS_${_CONFIG}}")
    endif()

    # Convert "/D define1" and "-Ddefine2" flags, interspersed with other
    # compiler nonsense, into a basic "-Ddefine1 -Ddefine2" string
    string(REGEX MATCHALL "[/-]D[ \t]*[A-Za-z0-9_]+" igate_flags "${flags}")
    string(REPLACE ";" " " igate_flags "${igate_flags}")
    string(REPLACE "/D" "-D" igate_flags "${igate_flags}")

    if(_config STREQUAL "<ALL>")
      list(APPEND define_flags "${igate_flags}")
    else()
      list(APPEND define_flags "$<$<CONFIG:${_config}>:${igate_flags}>")
    endif()
  endforeach(_config)

  get_filename_component(output_directory "${output}" DIRECTORY)
  get_filename_component(database_directory "${database}" DIRECTORY)

  add_custom_command(
    OUTPUT "${output}" "${database}"
    COMMAND ${CMAKE_COMMAND} -E
      make_directory "${output_directory}"
    COMMAND ${CMAKE_COMMAND} -E
      make_directory "${database_directory}"
    COMMAND interrogate
      -oc "${output}"
      -od "${database}"
      -srcdir "${srcdir}"
      -library ${target}
      ${INTERROGATE_OPTIONS}
      ${IGATE_FLAGS}
      ${language_flags}
      ${define_flags}
      -S "${PROJECT_SOURCE_DIR}/dtool/src/interrogatedb"
      -S "${PROJECT_SOURCE_DIR}/dtool/src/parser-inc"
      -S "${PYTHON_INCLUDE_DIRS}"
      ${include_flags}
      ${scan_sources}

    DEPENDS interrogate ${sources} ${extensions} ${nfiles}
    COMMENT "Interrogating ${target}")

  # Propagate the target's compile definitions to the output file
  set_source_files_properties("${output}" PROPERTIES
    COMPILE_DEFINITIONS "$<TARGET_PROPERTY:${target},INTERFACE_COMPILE_DEFINITIONS>")

endfunction(interrogate_sources)

#
# Function: add_python_module(module [lib1 [lib2 ...]] [LINK lib1 ...]
#    [IMPORT mod1 ...] [INIT func1 ...])
# Uses interrogate to create a Python module. If the LINK keyword is specified,
# the Python module is linked against the specified libraries instead of those
# listed before. The IMPORT keyword makes the output module import another
# Python module when it's initialized.
#
function(add_python_module module)
  if(NOT INTERROGATE_PYTHON_INTERFACE)
    return()
  endif()

  set(targets)
  set(component "Python")
  set(link_targets)
  set(import_flags)
  set(infiles_rel)
  set(infiles_abs)
  set(sources_abs)
  set(extensions)

  set(keyword)
  foreach(arg ${ARGN})
    if(arg STREQUAL "LINK" OR arg STREQUAL "IMPORT" OR arg STREQUAL "INIT" OR arg STREQUAL "COMPONENT")
      set(keyword "${arg}")

    elseif(keyword STREQUAL "LINK")
      list(APPEND link_targets "${arg}")
      set(keyword)

    elseif(keyword STREQUAL "IMPORT")
      list(APPEND import_flags "-import" "${arg}")
      set(keyword)

    elseif(keyword STREQUAL "INIT")
      list(APPEND import_flags "-init" "${arg}")
      set(keyword)

    elseif(keyword STREQUAL "COMPONENT")
      set(component "${arg}")
      set(keyword)

    else()
      list(APPEND targets "${arg}")

    endif()
  endforeach(arg)

  if(NOT link_targets)
    set(link_targets ${targets})
  endif()

  string(REGEX REPLACE "^.*\\." "" modname "${module}")

  foreach(target ${targets})
    get_target_property(workdir_abs "${target}" TARGET_BINDIR)
    if(NOT workdir_abs)
      # No TARGET_BINDIR was set, so we'll just use our current directory:
      set(workdir_abs "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_CFG_INTDIR}")
    endif()
    # Keep command lines short
    file(RELATIVE_PATH workdir_rel "${CMAKE_CURRENT_BINARY_DIR}" "${workdir_abs}")

    get_target_property(target_module "${target}" IGATE_MODULE)
    if(NOT target_module)
      set(target_module "${module}")
    endif()

    interrogate_sources(${target}
      "${workdir_abs}/${target}_igate.cxx"
      "${workdir_abs}/${target}.in"
      "-python-native;-module;${target_module}")

    get_target_property(target_extensions "${target}" IGATE_EXTENSIONS)
    list(APPEND infiles_rel "${workdir_rel}/${target}.in")
    list(APPEND infiles_abs "${workdir_abs}/${target}.in")
    list(APPEND sources_abs "${workdir_abs}/${target}_igate.cxx")
    list(APPEND extensions ${target_extensions})
  endforeach(target)

  add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_CFG_INTDIR}/${module}_module.cxx"
    COMMAND ${CMAKE_COMMAND} -E
      make_directory "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_CFG_INTDIR}"
    COMMAND interrogate_module
      -oc "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_CFG_INTDIR}/${module}_module.cxx"
      -module ${module} -library ${modname}
      ${import_flags}
      ${INTERROGATE_MODULE_OPTIONS}
      ${IMOD_FLAGS} ${infiles_rel}
    DEPENDS interrogate_module ${infiles_abs}
    COMMENT "Generating module ${module}")

  # CMake chokes on ${CMAKE_CFG_INTDIR} in source paths when unity builds are
  # enabled. The easiest way out of this is to skip unity for those paths.
  # Since generated Interrogate .cxx files are pretty big already, this doesn't
  # really inconvenience us at all.
  set_source_files_properties(
    "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_CFG_INTDIR}/${module}_module.cxx"
    ${sources_abs} PROPERTIES
    SKIP_UNITY_BUILD_INCLUSION YES)

  add_python_target(${module} COMPONENT "${component}" EXPORT "${component}"
    "${CMAKE_CURRENT_BINARY_DIR}/${PANDA_CFG_INTDIR}/${module}_module.cxx"
    ${sources_abs} ${extensions})
  target_link_libraries(${module} ${link_targets})

  if(CMAKE_VERSION VERSION_LESS "3.11")
    # CMake <3.11 doesn't allow generator expressions on source files, so we
    # need to copy them to our target, which does allow them.

    foreach(source ${sources_abs})
      get_source_file_property(compile_definitions "${source}" COMPILE_DEFINITIONS)
      if(compile_definitions)
        set_property(TARGET ${module} APPEND PROPERTY
          COMPILE_DEFINITIONS ${compile_definitions})

        set_source_files_properties("${source}" PROPERTIES COMPILE_DEFINITIONS "")
      endif()
    endforeach(source)
  endif()

  list(APPEND ALL_INTERROGATE_MODULES "${module}")
  set(ALL_INTERROGATE_MODULES "${ALL_INTERROGATE_MODULES}" CACHE INTERNAL "Internal variable")
endfunction(add_python_module)
