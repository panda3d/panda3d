# Filename: AddFlexTarget.cmake
# Description: This file defines the function add_flex_target which instructs
#   cmake to use flex on an input .lxx file.  If flex is not available on
#   the system, add_flex_target tries to use .prebuilt .cxx files instead.
#
# Usage:
#   add_flex_target(output_cxx input_lxx [DEFINES output_h] [PREFIX prefix])
#

# Define add_flex_target()
function(add_flex_target output_cxx input_lxx)
  set(arguments "")
  set(outputs "${output_cxx}")
  set(keyword "")

  # Parse the extra arguments to the function.
  foreach(arg ${ARGN})
    if(arg STREQUAL "DEFINES")
      set(keyword "DEFINES")

    elseif(arg STREQUAL "PREFIX")
      set(keyword "PREFIX")

    elseif(arg STREQUAL "CASE_INSENSITIVE")
      list(APPEND arguments "-i")

    elseif(keyword STREQUAL "PREFIX")
      list(APPEND arguments "-P${arg}")

    elseif(keyword STREQUAL "DEFINES")
      list(APPEND arguments "--header-file=${arg}")
      list(APPEND outputs "${arg}")

    else()
      message(SEND_ERROR "Unexpected argument ${arg} to add_flex_target")

    endif()
  endforeach()

  if(keyword STREQUAL arg AND NOT keyword STREQUAL "")
    message(SEND_ERROR "Expected argument after ${keyword}")
  endif()

  if(HAVE_FLEX)
    get_source_file_property(input_lxx "${input_lxx}" LOCATION)

    # If we have flex, we can of course just run it.
    add_custom_command(
      OUTPUT ${outputs}
      COMMAND ${FLEX_EXECUTABLE}
        "-o${output_cxx}" ${arguments}
        "${input_lxx}"
      MAIN_DEPENDENCY "${input_lxx}")

  else()
    # Look for prebuilt versions of the outputs.
    set(commands "")
    set(depends "")

    foreach(output ${outputs})
      set(prebuilt_file "${output}.prebuilt")
      get_filename_component(prebuilt_file "${prebuilt_file}" ABSOLUTE)

      if(NOT EXISTS "${prebuilt_file}")
        message(SEND_ERROR "Flex was not found and ${prebuilt_file} does not exist!")
      endif()

      list(APPEND depends "${prebuilt_file}")
      list(APPEND commands COMMAND ${CMAKE_COMMAND} -E copy ${prebuilt_file} ${output})
    endforeach()

    add_custom_command(
      OUTPUT ${outputs}
      ${commands}
      DEPENDS ${depends})
  endif()

  if(MSVC)
    set_source_files_properties(${outputs} PROPERTIES COMPILE_DEFINITIONS YY_NO_UNISTD_H=1)
  endif()
endfunction(add_flex_target)
