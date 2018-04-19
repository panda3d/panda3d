# Filename: BuildMetalib.cmake
#
# Description: This file contains macros to build Panda3D's "metalibs" - these
#   are special libraries that contain no unique code themselves and are
#   instead just an agglomeration of the various component libraries that get
#   linked into them. A library of libraries - a "metalibrary."

#
# Function: target_link_libraries(...)
#
# Overrides CMake's target_link_libraries() to support "linking" object
# libraries. This is a partial reimplementation of CMake commit dc38970f83,
# which as of this writing has not yet landed in any release.
#
function(target_link_libraries target)
  get_target_property(target_type "${target}" TYPE)
  if(NOT target_type STREQUAL "OBJECT_LIBRARY")
    _target_link_libraries("${target}" ${ARGN})
    return()
  endif()

  foreach(library ${ARGN})
    # This is a quick and dirty regex to tell targets apart from other stuff.
    # It just checks if it's alphanumeric and starts with p3/panda.
    if(library MATCHES "^(p3|panda)[A-Za-z0-9]*$")
      # We need to add "library"'s include directories to "target"
      # (and transitively to INTERFACE_INCLUDE_DIRECTORIES so further
      # dependencies will work)
      set(include_directories "$<TARGET_PROPERTY:${library},INTERFACE_INCLUDE_DIRECTORIES>")
      set_property(TARGET "${target}" APPEND PROPERTY INCLUDE_DIRECTORIES "${include_directories}")
      set_property(TARGET "${target}" APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${include_directories}")

      # Also build with the same BUILDING_ macros, because these will all end
      # up in the same library.
      set(compile_definitions "$<$<BOOL:$<TARGET_PROPERTY:${library},IS_COMPONENT>>:$<TARGET_PROPERTY:${library},COMPILE_DEFINITIONS>>")
      set_property(TARGET "${target}" APPEND PROPERTY COMPILE_DEFINITIONS "${compile_definitions}")

      # Libraries are only linked transitively if they aren't components.
      # Unfortunately, it seems like INTERFACE_LINK_LIBRARIES can't have
      # generator expressions on an object library(?) so we resort to taking
      # care of this at configuration time.
      if(TARGET "${library}")
        get_target_property(is_component "${library}" IS_COMPONENT)
        if(NOT is_component)
          set_property(TARGET "${target}" APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${library}")
        endif()
      endif()
    else()
      # This is a file path to an out-of-tree library - this needs to be
      # recorded so that the metalib can link them. (They aren't needed at
      # all for the object libraries themselves, so they don't have to work
      # transitively.)
      set_property(TARGET "${target}" APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${library}")
    endif()
  endforeach()

endfunction(target_link_libraries)

#
# Function: add_component_library(target [SYMBOL building_symbol] [SOURCES])
#
# Used very similarly to add_library. You can specify a symbol with SYMBOL,
# which works like CMake's own DEFINE_SYMBOL property: it's defined when
# building the library, but not when building something that links against the
# library.
#
# Note that this function gets to decide whether the component library is
# OBJECT or SHARED, and whether the library is installed or not. Also, as
# a rule, component libraries may only be linked by other component libraries
# in the same metalib - outside of the metalib, you must link the metalib
# itself.
#
function(add_component_library target_name)
  set(sources)
  unset(symbol)

  set(symbol_keyword OFF)
  foreach(source ${ARGN})
    if(source STREQUAL "SYMBOL")
      set(symbol_keyword ON)
    elseif(symbol_keyword)
      set(symbol_keyword OFF)
      set(symbol "${source}")
    else()
      list(APPEND sources "${source}")
    endif()
  endforeach()


  if(BUILD_METALIBS)
    add_library("${target_name}" OBJECT ${sources})
  else()
    add_library("${target_name}" ${sources})
  endif()

  set_target_properties("${target_name}" PROPERTIES IS_COMPONENT ON)
  if(symbol)
    # ... DEFINE_SYMBOL is apparently not respected for object libraries?
    set_property(TARGET "${target_name}" APPEND PROPERTY COMPILE_DEFINITIONS "${symbol}")
  endif()
  if(BUILD_METALIBS)
    # Apparently neither is CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE?
    set_property(TARGET "${target_name}" PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}")

    # If we're building dynamic libraries, the object library needs to be -fPIC
    if(BUILD_SHARED_LIBS)
      set_property(TARGET "${target_name}" PROPERTY POSITION_INDEPENDENT_CODE ON)
    endif()
  endif()

endfunction(add_component_library)

#
# Function: add_metalib(target [source1 source2] [COMPONENTS component1 ...])
#
# This is add_library, but for metalibs.
#
function(add_metalib target_name)
  set(components_keyword OFF)
  set(components)
  set(sources)
  foreach(arg ${ARGN})
    if(arg STREQUAL "COMPONENTS")
      set(components_keyword ON)
    elseif(components_keyword)
      list(APPEND components "${arg}")
    else()
      list(APPEND sources "${arg}")
    endif()
  endforeach()

  set(defines)
  set(includes)
  set(libs)
  foreach(component ${components})
    get_target_property(is_component "${component}" IS_COMPONENT)
    if(NOT is_component)
      message(FATAL_ERROR
        "Attempted to metalink non-component ${component} into ${target_name}!")
    endif()

    if(BUILD_METALIBS)
      list(APPEND defines "$<TARGET_PROPERTY:${component},COMPILE_DEFINITIONS>")
      list(APPEND includes "$<TARGET_PROPERTY:${component},INTERFACE_INCLUDE_DIRECTORIES>")
      list(APPEND libs "$<TARGET_PROPERTY:${component},INTERFACE_LINK_LIBRARIES>")
      list(APPEND sources "$<TARGET_OBJECTS:${component}>")
    else()
      list(APPEND libs "${component}")
    endif()
  endforeach()

  add_library("${target_name}" ${sources})
  target_compile_definitions("${target_name}" PRIVATE ${defines})
  target_link_libraries("${target_name}" ${libs})
  target_include_directories("${target_name}" PUBLIC ${includes})

endfunction(add_metalib)
