# Filename: CompositeSources.cmake
# Description: This file defines the function composite_sources which looks at
#   a provided list of sources, generates _compositeN.cxx, and appends the
#   composites to the list. The original files in the list are marked as headers
#   so that they will be available in an IDE, but not compiled at build time.
#
# Usage:
#   composite_sources(target source_var)
#
# Example:
#   set(MY_SOURCES a.cxx b.cxx c.cxx)
#   composite_sources(my_lib MY_SOURCES)
#   add_library(my_lib ${MY_SOURCES})
#
# TODO: Only supports .cxx files so far, not yet .mm files
#   It should probably sort the files by extension (.c, .cxx, .mm) first.
#


# Settings for composite builds.  Should be moved to Config.cmake?
set(COMPOSITE_SOURCE_LIMIT "30" CACHE STRING
  "Setting this to a value higher than 1 will enable unity builds, also
known as SCU (single compilation unit).  A high value will speed up the
build dramatically but will be more memory intensive than a low value.")

set(COMPOSITE_SOURCE_EXTENSIONS "cxx;c;mm" CACHE STRING
  "Only files of these extensions will be added to composite files.")

set(COMPOSITE_GENERATOR "${CMAKE_SOURCE_DIR}/cmake/scripts/MakeComposite.cmake")


# Define composite_sources()
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
