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
add_subdirectory(src/interval)
add_subdirectory(src/motiontrail)
add_subdirectory(src/showbase)

set(P3DIRECT_COMPONENTS
  p3dcparser p3deadrec
  p3interval p3motiontrail p3showbase)
if(HAVE_PYTHON)
  list(APPEND P3DIRECT_COMPONENTS p3distributed)
endif()
add_metalib(p3direct INIT init_libdirect direct.h COMPONENTS ${P3DIRECT_COMPONENTS})
set_property(TARGET p3direct PROPERTY LINKER_LANGUAGE "CXX")

if(HAVE_PYTHON)
  add_python_module(direct
    p3dcparser p3deadrec p3distributed p3interval
    p3motiontrail p3showbase LINK p3direct IMPORT panda3d.core)

  # Make an __init__.py pointing at the source directory so users can run
  # Panda3D code straight from their build path:
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/__init__.py" "__path__.insert(0,r\"${CMAKE_CURRENT_SOURCE_DIR}/src\")")
endif()

# Installation:
install(TARGETS p3direct DESTINATION lib)
if(HAVE_PYTHON)
  set(DIRECT_INSTALL_DIRECTORIES
      actor cluster controls directbase directdevices directnotify directscripts
      directtools directutil distributed extensions extensions_native ffi filter
      fsm gui http interval leveleditor motiontrail p3d particles physics pyinst
      showbase showutil stdpy task test tkpanels tkwidgets wxwidgets)

  foreach(dir ${DIRECT_INSTALL_DIRECTORIES})
    file(COPY
      DIRECTORY "src/${dir}"
      DESTINATION "${PROJECT_BINARY_DIR}/direct"
      FILES_MATCHING PATTERN "*.py")
    install(
      DIRECTORY "src/${dir}"
      DESTINATION "${PYTHON_LIB_INSTALL_DIR}/direct"
      FILES_MATCHING PATTERN "*.py")
  endforeach()

  # We also need an __init__.py file, which we have to generate:
  file(WRITE "${PROJECT_BINARY_DIR}/direct/__init__.py" "")
  install(
    FILES "${PROJECT_BINARY_DIR}/direct/__init__.py"
    DESTINATION "${PYTHON_LIB_INSTALL_DIR}/direct")
endif()
