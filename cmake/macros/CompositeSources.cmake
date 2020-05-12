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


# Settings for composite builds.  Should be moved to Config.cmake?
set(CMAKE_UNITY_BUILD "ON" CACHE BOOL
  "Enable unity builds; Panda defaults this to on.")

set(CMAKE_UNITY_BUILD_BATCH_SIZE "30" CACHE STRING
  "How many source files to build at a time through the unity build mechanism.
  A high value will speed up the build dramatically but will be more memory
  intensive than a low value.")

set(COMPOSITE_SOURCE_EXTENSIONS ".cxx;.mm;.c" CACHE STRING
  "Only files of these extensions will be composited.")

set(COMPOSITE_SOURCE_EXCLUSIONS "" CACHE STRING
  "A list of targets to skip when compositing sources. This is mainly
desirable for CI builds.")

set(COMPOSITE_GENERATOR "${CMAKE_CURRENT_SOURCE_DIR}/cmake/scripts/MakeComposite.cmake")


# Define composite_sources()
function(composite_sources target sources_var)
  if(NOT CMAKE_VERSION VERSION_LESS "3.16")
    # CMake 3.16+ implements CMAKE_UNITY_BUILD* natively; no need to continue!

    # Actually - <=3.16.2 has difficulty with multi-language support, so only
    # allow .cxx in. Hopefully this can be removed soon.
    foreach(_source ${${sources_var}})
      get_filename_component(_source_ext "${_source}" EXT)
      if(NOT _source_ext STREQUAL ".cxx")
        set_source_files_properties(${_source} PROPERTIES
          SKIP_UNITY_BUILD_INCLUSION YES)
      endif()
    endforeach(_source)

    return()
  endif()

  if(NOT CMAKE_UNITY_BUILD)
    # We've been turned off
    return()
  endif()

  # How many sources were specified?
  set(orig_sources ${${sources_var}})
  set(sources ${orig_sources})
  list(LENGTH sources num_sources)

  # Don't composite if in the list of exclusions, and don't bother compositing
  # with too few sources
  list (FIND COMPOSITE_SOURCE_EXCLUSIONS ${target} _index)
  if(num_sources LESS 2 OR ${CMAKE_UNITY_BUILD_BATCH_SIZE} LESS 2 OR ${_index} GREATER -1)
    return()
  endif()

  # Sort each source file into a list.
  foreach(source ${sources})
    get_filename_component(extension "${source}" EXT)
    get_source_file_property(generated "${source}" GENERATED)
    get_source_file_property(is_header "${source}" HEADER_FILE_ONLY)
    get_source_file_property(skip_compositing "${source}" SKIP_UNITY_BUILD_INCLUSION)

    # Check if we can safely add this to a composite file.
    if(NOT generated AND NOT is_header AND NOT skip_compositing AND
        ";${COMPOSITE_SOURCE_EXTENSIONS};" MATCHES ";${extension};")

      if(NOT DEFINED sources_${extension})
        set(sources_${extension})
      endif()

      # Append it to one of the lists.
      list(APPEND sources_${extension} "${source}")
    endif()
  endforeach(source)

  # Now, put it all into one big list!
  set(sorted_sources)
  foreach(extension ${COMPOSITE_SOURCE_EXTENSIONS})
    if(DEFINED sources_${extension})
      list(APPEND sorted_sources ${sources_${extension}})
    endif()
  endforeach(extension)

  set(composite_files)
  set(composite_sources)

  # Fill in composite_ext so we can kick off the loop.
  list(GET sorted_sources 0 first_source)
  get_filename_component(first_source_ext "${first_source}" EXT)
  set(composite_ext ${first_source_ext})

  while(num_sources GREATER 0)
    # Pop the first element and adjust the sorted_sources length accordingly.
    list(GET sorted_sources 0 source)
    list(REMOVE_AT sorted_sources 0)
    list(LENGTH sorted_sources num_sources)

    # Add this file to our composite_sources buffer.
    list(APPEND composite_sources ${source})
    list(LENGTH composite_sources num_composite_sources)

    # Get the next source file's extension, so we can see if we're done with
    # this set of source files.
    if(num_sources GREATER 0)
      list(GET sorted_sources 0 next_source)
      get_filename_component(next_extension "${next_source}" EXT)
    else()
      set(next_extension "")
    endif()

    # Check if this is the point where we should cut the file.
    if(num_sources EQUAL 0 OR NOT num_composite_sources LESS ${CMAKE_UNITY_BUILD_BATCH_SIZE}
       OR NOT composite_ext STREQUAL next_extension)
      # It's pointless to make a composite source from just one file.
      if(num_composite_sources GREATER 1)

        # Figure out the name of our composite file.
        list(LENGTH composite_files index)
        math(EXPR index "1+${index}")
        set(composite_file "${CMAKE_CURRENT_BINARY_DIR}/${target}_composite${index}${composite_ext}")
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
      endif()

      # Reset for the next composite file.
      set(composite_sources "")
      set(composite_ext ${next_extension})
    endif()
  endwhile()

  set_source_files_properties(${composite_files} PROPERTIES GENERATED YES)

  # The new files are added to the existing files, which means the old files
  # are still there, but they won't be compiled due to the HEADER_FILE_ONLY setting.
  set(${sources_var} ${orig_sources} ${composite_files} PARENT_SCOPE)

endfunction(composite_sources)
