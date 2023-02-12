set(P3DISPLAY_HEADERS
  standardMunger.I standardMunger.h
  config_display.h
  callbackGraphicsWindow.I callbackGraphicsWindow.h
  drawableRegion.I drawableRegion.h
  displayRegion.I displayRegion.h
  displayRegionCullCallbackData.I displayRegionCullCallbackData.h
  displayRegionDrawCallbackData.I displayRegionDrawCallbackData.h
  frameBufferProperties.I frameBufferProperties.h
  get_x11.h pre_x11_include.h post_x11_include.h
  graphicsEngine.I graphicsEngine.h
  graphicsOutput.I graphicsOutput.h
  graphicsBuffer.I graphicsBuffer.h
  graphicsDevice.h graphicsDevice.I
  graphicsPipe.I graphicsPipe.h
  graphicsPipeSelection.I graphicsPipeSelection.h
  graphicsStateGuardian.I graphicsStateGuardian.h
  graphicsThreadingModel.I graphicsThreadingModel.h
  graphicsWindow.I graphicsWindow.h
  graphicsWindowInputDevice.I graphicsWindowInputDevice.h
  graphicsWindowProc.h
  graphicsWindowProcCallbackData.I graphicsWindowProcCallbackData.h
  mouseAndKeyboard.h
  nativeWindowHandle.I nativeWindowHandle.h
  parasiteBuffer.I parasiteBuffer.h
  pStatGPUTimer.I pStatGPUTimer.h
  windowHandle.I windowHandle.h
  windowProperties.I windowProperties.h
  renderBuffer.h
  screenshotRequest.I screenshotRequest.h
  stereoDisplayRegion.I stereoDisplayRegion.h
  displaySearchParameters.h
  displayInformation.h
  subprocessWindow.h subprocessWindow.I
  touchInfo.h
)

set(P3DISPLAY_SOURCES
  standardMunger.cxx
  config_display.cxx
  callbackGraphicsWindow.cxx
  drawableRegion.cxx
  displayRegion.cxx
  displayRegionCullCallbackData.cxx
  displayRegionDrawCallbackData.cxx
  displaySearchParameters.cxx
  displayInformation.cxx
  frameBufferProperties.cxx
  graphicsEngine.cxx
  graphicsOutput.cxx
  graphicsBuffer.cxx
  graphicsPipe.cxx
  graphicsPipeSelection.cxx
  graphicsStateGuardian.cxx
  graphicsThreadingModel.cxx
  graphicsWindow.cxx graphicsWindowInputDevice.cxx
  graphicsWindowProc.cxx
  graphicsWindowProcCallbackData.cxx
  graphicsDevice.cxx
  mouseAndKeyboard.cxx
  nativeWindowHandle.cxx
  parasiteBuffer.cxx
  windowHandle.cxx
  windowProperties.cxx
  screenshotRequest.cxx
  stereoDisplayRegion.cxx
  subprocessWindow.cxx
  touchInfo.cxx
)

set(P3DISPLAY_IGATEEXT
  frameBufferProperties_ext.cxx
  frameBufferProperties_ext.h
  graphicsPipeSelection_ext.cxx
  graphicsPipeSelection_ext.h
  graphicsStateGuardian_ext.cxx
  graphicsStateGuardian_ext.h
  graphicsWindow_ext.cxx
  graphicsWindow_ext.h
  pythonGraphicsWindowProc.cxx
  pythonGraphicsWindowProc.h
  windowProperties_ext.cxx
  windowProperties_ext.h
)

if(APPLE)
  set(P3DISPLAY_HEADERS
    ${P3DISPLAY_HEADERS}
    subprocessWindowBuffer.h
    subprocessWindowBuffer.I
  )

  set(P3DISPLAY_SOURCES
    ${P3DISPLAY_SOURCES}
    subprocessWindowBuffer.cxx
  )
endif()

# This is a large file; let's build it separately
set_source_files_properties(graphicsStateGuardian.cxx PROPERTIES SKIP_UNITY_BUILD_INCLUSION YES)

composite_sources(p3display P3DISPLAY_SOURCES)
add_component_library(p3display SYMBOL BUILDING_PANDA_DISPLAY
  ${P3DISPLAY_HEADERS} ${P3DISPLAY_SOURCES})
target_link_libraries(p3display p3cull p3device p3pgraphnodes
  PKG::X11)
target_interrogate(p3display ALL EXTENSIONS ${P3DISPLAY_IGATEEXT})

if(HAVE_EIGEN)
  # Eigen puts this over the threshold where bigobj is needed on MSVC
  set_target_properties(p3display PROPERTIES MSVC_BIGOBJ ON)
endif()

if(NOT BUILD_METALIBS)
  install(TARGETS p3display
    EXPORT Core COMPONENT Core
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d
    ARCHIVE COMPONENT CoreDevel)
endif()
install(FILES ${P3DISPLAY_HEADERS} COMPONENT CoreDevel DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/panda3d)
