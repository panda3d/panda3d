# Settings for composite builds.  Should be moved to Config.cmake?
set(COMPOSITE_SOURCE_LIMIT "30" CACHE STRING
  "Setting this to a value higher than 1 will enable unity builds, also
known as SCU (single compilation unit).  A high value will speed up the
build dramatically but will be more memory intensive than a low value.")

set(COMPOSITE_SOURCE_EXTENSIONS "cxx;c;mm" CACHE STRING
  "Only files of these extensions will be added to composite files.")

set(COMPOSITE_GENERATOR "${CMAKE_CURRENT_LIST_DIR}/MakeComposite.cmake")

#
# Macro: composite_sources(target sources_var)
# Looks at all the sources, generates _composite#.cxx and modifies the list.
# Usage:
#   set(MY_SOURCES a.cxx b.cxx c.cxx)
#   composite_sources(MY_SOURCES)
#   add_library(my ${MY_SOURCES})
#
#TODO: only supports .cxx files so far, not yet .mm files
# it should probably sort the files by extension (.c, .cxx, .mm) first.
#
function(composite_sources target sources_var)
  # How many sources were specified?
  set(orig_sources ${${sources_var}})
  set(sources ${orig_sources})
  list(LENGTH sources num_sources)

  if(num_sources LESS 2 OR ${COMPOSITE_SOURCE_LIMIT} LESS 2)
    # It's silly to do this for a single source.
    return()
  endif()

  set(composite_files "")
  set(composite_sources "")

  while(num_sources GREATER 0)
    # Pop the first element
    list(GET sources 0 source)
    list(REMOVE_AT sources 0)
    list(LENGTH sources num_sources)

    # Check if we can safely add this to a composite file.
    get_source_file_property(generated "${source}" GENERATED)
    get_source_file_property(is_header "${source}" HEADER_FILE_ONLY)

    if(NOT generated AND NOT is_header)
      # Add it to composite_sources.
      list(APPEND composite_sources ${source})
      list(LENGTH composite_sources num_composite_sources)

      if(num_sources EQUAL 0 OR NOT num_composite_sources LESS ${COMPOSITE_SOURCE_LIMIT})

        # It's pointless to make a composite source from just one file.
        if(num_composite_sources GREATER 1)

          # Figure out the name of our composite file.
          list(LENGTH composite_files index)
          math(EXPR index "1+${index}")
          set(composite_file "${CMAKE_CURRENT_BINARY_DIR}/${target}_composite${index}.cxx")
          list(APPEND composite_files "${composite_file}")

          # Set HEADER_FILE_ONLY to prevent it from showing up in the
          # compiler command, but still show up in the IDE environment.
          set_source_files_properties(${composite_sources} PROPERTIES HEADER_FILE_ONLY ON)

          # We'll interrogate the composite files, so exclude the original sources.
          set_source_files_properties(${composite_sources} PROPERTIES WRAP_EXCLUDE YES)

          # Finally, add the target that generates the composite file.
          add_custom_command(
            OUTPUT "${composite_file}"
            COMMAND ${CMAKE_COMMAND}
              -DCOMPOSITE_FILE="${composite_file}"
              -DCOMPOSITE_SOURCES="${composite_sources}"
              -P "${COMPOSITE_GENERATOR}"
            DEPENDS ${composite_sources})          

          # Reset for the next composite file.
          set(composite_sources "")
        endif()
      endif()
    endif()
  endwhile()

  #set_source_files_properties(${composite_files} PROPERTIES GENERATED YES)

  # The new files are added to the existing files, which means the old files
  # are still there, but they won't be compiled due to the HEADER_FILE_ONLY setting.
  set(${sources_var} ${orig_sources} ${composite_files} PARENT_SCOPE)
endfunction(composite_sources)

#
# Function: add_bison_target(output_cxx input_yxx [DEFINES output_h] [PREFIX prefix])
# Takes .prebuilt files into account if BISON_FOUND is not true.
#
function(add_bison_target output_cxx input_yxx)
  set(arguments "")
  set(outputs "${output_cxx}")
  set(keyword "")

  # Parse the extra arguments to the function.
  foreach(arg ${ARGN})
    if(arg STREQUAL "DEFINES")
      set(keyword "DEFINES")
    elseif(arg STREQUAL "PREFIX")
      set(keyword "PREFIX")

    elseif(keyword STREQUAL "PREFIX")
      set(arguments ${arguments} -p "${arg}")
    elseif(keyword STREQUAL "DEFINES")
      set(arguments ${arguments} --defines="${arg}")
      list(APPEND outputs "${arg}")

    else()
      message(SEND_ERROR "Unexpected argument ${arg} to add_bison_target")
    endif()
  endforeach()

  if(keyword STREQUAL arg AND NOT keyword STREQUAL "")
    message(SEND_ERROR "Expected argument after ${keyword}")
  endif()

  if(HAVE_BISON)
    get_source_file_property(input_yxx "${input_yxx}" LOCATION)

    # If we have bison, we can of course just run it.
    add_custom_command(
      OUTPUT ${outputs}
      COMMAND ${BISON_EXECUTABLE}
        -o "${output_cxx}" ${arguments}
        "${input_yxx}"
      MAIN_DEPENDENCY "${input_yxx}"
    )

  else()
    # Look for prebuilt versions of the outputs.
    set(commands "")
    set(depends "")

    foreach(output ${outputs})
      set(prebuilt_file "${output}.prebuilt")
      get_filename_component(prebuilt_file "${prebuilt_file}" ABSOLUTE)

      if(NOT EXISTS "${prebuilt_file}")
        message(SEND_ERROR "Bison was not found and ${prebuilt_file} does not exist!")
      endif()

      list(APPEND depends "${prebuilt_file}")
      set(commands ${commands} COMMAND ${CMAKE_COMMAND} -E copy ${prebuilt_file} ${output})
    endforeach()

    add_custom_command(
      OUTPUT ${outputs}
      ${commands}
      DEPENDS ${depends}
    )
  endif()
endfunction(add_bison_target)


# Emulate CMake 2.8.11's CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE behavior.
if(CMAKE_VERSION VERSION_LESS 2.8.11)
  # Replace some built-in functions in order to extend their functionality.
  function(add_library target)
    _add_library(${target} ${ARGN})
    set_target_properties("${target}" PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}")
  endfunction()

  function(add_executable target)
    _add_executable(${target} ${ARGN})
    set_target_properties("${target}" PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR};${CMAKE_CURRENT_BINARY_DIR}")
  endfunction()

  function(target_link_libraries target)
    set(interface_dirs)
    get_target_property(target_interface_dirs "${target}" INTERFACE_INCLUDE_DIRECTORIES)

    foreach(lib ${ARGN})
      get_target_property(lib_interface_dirs "${lib}" INTERFACE_INCLUDE_DIRECTORIES)

      if(lib_interface_dirs)
        list(APPEND interface_dirs ${lib_interface_dirs})
      endif()
    endforeach()

    list(REMOVE_DUPLICATES interface_dirs)

    #NB. target_include_directories is new in 2.8.8.
    #target_include_directories("${target}" ${interface_dirs})
    include_directories(${interface_dirs})

    # Update this target's interface inc dirs.
    set_target_properties("${target}" PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${target_interface_dirs};${interface_dirs}")

    # Call to the built-in function we are overriding.
    _target_link_libraries(${target} ${ARGN})
  endfunction()

else()
  # 2.8.11 supports this natively.
  set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
