set(P3SHADERPIPELINE_HEADERS
  config_shaderpipeline.h
  shaderCompilerGlslang.h
  shaderCompilerGlslPreProc.h
  shaderModuleGlsl.h
  shaderModuleSpirV.h shaderModuleSpirV.I
  spirVFlattenStructPass.h
  spirVHoistStructResourcesPass.h
  spirVInjectAlphaTestPass.h
  spirVMakeBlockPass.h
  spirVRemoveUnusedVariablesPass.h
  spirVReplaceVariableTypePass.h
  spirVResultDatabase.h spirVResultDatabase.I
  spirVTransformer.h spirVTransformer.I
  spirVTransformPass.h spirVTransformPass.I
)

set(P3SHADERPIPELINE_SOURCES
  config_shaderpipeline.cxx
  shaderCompilerGlslang.cxx
  shaderCompilerGlslPreProc.cxx
  shaderModuleGlsl.cxx
  shaderModuleSpirV.cxx
  spirVFlattenStructPass.cxx
  spirVHoistStructResourcesPass.cxx
  spirVInjectAlphaTestPass.cxx
  spirVMakeBlockPass.cxx
  spirVRemoveUnusedVariablesPass.cxx
  spirVReplaceVariableTypePass.cxx
  spirVResultDatabase.cxx
  spirVTransformer.cxx
  spirVTransformPass.cxx
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/cg_preamble.cxx
  COMMAND ${CMAKE_COMMAND}
    -D OUTPUT_FILE="${CMAKE_CURRENT_BINARY_DIR}/cg_preamble.cxx"
    -D INPUT_FILES="${CMAKE_CURRENT_SOURCE_DIR}/cg_preamble.hlsl"
    -D SYMBOL_NAME="cg_preamble"
    -P ${PROJECT_SOURCE_DIR}/cmake/scripts/ConcatenateToCXX.cmake
  DEPENDS cg_preamble.hlsl
  WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

composite_sources(p3shaderpipeline P3SHADERPIPELINE_SOURCES)
add_component_library(p3shaderpipeline NOINIT SYMBOL BUILDING_PANDA_SHADERPIPELINE
  ${P3SHADERPIPELINE_HEADERS} ${P3SHADERPIPELINE_SOURCES}
  ${CMAKE_CURRENT_BINARY_DIR}/cg_preamble.cxx)
target_link_libraries(p3shaderpipeline p3gobj PKG::GLSLANG)

if(CMAKE_CXX_COMPILER_ID MATCHES "^(GNU|Clang)$")
  # Keep symbols from glslang internal.
  target_link_options(p3shaderpipeline PRIVATE "LINKER:--exclude-libs,libglslang.a")
  target_link_options(p3shaderpipeline PRIVATE "LINKER:--exclude-libs,libSPIRV.a")
  target_link_options(p3shaderpipeline PRIVATE "LINKER:--exclude-libs,libOSDependent.a")
  target_link_options(p3shaderpipeline PRIVATE "LINKER:--exclude-libs,libOGLCompiler.a")
  target_link_options(p3shaderpipeline PRIVATE "LINKER:--exclude-libs,libHLSL.a")
  target_link_options(p3shaderpipeline PRIVATE "LINKER:--exclude-libs,libglslang-default-resource-limits.a")
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3shaderpipeline
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3SHADERPIPELINE_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
