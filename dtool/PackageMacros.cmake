# Filename: ConfigurePackage.cmake
# Author: kestred (30 Nov, 2013)
#
# This modules defines a function which finds and configures libraries
# and packages for Panda3Ds configuration file headers (.h files).
#
# Assumes the file has already been found with find_package().
#

#
# Usage:
# package_option(PYTHON "Enables support for Python" [DEFAULT ON | OFF])
# If no default is given, it is initialized to whether it was found on the system.
#
function(package_option name)
  # Parse the arguments.
  set(command)
  set(default)
  set(cache_string)

  foreach(arg ${ARGN})
    if(command STREQUAL "DEFAULT")
      set(default "${arg}")
      set(command)

    elseif(arg STREQUAL "DEFAULT")
      set(command "DEFAULT")

    else()
      # Yes, a list, ecause semicolons can be in there, and
      # that gets split up into multiple args, so we have to
      # join it back together here.
      list(APPEND cache_string "${arg}")

    endif()
  endforeach()

  if(command STREQUAL "DEFAULT")
    message(SEND_ERROR "DEFAULT in package_option takes an argument")    
  endif()

  # If the default is not set, we set it.
  if(NOT DEFINED default)
    set(default "${${name}_FOUND}")
  endif()

  # If it was set by the user but not found, display an error.
  if(HAVE_${name} AND NOT ${name}_FOUND)
    message(SEND_ERROR "NOT FOUND: ${name}.  Disable HAVE_${name} to continue.")
  endif()

  # Create the option.
  option("HAVE_${name}" "${cache_string}" "${default}")
endfunction()

#
# Useful macro that picks up a package located using find_package
# as dependencies of a target that is going to be built.
# Example use:
# add_library(mylib ${SOURCES})
# target_use_packages(mylib PYTHON PNG)
#
macro(target_use_packages target)
  set(libs ${ARGV})
  list(REMOVE_AT libs 0)

  foreach(lib ${libs})
    if(HAVE_${lib})
      target_include_directories("${target}" PUBLIC "${${lib}_INCLUDE_DIRS};${${lib}_INCLUDE_DIR}")
      if(${lib}_LIBRARIES)
        target_link_libraries("${target}" ${${lib}_LIBRARIES})
      else()
        target_link_libraries("${target}" ${${lib}_LIBRARY})
      endif()
    endif()
  endforeach(lib)
endmacro(target_use_packages)
