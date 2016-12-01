# Filename: AutoInclude.cmake
# Description: This file backports the CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE
#   introduced in CMake 2.8.11 to previous versions of cmake, and enables the
#   behavior by default.
#

# Emulate CMake 2.8.11's CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE behavior if
# this version doesn't define CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE.
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
      if(TARGET "${lib}")
        get_target_property(lib_interface_dirs "${lib}" INTERFACE_INCLUDE_DIRECTORIES)

        if(lib_interface_dirs)
          list(APPEND interface_dirs ${lib_interface_dirs})
        endif()
      endif()
    endforeach()

    if(interface_dirs)
      list(REMOVE_DUPLICATES interface_dirs)

      #NB. target_include_directories is new in 2.8.8.
      #target_include_directories("${target}" ${interface_dirs})
      include_directories(${interface_dirs})

      # Update this target's interface inc dirs.
      set_target_properties("${target}" PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${target_interface_dirs};${interface_dirs}")
    endif()

    # Call to the built-in function we are overriding.
    _target_link_libraries(${target} ${ARGN})
  endfunction()

else()
  # 2.8.11 supports this natively.
  set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
