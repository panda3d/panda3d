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
  lru.h
  nativeWindowHandle.I nativeWindowHandle.h
  parasiteBuffer.I parasiteBuffer.h
  windowHandle.I windowHandle.h
  windowProperties.I windowProperties.h
  renderBuffer.h
  stencilRenderStates.h
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
  lru.cxx
  nativeWindowHandle.cxx
  parasiteBuffer.cxx
  windowHandle.cxx
  windowProperties.cxx
  stencilRenderStates.cxx
  stereoDisplayRegion.cxx
  subprocessWindow.cxx
  touchInfo.cxx
)

if(HAVE_PYTHON)
	set(P3DISPLAY_HEADERS
		${P3DISPLAY_HEADERS}
		pythonGraphicsWindowProc.h
	)
	set(P3DISPLAY_SOURCES
		${P3DISPLAY_SOURCES}
		pythonGraphicsWindowProc.cxx
	)
endif()

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

composite_sources(p3display P3DISPLAY_SOURCES)
add_library(p3display ${P3DISPLAY_HEADERS} ${P3DISPLAY_SOURCES})
target_link_libraries(p3display p3cull p3pgraphnodes)
target_interrogate(p3display ALL)

install(TARGETS p3display DESTINATION lib)

if(OSX_PLATFORM AND HAVE_P3D_PLUGIN)
  # We build a static library of just these files, so the plugin can
  # link with it in direct/src/plugin, without pulling in the rest of 
  # Panda.
  add_library(p3subprocbuffer
    subprocessWindowBuffer.h
    subprocessWindowBuffer.I
    subprocessWindowBuffer.cxx
  )
endif()

#begin test_bin_target
  #define TARGET test_display
  #define LOCAL_LIBS \
#    p3display p3putil

  #define SOURCES \
#    test_display.cxx

#end test_bin_target
