#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
                   
#begin lib_target
  #define TARGET display
  #define LOCAL_LIBS \
    pgraph putil gsgbase gobj linmath mathutil \
    pstatclient

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 
 
  #define SOURCES  \
    config_display.h \
    clearableRegion.I clearableRegion.h \
    displayRegion.I displayRegion.h  \
    displayRegionStack.I \
    displayRegionStack.h \
    frameBufferProperties.I frameBufferProperties.h \
    frameBufferStack.I frameBufferStack.h \
    geomContext.I geomContext.h geomNodeContext.I geomNodeContext.h \
    graphicsChannel.I graphicsChannel.h \
    graphicsEngine.I graphicsEngine.h \
    graphicsLayer.I  \
    graphicsLayer.h graphicsPipe.I graphicsPipe.h  \
    graphicsPipeSelection.I graphicsPipeSelection.h \
    graphicsStateGuardian.I \
    graphicsStateGuardian.h graphicsWindow.I \
    graphicsThreadingModel.I graphicsThreadingModel.h \
    graphicsWindow.h graphicsWindowInputDevice.I  \
    graphicsWindowInputDevice.h \
    windowProperties.I windowProperties.h \
    hardwareChannel.I  \
    hardwareChannel.h \
    lensStack.I lensStack.h \
    savedFrameBuffer.I savedFrameBuffer.h
    
 #define INCLUDED_SOURCES  \
    config_display.cxx \
    clearableRegion.cxx \
    displayRegion.cxx \
    frameBufferProperties.cxx \
    geomContext.cxx geomNodeContext.cxx graphicsChannel.cxx  \
    graphicsEngine.cxx \
    graphicsLayer.cxx graphicsPipe.cxx \
    graphicsPipeSelection.cxx \
    graphicsStateGuardian.cxx  \
    graphicsThreadingModel.cxx \
    graphicsWindow.cxx graphicsWindowInputDevice.cxx  \
    windowProperties.cxx \
    hardwareChannel.cxx \
    savedFrameBuffer.cxx

  #define INSTALL_HEADERS \
    config_display.h \
    clearableRegion.I clearableRegion.h \
    displayRegion.I displayRegion.h displayRegionStack.I \
    displayRegionStack.h \
    frameBufferProperties.I frameBufferProperties.h \
    frameBufferStack.I frameBufferStack.h \
    geomContext.I geomContext.h geomNodeContext.I geomNodeContext.h \
    graphicsChannel.I graphicsChannel.h \
    graphicsEngine.I graphicsEngine.h \
    graphicsLayer.I graphicsLayer.h \
    graphicsPipe.I graphicsPipe.h \
    graphicsPipeSelection.I graphicsPipeSelection.h \
    graphicsStateGuardian.I \
    graphicsStateGuardian.h graphicsWindow.I graphicsWindow.h \
    graphicsThreadingModel.I graphicsThreadingModel.h \
    graphicsWindowInputDevice.I graphicsWindowInputDevice.h \
    windowProperties.I windowProperties.h \
    hardwareChannel.I hardwareChannel.h \
    lensStack.I lensStack.h \
    renderBuffer.h \
    savedFrameBuffer.I savedFrameBuffer.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_display
  #define LOCAL_LIBS \
    display putil

  #define SOURCES \
    test_display.cxx

#end test_bin_target

