if(NOT BUILD_TOOLS)
  return()
endif()

add_executable(dxf-points dxfPoints.cxx dxfPoints.h)
target_link_libraries(dxf-points p3progbase p3dxf)
install(TARGETS dxf-points EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

if(HAVE_EGG)

  add_executable(egg2dxf eggToDXF.cxx eggToDXF.h eggToDXFLayer.cxx eggToDXFLayer.h)
  target_link_libraries(egg2dxf p3dxfegg p3eggbase p3progbase)
  install(TARGETS egg2dxf EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

  add_executable(dxf2egg dxfToEgg.cxx dxfToEgg.h)
  target_link_libraries(dxf2egg p3dxfegg p3eggbase p3progbase)
  install(TARGETS dxf2egg EXPORT Tools COMPONENT Tools DESTINATION ${CMAKE_INSTALL_BINDIR})

endif()
