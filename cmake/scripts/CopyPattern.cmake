# Filename: CopyPattern.cmake
#
# Description: This is a standalone version of CMake's file(COPY) command so we
#              can use all of its features during build-time instead of
#              config-time.
#
# Usage:
#   This script is invoked via add_custom_target, like this:
#   cmake -D SOURCE=[source directory]
#         -D DESTINATION=[destination directory]
#         -D FILES_MATCHING="[globbing patterns passed to file(COPY)]"
#         -P CopyPattern.cmake
if(NOT DEFINED SOURCE OR NOT DEFINED DESTINATION)
    message(SEND_ERROR "CopyPattern.cmake requires SOURCE and DESTINATION to be
defined.")
endif()

if(DEFINED FILES_MATCHING)
  separate_arguments(FILES_MATCHING UNIX_COMMAND ${FILES_MATCHING})

  file(COPY "${SOURCE}"
       DESTINATION "${DESTINATION}"
       FILES_MATCHING ${FILES_MATCHING})
else()
  file(COPY "${SOURCE}"
       DESTINATION "${DESTINATION}")
endif()
