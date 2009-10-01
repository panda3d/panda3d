#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m prc:c
#define WIN_SYS_LIBS $[WIN_SYS_LIBS] ws2_32.lib
                   
#begin lib_target
  #define TARGET display
  #define LOCAL_LIBS \
    pgraph pgraphnodes cull putil gsgbase gobj linmath mathutil \
    pstatclient

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 
 
  #define SOURCES  \
    standardMunger.I standardMunger.h \
    config_display.h \
    drawableRegion.I drawableRegion.h \
    displayRegion.I displayRegion.h  \
    displayRegionCullCallbackData.I displayRegionCullCallbackData.h \
    displayRegionDrawCallbackData.I displayRegionDrawCallbackData.h \
    frameBufferProperties.I frameBufferProperties.h \
    graphicsEngine.I graphicsEngine.h \
    graphicsOutput.I graphicsOutput.h \
    graphicsBuffer.I graphicsBuffer.h \
    graphicsPipe.I graphicsPipe.h  \
    graphicsPipeSelection.I graphicsPipeSelection.h \
    graphicsStateGuardian.I \
    graphicsStateGuardian.h \
    graphicsThreadingModel.I graphicsThreadingModel.h \
    graphicsWindow.I graphicsWindow.h \
    graphicsWindowInputDevice.I  \
    graphicsWindowInputDevice.h \
    graphicsDevice.h graphicsDevice.I \
    lru.h \
    nativeWindowHandle.I nativeWindowHandle.h \
    parasiteBuffer.I parasiteBuffer.h \
    windowHandle.I windowHandle.h \
    windowProperties.I windowProperties.h \
    renderBuffer.h \
    stencilRenderStates.h \
    stereoDisplayRegion.I stereoDisplayRegion.h \
    displaySearchParameters.h \
    displayInformation.h \
    subprocessWindow.h subprocessWindow.I \
    $[if $[and $[OSX_PLATFORM],$[HAVE_P3D_PLUGIN]], subprocessWindowBuffer.h subprocessWindowBuffer.I subprocessWindowBuffer.cxx]
    
 #define INCLUDED_SOURCES  \
    standardMunger.cxx \
    config_display.cxx \
    drawableRegion.cxx \
    displayRegion.cxx \
    displayRegionCullCallbackData.cxx \
    displayRegionDrawCallbackData.cxx \
    frameBufferProperties.cxx \
    graphicsEngine.cxx \
    graphicsOutput.cxx \
    graphicsBuffer.cxx \
    graphicsPipe.cxx \
    graphicsPipeSelection.cxx \
    graphicsStateGuardian.cxx  \
    graphicsThreadingModel.cxx \
    graphicsWindow.cxx graphicsWindowInputDevice.cxx  \
    graphicsDevice.cxx \
    nativeWindowHandle.cxx \
    parasiteBuffer.cxx \
    windowHandle.cxx \
    windowProperties.cxx \
    lru.cxx \
    stencilRenderStates.cxx \
    stereoDisplayRegion.cxx \
    displaySearchParameters.cxx \
    displayInformation.cxx \
    subprocessWindow.cxx

  #define INSTALL_HEADERS \
    standardMunger.I standardMunger.h \
    config_display.h \
    drawableRegion.I drawableRegion.h \
    displayRegion.I displayRegion.h \
    displayRegionCullCallbackData.I displayRegionCullCallbackData.h \
    displayRegionDrawCallbackData.I displayRegionDrawCallbackData.h \
    frameBufferProperties.I frameBufferProperties.h \
    graphicsEngine.I graphicsEngine.h \
    graphicsOutput.I graphicsOutput.h \
    graphicsBuffer.I graphicsBuffer.h \
    graphicsPipe.I graphicsPipe.h \
    graphicsPipeSelection.I graphicsPipeSelection.h \
    graphicsStateGuardian.I \
    graphicsStateGuardian.h \
    graphicsWindow.I graphicsWindow.h \
    graphicsThreadingModel.I graphicsThreadingModel.h \
    graphicsWindowInputDevice.I graphicsWindowInputDevice.h \
    graphicsDevice.I graphicsDevice.h \
    lru.h \
    nativeWindowHandle.I nativeWindowHandle.h \
    parasiteBuffer.I parasiteBuffer.h \
    windowHandle.I windowHandle.h \
    windowProperties.I windowProperties.h \
    renderBuffer.h \
    stencilRenderStates.h \
    stereoDisplayRegion.I stereoDisplayRegion.h \
    displaySearchParameters.h \
    displayInformation.h \
    subprocessWindow.h subprocessWindow.I \
    subprocessWindowBuffer.h subprocessWindowBuffer.I

  #define IGATESCAN all

#end lib_target


#begin static_lib_target
  // We build a static library of just these files, so the plugin can
  // link with it in direct/src/plugin, without pulling in the rest of
  // Panda.

  #define BUILD_TARGET $[and $[OSX_PLATFORM],$[HAVE_P3D_PLUGIN]]

  #define TARGET subprocbuffer

  #define SOURCES \
    subprocessWindowBuffer.h subprocessWindowBuffer.I \
    subprocessWindowBuffer.cxx

#end static_lib_target


#begin test_bin_target
  #define TARGET test_display
  #define LOCAL_LIBS \
    display putil

  #define SOURCES \
    test_display.cxx

#end test_bin_target

