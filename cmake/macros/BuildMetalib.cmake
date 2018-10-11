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
    if(library MATCHES "^(PKG::|p3|panda)[A-Za-z0-9]*$")
      # We need to add "library"'s include directories to "target"
      # (and transitively to INTERFACE_INCLUDE_DIRECTORIES so further
      # dependencies will work)
      set(include_directories "$<TARGET_PROPERTY:${library},INTERFACE_INCLUDE_DIRECTORIES>")
      set_property(TARGET "${target}" APPEND PROPERTY INCLUDE_DIRECTORIES "${include_directories}")
      set_property(TARGET "${target}" APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${include_directories}")

      # And for INTERFACE_COMPILE_DEFINITIONS as well
      set(compile_definitions "$<TARGET_PROPERTY:${library},INTERFACE_COMPILE_DEFINITIONS>")
      set_property(TARGET "${target}" APPEND PROPERTY COMPILE_DEFINITIONS "${compile_definitions}")

      # Build up some generator expressions for determining whether `library`
      # is a component library or not.
      if(library MATCHES ".*::.*")
        # "::" messes up CMake's genex parser; fortunately, a library whose
        # name contains that is either an interface library or alias, and
        # definitely not a component
        set(is_component 0)
        set(name_of_component "")
        set(name_of_non_component "${library}")
      else()
        set(is_component "$<TARGET_PROPERTY:${library},IS_COMPONENT>")

        # CMake complains if we lookup IS_COMPONENT on an INTERFACE library :(
        set(is_object "$<STREQUAL:$<TARGET_PROPERTY:${library},TYPE>,OBJECT_LIBRARY>")
        set(is_component "$<BOOL:$<${is_object}:${is_component}>>")

        set(name_of_component "$<${is_component}:${library}>")
        set(name_of_non_component "$<$<NOT:${is_component}>:${library}>")
      endif()

      # Libraries are only linked transitively if they aren't components.
      set_property(TARGET "${target}" APPEND PROPERTY
        INTERFACE_LINK_LIBRARIES "${name_of_non_component}")

      # Also build with the same BUILDING_ macros, because these will all end
      # up in the same library.
      set(compile_definitions "$<TARGET_PROPERTY:${library},COMPILE_DEFINITIONS>")
      set_property(TARGET "${target}" APPEND PROPERTY
        COMPILE_DEFINITIONS "$<${is_component}:${compile_definitions}>")
    else()
      # This is a file path to an out-of-tree library - this needs to be
      # recorded so that the metalib can link them. (They aren't needed at
      # all for the object libraries themselves, so they don't have to work
      # transitively.)
      set_property(TARGET "${target}" APPEND PROPERTY INTERFACE_LINK_LIBRARIES "${library}")
    endif()

  endforeach(library)

endfunction(target_link_libraries)

#
# Function: add_component_library(target [SYMBOL building_symbol]
#                                 [SOURCES] [[NOINIT]/[INIT func [header]]])
#
# Used very similarly to add_library.  You can specify a symbol with SYMBOL,
# which works like CMake's own DEFINE_SYMBOL property: it's defined when
# building the library, but not when building something that links against the
# library.
#
# INIT specifies the init function that should be called from a metalib's init
# function when this is added to a metalib.  The header parameter can further
# clarify what header declares this function.  By default, this is
# init_libTARGET and config_TARGET.h, respectively, where TARGET is the
# target name (with 'p3' stripped off, if applicable).  The NOINIT keyword
# suppresses this default.
#
# Note that this function gets to decide whether the component library is
# OBJECT or SHARED, and whether the library is installed or not.  Also, as
# a rule, component libraries may only be linked by other component libraries
# in the same metalib - outside of the metalib, you must link the metalib
# itself.
#
function(add_component_library target_name)
  set(sources)
  unset(symbol)

  if(target_name MATCHES "^p3.*")
    string(SUBSTRING "${target_name}" 2 -1 name_without_prefix)
  else()
    set(name_without_prefix "${target_name}")
  endif()
  set(init_func "init_lib${name_without_prefix}")
  set(init_header "config_${name_without_prefix}.h")

  set(symbol_keyword OFF)
  set(init_keyword 0)
  foreach(source ${ARGN})
    if(source STREQUAL "SYMBOL")
      set(symbol_keyword ON)
      set(init_keyword 0)
    elseif(source STREQUAL "INIT")
      set(symbol_keyword OFF)
      set(init_keyword 2)
    elseif(source STREQUAL "NOINIT")
      set(init_func)
      set(init_header)
    elseif(symbol_keyword)
      set(symbol_keyword OFF)
      set(symbol "${source}")
    elseif(init_keyword EQUAL 2)
      set(init_func "${source}")
      set(init_keyword 1)
    elseif(init_keyword EQUAL 1)
      set(init_header "${source}")
      set(init_keyword 0)
    else()
      list(APPEND sources "${source}")
    endif()
  endforeach()

  if(BUILD_METALIBS)
    # CMake 3.0.2 doesn't like .I/.N/.T files!  We let it know that they're only
    # headers.
    foreach(source ${sources})
      if(source MATCHES "\\.[INT]$")
        set_source_files_properties(${source} PROPERTIES HEADER_FILE_ONLY ON)
      endif()
    endforeach(source)

    add_library("${target_name}" OBJECT ${sources})
  else()
    add_library("${target_name}" ${sources})
  endif()

  set_target_properties("${target_name}" PROPERTIES 
    IS_COMPONENT ON
    INIT_FUNCTION "${init_func}"
    INIT_HEADER "${init_header}")
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
# Function: add_metalib(target [source1 source2] [INIT initfunc [initheader.h]]
#                       [COMPONENTS component1 ...])
#
# This is add_library, but for metalibs.
#
# The INIT keyword can specify an initialization function/header (which will be
# autogenerated by this function) that calls the underlying component libs'
# init functions.
#
function(add_metalib target_name)
  set(components_keyword OFF)
  set(init_keyword 0)
  set(init_func)
  set(init_header "${target_name}.h")
  set(components)
  set(sources)
  foreach(arg ${ARGN})
    if(arg STREQUAL "COMPONENTS")
      set(components_keyword ON)
      set(init_keyword 0)
    elseif(arg STREQUAL "INIT")
      set(init_keyword 2)
      set(components_keyword OFF)
    elseif(components_keyword)
      list(APPEND components "${arg}")
    elseif(init_keyword EQUAL 2)
      set(init_func "${arg}")
      set(init_keyword 1)
    elseif(init_keyword EQUAL 1)
      set(init_header "${arg}")
      set(init_keyword 0)
    else()
      list(APPEND sources "${arg}")
    endif()
  endforeach()

  string(REPLACE ";" "|" piped_components "${components}")
  set(component_genex_regex ".*TARGET_PROPERTY:(${piped_components}),.*")

  set(private_defines)
  set(interface_defines)
  set(includes)
  set(libs)
  set(component_init_funcs "")
  set(component_init_headers "")
  foreach(component ${components})
    if(NOT TARGET "${component}")
      message(FATAL_ERROR
        "Missing component library ${component} referenced by metalib ${target_name}!
        (Component library targets must be created BEFORE add_metalib.)")
    endif()

    get_target_property(is_component "${component}" IS_COMPONENT)
    if(NOT is_component)
      message(FATAL_ERROR
        "Attempted to metalink non-component ${component} into ${target_name}!")
    endif()

    get_target_property(component_init_header "${component}" INIT_HEADER)
    get_target_property(component_init_func "${component}" INIT_FUNCTION)

    if(component_init_header)
      set(component_init_headers
        "${component_init_headers}#include \"${component_init_header}\"\n")
    endif()
    if(component_init_func)
      set(component_init_funcs
        "${component_init_funcs}  ${component_init_func}();\n")
    endif()

    if(BUILD_METALIBS)
      # This will be an object library we're pulling in; rather than link,
      # let's try to "flatten" all of its properties into ours.

      # Private defines: Just reference using a generator expression
      list(APPEND private_defines "$<TARGET_PROPERTY:${component},COMPILE_DEFINITIONS>")
      # Interface defines: Copy those
      get_target_property(component_defines "${component}" INTERFACE_COMPILE_DEFINITIONS)
      if(component_defines)
        list(APPEND interface_defines ${component_defines})
      endif()

      # Include directories: Filter out anything that references a component
      # library or anything in the project path
      get_target_property(component_includes "${component}" INTERFACE_INCLUDE_DIRECTORIES)
      foreach(component_include ${component_includes})
        if(component_include MATCHES "${component_genex_regex}")
          # Ignore component references
        elseif(component_include MATCHES "^${PROJECT_SOURCE_DIR}")
          # Include path within project; should only be included when building
          list(APPEND includes "$<BUILD_INTERFACE:${component_include}>")
        else()
          # Anything else gets included
          list(APPEND includes "${component_include}")
        endif()
      endforeach(component_include)

      # Link libraries: Filter out component libraries; we aren't linking
      # against them, we're using their objects
      get_target_property(component_libraries "${component}" INTERFACE_LINK_LIBRARIES)
      foreach(component_library ${component_libraries})
        if(NOT component_library)
          # NOTFOUND - guess there are no INTERFACE_LINK_LIBRARIES
        elseif(component_library MATCHES "${component_genex_regex}")
          # Ignore component references
        elseif(component_library MATCHES ".*(${piped_components}).*")
          # Component library, ignore
        else()
          # Anything else gets included
          list(APPEND libs "${component_library}")
        endif()
      endforeach(component_library)

      # Consume this component's objects
      list(APPEND sources "$<TARGET_OBJECTS:${component}>")
    else()
      list(APPEND libs "${component}")
    endif()
  endforeach()

  if(init_func)
    set(init_source_path "${CMAKE_CURRENT_BINARY_DIR}/init_${target_name}.cxx")
    set(init_header_path "${CMAKE_CURRENT_BINARY_DIR}/${init_header}")

    configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/metalib_init.cxx.in" "${init_source_path}")
    list(APPEND sources "${init_source_path}")

    configure_file("${PROJECT_SOURCE_DIR}/cmake/templates/metalib_init.h.in" "${init_header_path}")
    install(FILES "${init_header_path}" DESTINATION include/panda3d)
  endif()

  add_library("${target_name}" ${sources})
  target_compile_definitions("${target_name}"
    PRIVATE ${private_defines}
    INTERFACE ${interface_defines})
  target_link_libraries("${target_name}" ${libs})
  target_include_directories("${target_name}"
    PUBLIC ${includes}
    INTERFACE "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include/panda3d>")

endfunction(add_metalib)
