# Filename: AddBisonTarget.cmake
# Description: This file defines the function add_bison_target which instructs
#   cmake to use bison on an input .yxx file.  If bison is not available on
#   the system, add_bison_target tries to use .prebuilt .cxx files instead.
#
# Usage:
#   add_bison_target(output_cxx input_yxx [DEFINES output_h] [PREFIX prefix])
#

# Define add_bison_target()
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
      list(APPEND arguments -p "${arg}")

    elseif(keyword STREQUAL "DEFINES")
      list(APPEND arguments --defines="${arg}")
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
      list(APPEND commands COMMAND ${CMAKE_COMMAND} -E copy ${prebuilt_file} ${output})
    endforeach()

    add_custom_command(
      OUTPUT ${outputs}
      ${commands}
      DEPENDS ${depends})
  endif()
endfunction(add_bison_target)
