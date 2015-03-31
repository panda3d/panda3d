#define OTHER_LIBS p3interrogatedb:c p3dconfig:c p3dtoolconfig:m \
                   p3dtoolutil:c p3dtoolbase:c p3dtool:m p3prc:c
#define WIN_SYS_LIBS $[WIN_SYS_LIBS] ws2_32.lib
                   
#begin lib_target
  #define TARGET p3display
  #define LOCAL_LIBS \
    p3pgraph p3pgraphnodes p3cull p3putil p3gsgbase p3gobj p3linmath p3mathutil \
    p3pstatclient

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 
 
  #define SOURCES  \
    standardMunger.I standardMunger.h \
    config_display.h \
    $[if $[HAVE_PYTHON], pythonGraphicsWindowProc.h] \
    $[if $[HAVE_PYTHON], pythonGraphicsWindowProc.cxx] \
    callbackGraphicsWindow.I callbackGraphicsWindow.h \
    drawableRegion.I drawableRegion.h \
    displayRegion.I displayRegion.h  \
    displayRegionCullCallbackData.I displayRegionCullCallbackData.h \
    displayRegionDrawCallbackData.I displayRegionDrawCallbackData.h \
    frameBufferProperties.I frameBufferProperties.h \
    get_x11.h pre_x11_include.h post_x11_include.h \
    graphicsEngine.I graphicsEngine.h \
    graphicsOutput.I graphicsOutput.h \
    graphicsBuffer.I graphicsBuffer.h \
    graphicsDevice.h graphicsDevice.I \
    graphicsPipe.I graphicsPipe.h  \
    graphicsPipeSelection.I graphicsPipeSelection.h \
    graphicsStateGuardian.I graphicsStateGuardian.h \
    graphicsStateGuardian_ext.cxx graphicsStateGuardian_ext.h \
    graphicsThreadingModel.I graphicsThreadingModel.h \
    graphicsWindow.I graphicsWindow.h \
    graphicsWindow_ext.cxx graphicsWindow_ext.h \
    graphicsWindowInputDevice.I  \
    graphicsWindowInputDevice.h \
    graphicsWindowProc.h \
    graphicsWindowProcCallbackData.I graphicsWindowProcCallbackData.h \
    nativeWindowHandle.I nativeWindowHandle.h \
    parasiteBuffer.I parasiteBuffer.h \
    pStatGPUTimer.I pStatGPUTimer.h \
    windowHandle.I windowHandle.h \
    windowProperties.I windowProperties.h \
    renderBuffer.h \
    stereoDisplayRegion.I stereoDisplayRegion.h \
    displaySearchParameters.h \
    displayInformation.h \
    subprocessWindow.h subprocessWindow.I \
    $[if $[OSX_PLATFORM], subprocessWindowBuffer.h subprocessWindowBuffer.I] \
    touchInfo.h 

    
 #define INCLUDED_SOURCES  \
    standardMunger.cxx \
    config_display.cxx \
    callbackGraphicsWindow.cxx \
    drawableRegion.cxx \
    displayRegion.cxx \
    displayRegionCullCallbackData.cxx \
    displayRegionDrawCallbackData.cxx \
    displaySearchParameters.cxx \
    displayInformation.cxx \
    frameBufferProperties.cxx \
    graphicsEngine.cxx \
    graphicsOutput.cxx \
    graphicsBuffer.cxx \
    graphicsPipe.cxx \
    graphicsPipeSelection.cxx \
    graphicsStateGuardian.cxx  \
    graphicsThreadingModel.cxx \
    graphicsWindow.cxx graphicsWindowInputDevice.cxx  \
    graphicsWindowProc.cxx \
    graphicsWindowProcCallbackData.cxx \
    graphicsDevice.cxx \
    nativeWindowHandle.cxx \
    parasiteBuffer.cxx \
    windowHandle.cxx \
    windowProperties.cxx \
    stereoDisplayRegion.cxx \
    subprocessWindow.cxx \
    touchInfo.cxx

  #define INSTALL_HEADERS \
    standardMunger.I standardMunger.h \
    config_display.h \
    $[if $[HAVE_PYTHON], pythonGraphicsWindowProc.h] \
    callbackGraphicsWindow.I callbackGraphicsWindow.h \
    drawableRegion.I drawableRegion.h \
    displayInformation.h \
    displayRegion.I displayRegion.h \
    displayRegionCullCallbackData.I displayRegionCullCallbackData.h \
    displayRegionDrawCallbackData.I displayRegionDrawCallbackData.h \
    displaySearchParameters.h \
    frameBufferProperties.I frameBufferProperties.h \
    get_x11.h pre_x11_include.h post_x11_include.h \
    graphicsEngine.I graphicsEngine.h \
    graphicsOutput.I graphicsOutput.h \
    graphicsBuffer.I graphicsBuffer.h \
    graphicsPipe.I graphicsPipe.h \
    graphicsPipeSelection.I graphicsPipeSelection.h \
    graphicsStateGuardian.I \
    graphicsStateGuardian.h \
    graphicsWindow.I graphicsWindow.h \
    graphicsWindowProc.h \
    graphicsWindowProcCallbackData.I graphicsWindowProcCallbackData.h \
    graphicsThreadingModel.I graphicsThreadingModel.h \
    graphicsWindowInputDevice.I graphicsWindowInputDevice.h \
    graphicsDevice.I graphicsDevice.h \
    nativeWindowHandle.I nativeWindowHandle.h \
    parasiteBuffer.I parasiteBuffer.h \
    pStatGPUTimer.I pStatGPUTimer.h \
    windowHandle.I windowHandle.h \
    windowProperties.I windowProperties.h \
    renderBuffer.h \
    stereoDisplayRegion.I stereoDisplayRegion.h \
    subprocessWindow.h subprocessWindow.I \
    subprocessWindowBuffer.h subprocessWindowBuffer.I \
    touchInfo.h

  #define IGATESCAN all

#end lib_target


#begin static_lib_target
  // We build a static library of just these files, so the plugin can
  // link with it in direct/src/plugin, without pulling in the rest of
  // Panda.

  #define BUILD_TARGET $[and $[OSX_PLATFORM],$[HAVE_P3D_PLUGIN]]

  #define TARGET p3subprocbuffer

  #define SOURCES \
    subprocessWindowBuffer.h subprocessWindowBuffer.I \
    subprocessWindowBuffer.cxx

#end static_lib_target


#begin test_bin_target
  #define TARGET test_display
  #define LOCAL_LIBS \
    p3display p3putil

  #define SOURCES \
    test_display.cxx

#end test_bin_target

