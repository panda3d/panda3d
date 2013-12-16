#
# dtool/Interrogate.cmake
#
# This file contains macros and functions that are used to invoke
# interrogate, in order to generate wrappers for Python and/or other
# languages.
#

set(IGATE_FLAGS ${INTERROGATE_OPTIONS} -DCPPPARSER -D__cplusplus -Dvolatile -Dmutable)

if(WIN32)
  list(APPEND IGATE_FLAGS -longlong __int64 -D_X86_ -D__STDC__=1 -DWIN32_VC -D "_declspec(param)=" -D "__declspec(param)=" -D_near -D_far -D__near -D__far -D_WIN32 -D__stdcall -DWIN32)
endif()
if(INTERROGATE_VERBOSE)
  list(APPEND IGATE_FLAGS "-v")
endif()

set(IMOD_FLAGS ${INTERROGATE_MODULE_OPTIONS})

#
# Function: target_interrogate(target [ALL] [source1 [source2 ...]])
# NB. This doesn't actually invoke interrogate, but merely adds the
# sources to the list of scan sources associated with the target.
# Interrogate will be invoked when add_python_module is called.
# If ALL is specified, all of the sources from the associated
# target are added.
#
function(target_interrogate target)
  if(HAVE_PYTHON AND HAVE_INTERROGATE)
    set(sources)
    set(want_all OFF)

    # Find any .N files that would normally be picked up by interrogate.
    # We let CMake add these as dependencies too, to allow rebuilding
    # the wrappers when the .N files have been modified.
    set(deps)
    foreach(arg ${ARGV})
      if(arg STREQUAL "ALL")
        set(want_all ON)
      else()
        list(APPEND sources "${source}")
      endif()
    endforeach()

    # If ALL was specified, pull in all sources from the target.
    if(want_all)
      get_target_property(target_sources "${target}" SOURCES)
      list(APPEND sources ${target_sources})
    endif()

    list(REMOVE_DUPLICATES sources)

    # Go through the sources to determine the full name,
    # and also find out if there are any .N files to pick up.
    foreach(source ${sources})
      get_source_file_property(exclude "${source}" WRAP_EXCLUDE)

      if(NOT exclude)
        get_filename_component(basename "${source}" NAME_WE)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.N")
          list(APPEND deps "${CMAKE_CURRENT_SOURCE_DIR}/${basename}.N")
        endif()

        # Add the full path to the source file itself.
        get_source_file_property(location "${source}" LOCATION)
        list(APPEND deps "${location}")
      endif()
    endforeach(source)

    set_target_properties("${target}" PROPERTIES IGATE_SOURCES "${sources}")
    set_target_properties("${target}" PROPERTIES IGATE_SRCDIR "${CMAKE_CURRENT_SOURCE_DIR}")
    set_target_properties("${target}" PROPERTIES IGATE_DEPS "${deps}")
  endif()
endfunction(target_interrogate)

#
# Function: add_python_module(module [lib1 [lib2 ...]])
# Uses interrogate to create a Python module.
#
function(add_python_module module)
  if(HAVE_PYTHON AND HAVE_INTERROGATE)
    set(targets ${ARGN})
    set(infiles)
    set(sources)

    foreach(target ${targets})
      get_target_property(scansrc "${target}" IGATE_SOURCES)
      get_target_property(srcdir "${target}" IGATE_SRCDIR)
      get_target_property(deps "${target}" IGATE_DEPS)

      add_custom_command(
        OUTPUT "${target}_igate.cxx" "${target}.in"
        COMMAND interrogate
          -od "${target}.in"
          -oc "${target}_igate.cxx"
          -module ${module} -library ${target} ${IGATE_FLAGS}
          -srcdir "${srcdir}"
          -I "${PROJECT_BINARY_DIR}/include"
          -S "${PROJECT_SOURCE_DIR}/dtool/src/parser-inc"
          -S "${PROJECT_BINARY_DIR}/include/parser-inc"
          ${scansrc}
        DEPENDS interrogate ${deps}
      )

      list(APPEND infiles "${target}.in")
      list(APPEND sources "${target}_igate.cxx")
    endforeach(target)

    add_custom_command(
      OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/${module}_module.cxx"
      COMMAND interrogate_module
        -oc "${CMAKE_CURRENT_BINARY_DIR}/${module}_module.cxx"
        -module ${module} -library ${module}
        ${IMOD_FLAGS} ${infiles}
      DEPENDS interrogate_module ${infiles}
    )

    add_library(${module} MODULE "${module}_module.cxx" ${sources})
    target_link_libraries(${module} ${PYTHON_LIBRARIES})
    target_link_libraries(${module} p3interrogatedb)

    set_target_properties(${module} PROPERTIES
      LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/panda3d"
      PREFIX ""
    )
    if(WIN32 AND NOT CYGWIN)
      set_target_properties(${module} PROPERTIES SUFFIX ".pyd")
    endif()
  endif()
endfunction(add_python_module)
