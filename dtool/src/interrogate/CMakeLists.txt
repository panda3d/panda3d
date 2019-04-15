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
target_link_libraries(interrogate p3cppParser p3dtoolconfig p3pystub
  PKG::OPENSSL)

# Python preamble for interrogate_module
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/interrogate_preamble_python_native.cxx
  "extern const char interrogate_preamble_python_native[];\n")
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/interrogate_preamble_python_native.cxx
  "const char interrogate_preamble_python_native[] = (\n")
foreach(preamble_file ${INTERROGATE_PREAMBLE_PYTHON_NATIVE})
  file(READ ${preamble_file} preamble_content)
  string(REPLACE "\\" "\\\\" preamble_content "${preamble_content}")
  string(REPLACE "\"" "\\\"" preamble_content "${preamble_content}")
  string(REPLACE "\n" "\\n" preamble_content "${preamble_content}")
  file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/interrogate_preamble_python_native.cxx
    "  \"${preamble_content}\"\n")
endforeach(preamble_file)
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/interrogate_preamble_python_native.cxx
  ");\n")

add_executable(interrogate_module interrogate_module.cxx
  ${CMAKE_CURRENT_BINARY_DIR}/interrogate_preamble_python_native.cxx)
target_link_libraries(interrogate_module p3cppParser p3dtoolconfig p3pystub
  PKG::OPENSSL)

if(NOT CMAKE_CROSSCOMPILING)
  add_executable(host_interrogate ALIAS interrogate)
  add_executable(host_interrogate_module ALIAS interrogate_module)
endif()

if(WANT_INTERROGATE)
  install(TARGETS interrogate interrogate_module EXPORT Core COMPONENT CoreDevel DESTINATION bin)
  install(FILES ${INTERROGATE_HEADERS} COMPONENT CoreDevel DESTINATION include/panda3d)

else()
  set_target_properties(interrogate interrogate_module
    PROPERTIES EXCLUDE_FROM_ALL ON)

endif()
