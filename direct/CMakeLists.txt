if(NOT BUILD_PANDA)
  message(FATAL_ERROR "Cannot build direct without panda!  Please enable the BUILD_PANDA option.")
endif()

# Include source directories which have C++ components:
add_subdirectory(src/directbase)
add_subdirectory(src/dcparser)
add_subdirectory(src/dcparse)
add_subdirectory(src/deadrec)
#add_subdirectory(src/directdServer)
#add_subdirectory(src/directd)
add_subdirectory(src/distributed)
#add_subdirectory(src/heapq)
#add_subdirectory(src/http)
add_subdirectory(src/interval)
#add_subdirectory(src/motiontrail)
add_subdirectory(src/showbase)

add_python_module(direct p3dcparser p3deadrec p3distributed p3interval p3showbase)
target_link_libraries(direct core)

# Installation:
if(HAVE_PYTHON)
  set(DIRECT_INSTALL_DIRECTORIES
      actor cluster controls directbase directdevices directnotify directscripts
      directtools directutil distributed extensions extensions_native ffi filter
      fsm gui http interval leveleditor motiontrail p3d particles physics pyinst
      showbase showutil stdpy task test tkpanels tkwidgets wxwidgets)

  foreach(dir ${DIRECT_INSTALL_DIRECTORIES})
    install(
      DIRECTORY "src/${dir}"
      DESTINATION "${PYTHON_LIB_INSTALL_DIR}/direct"
      FILES_MATCHING PATTERN "*.py")
  endforeach()

  # We also need an __init__.py file, which we have to generate:
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" "")
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/__init__.py"
    DESTINATION "${PYTHON_LIB_INSTALL_DIR}/direct")
endif()
