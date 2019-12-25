if(NOT HAVE_DX9)
  return()
endif()

set(P3DXGSG9_HEADERS
  config_dxgsg9.h
  dxGeomMunger9.h dxGeomMunger9.I
  dxGraphicsDevice9.h
  dxGraphicsStateGuardian9.h dxGraphicsStateGuardian9.I
  dxgsg9base.h
  dxIndexBufferContext9.h dxIndexBufferContext9.I
  dxOcclusionQueryContext9.h dxOcclusionQueryContext9.I
  dxShaderContext9.h dxShaderContext9.I
  dxTextureContext9.h dxTextureContext9.I
  dxVertexBufferContext9.h dxVertexBufferContext9.I
  vertexElementArray.h
  wdxGraphicsBuffer9.h wdxGraphicsBuffer9.I
  wdxGraphicsPipe9.h wdxGraphicsPipe9.I
  wdxGraphicsWindow9.h wdxGraphicsWindow9.I
)

set(P3DXGSG9_SOURCES
  config_dxgsg9.cxx
  dxGeomMunger9.cxx
  dxGraphicsDevice9.cxx
  dxGraphicsStateGuardian9.cxx
  dxIndexBufferContext9.cxx
  dxOcclusionQueryContext9.cxx
  dxShaderContext9.cxx
  dxTextureContext9.cxx
  dxVertexBufferContext9.cxx
  vertexElementArray.cxx
  wdxGraphicsBuffer9.cxx
  wdxGraphicsPipe9.cxx
  wdxGraphicsWindow9.cxx
)

# This is a large file; let's build it separately
set_source_files_properties(dxGraphicsStateGuardian9.cxx PROPERTIES SKIP_UNITY_BUILD_INCLUSION YES)

composite_sources(p3dxgsg9 P3DXGSG9_SOURCES)
add_component_library(p3dxgsg9 SYMBOL BUILDING_PANDADX
  ${P3DXGSG9_HEADERS} ${P3DXGSG9_SOURCES})
target_link_libraries(p3dxgsg9 p3windisplay panda
  PKG::CG PKG::DX9)
target_compile_definitions(p3dxgsg9 PUBLIC USE_GENERIC_DXERR_LIBRARY) # FIXME

if(NOT BUILD_METALIBS)
  install(TARGETS p3dxgsg9
    EXPORT DX9 COMPONENT DX9
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT DX9Devel)
endif()
install(FILES ${P3DXGSG9_HEADERS} COMPONENT DX9Devel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
