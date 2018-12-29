if(NOT BUILD_PANDA)
  message(FATAL_ERROR "Cannot build direct without panda!  Please enable the BUILD_PANDA option.")
endif()

# Include source directories which have C++ components:
add_subdirectory(src/dcparse)
add_subdirectory(src/dcparser)
add_subdirectory(src/deadrec)
add_subdirectory(src/directbase)
#add_subdirectory(src/directd)
#add_subdirectory(src/directdServer)
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
set(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME "DirectDevel")
add_metalib(p3direct INIT init_libdirect direct.h COMPONENTS ${P3DIRECT_COMPONENTS})
unset(CMAKE_INSTALL_DEFAULT_COMPONENT_NAME)
set_property(TARGET p3direct PROPERTY LINKER_LANGUAGE "CXX")

# Installation:
install(TARGETS p3direct
  EXPORT Direct COMPONENT Direct
  DESTINATION lib
  RUNTIME DESTINATION bin
  ARCHIVE COMPONENT DirectDevel)

if(HAVE_PYTHON)
  # Now for the Python side of everything

  add_python_module(direct
    p3dcparser p3deadrec p3distributed p3interval
    p3motiontrail p3showbase LINK p3direct IMPORT panda3d.core COMPONENT Direct)

  # Copy Python source files into the build directory
  set(DIRECT_INSTALL_DIRECTORIES
      actor cluster controls directbase directdevices directnotify directscripts
      directtools directutil dist distributed extensions_native filter
      fsm gui interval leveleditor motiontrail p3d particles physics
      showbase showutil stdpy task tkpanels tkwidgets wxwidgets)

  foreach(dir ${DIRECT_INSTALL_DIRECTORIES})
    file(COPY
      DIRECTORY "src/${dir}"
      DESTINATION "${PROJECT_BINARY_DIR}/direct"
      FILES_MATCHING PATTERN "*.py")
  endforeach()

  # Install all files
  install_python_package("${PROJECT_BINARY_DIR}/direct" LIB COMPONENT Direct)

  # This bit is to generate the 'pandac' compatibility shim. It's deprecated now,
  # but in older versions of Panda3D, one would use
  # from pandac.PandaModules import *
  # instead of
  # from panda3d.FOO import *
  # Generate PandaModules:
  file(WRITE "${PROJECT_BINARY_DIR}/pandac/PandaModules.py"
    "\"This module is deprecated.  Import from panda3d.core and other panda3d.* modules instead.\"

print(\"Warning: pandac.PandaModules is deprecated, import from panda3d.core instead\")\n")

  foreach(module ${ALL_INTERROGATE_MODULES})
    file(APPEND "${PROJECT_BINARY_DIR}/pandac/PandaModules.py" "
try:
    from panda3d.${module} import *
except ImportError as err:
    if not (\"No module named\" in str(err) and \"${module}\" in str(err)):
        raise
")
  endforeach()

  # Now install ourselves:
  install_python_package("${PROJECT_BINARY_DIR}/pandac" LIB COMPONENT Direct)
endif()

# "Direct" component contains both the Python stuff and the non-Python stuff,
# because direct has a pretty solid dependency on Python.
export_targets(Direct NAMESPACE "Panda3D::Direct::" COMPONENT DirectDevel)