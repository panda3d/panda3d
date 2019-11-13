set(INTERROGATE_HEADERS
  functionRemap.h
  functionWriter.h
  functionWriterPtrFromPython.h functionWriterPtrToPython.h
  functionWriters.h
  interfaceMaker.h
  interfaceMakerC.h
  interfaceMakerPython.h interfaceMakerPythonObj.h
  interfaceMakerPythonSimple.h
  interfaceMakerPythonNative.h
  interrogate.h interrogateBuilder.h parameterRemap.I
  parameterRemap.h
  parameterRemapBasicStringPtrToString.h
  parameterRemapBasicStringRefToString.h
  parameterRemapBasicStringToString.h
  parameterRemapCharStarToString.h
  parameterRemapConcreteToPointer.h
  parameterRemapConstToNonConst.h parameterRemapEnumToInt.h
  parameterRemapPTToPointer.h
  parameterRemapReferenceToConcrete.h
  parameterRemapReferenceToPointer.h parameterRemapThis.h
  parameterRemapToString.h
  parameterRemapHandleToInt.h
  parameterRemapUnchanged.h
  typeManager.h
)

set(INTERROGATE_SOURCES
  functionRemap.cxx
  functionWriter.cxx
  functionWriterPtrFromPython.cxx functionWriterPtrToPython.cxx
  functionWriters.cxx
  interfaceMaker.cxx
  interfaceMakerC.cxx
  interfaceMakerPython.cxx interfaceMakerPythonObj.cxx
  interfaceMakerPythonSimple.cxx
  interfaceMakerPythonNative.cxx
  interrogate.cxx interrogateBuilder.cxx parameterRemap.cxx
  parameterRemapBasicStringPtrToString.cxx
  parameterRemapBasicStringRefToString.cxx
  parameterRemapBasicStringToString.cxx
  parameterRemapCharStarToString.cxx
  parameterRemapConcreteToPointer.cxx
  parameterRemapConstToNonConst.cxx
  parameterRemapEnumToInt.cxx parameterRemapPTToPointer.cxx
  parameterRemapReferenceToConcrete.cxx
  parameterRemapReferenceToPointer.cxx parameterRemapThis.cxx
  parameterRemapToString.cxx
  parameterRemapHandleToInt.cxx
  parameterRemapUnchanged.cxx
  typeManager.cxx
)

set(INTERROGATE_PREAMBLE_PYTHON_NATIVE
  ../interrogatedb/py_panda.cxx
  ../interrogatedb/py_compat.cxx
  ../interrogatedb/py_wrappers.cxx
  ../interrogatedb/dtool_super_base.cxx
)

composite_sources(interrogate INTERROGATE_SOURCES)
add_executable(interrogate ${INTERROGATE_HEADERS} ${INTERROGATE_SOURCES})
target_link_libraries(interrogate p3cppParser p3interrogatedb
  PKG::OPENSSL)

# Python preamble for interrogate_module
set(_preamble_files)
foreach(_file ${INTERROGATE_PREAMBLE_PYTHON_NATIVE})
  # Resolve symlinks, use absolute path, and add a `#line 1 ...` directive
  get_filename_component(_file "${_file}" REALPATH)

  string(SHA1 _hash "${_file}")
  set(_line_file "${CMAKE_CURRENT_BINARY_DIR}/.${_hash}.h")
  file(WRITE "${_line_file}" "#line 1 \"${_file}\"\n")

  list(APPEND _preamble_files "${_line_file}")
  list(APPEND _preamble_files "${_file}")
endforeach(_file)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/interrogate_preamble_python_native.cxx
  COMMAND ${CMAKE_COMMAND}
    -D OUTPUT_FILE="${CMAKE_CURRENT_BINARY_DIR}/interrogate_preamble_python_native.cxx"
    -D INPUT_FILES="${_preamble_files}"
    -D SYMBOL_NAME="interrogate_preamble_python_native"
    -P ${PROJECT_SOURCE_DIR}/cmake/scripts/ConcatenateToCXX.cmake
  DEPENDS ${INTERROGATE_PREAMBLE_PYTHON_NATIVE}
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(interrogate_module interrogate_module.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/interrogate_preamble_python_native.cxx)
target_link_libraries(interrogate_module p3cppParser p3interrogatedb
  PKG::OPENSSL)

if(NOT CMAKE_CROSSCOMPILING)
  add_executable(host_interrogate ALIAS interrogate)
  add_executable(host_interrogate_module ALIAS interrogate_module)

else()
  # When cross-compiling, we must find the host Interrogate.
  # TODO: We should also try to find_package(Panda3D ...) before falling back
  # to searching manually.
  find_program(HOST_PATH_INTERROGATE interrogate)
  find_program(HOST_PATH_INTERROGATE_MODULE interrogate_module)

  add_executable(host_interrogate IMPORTED GLOBAL)
  if(HOST_PATH_INTERROGATE)
    set_target_properties(host_interrogate PROPERTIES
      IMPORTED_LOCATION "${HOST_PATH_INTERROGATE}")
  endif()

  add_executable(host_interrogate_module IMPORTED GLOBAL)
  if(HOST_PATH_INTERROGATE_MODULE)
    set_target_properties(host_interrogate_module PROPERTIES
      IMPORTED_LOCATION "${HOST_PATH_INTERROGATE_MODULE}")
  endif()

endif()

if(WANT_INTERROGATE)
  install(TARGETS interrogate interrogate_module EXPORT Core COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_BINDIR})
  install(FILES ${INTERROGATE_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)

else()
  set_target_properties(interrogate interrogate_module
    PROPERTIES EXCLUDE_FROM_ALL ON)

endif()
