#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
                   
#begin lib_target
  #define TARGET display
  #define LOCAL_LIBS \
    pgraph putil gsgbase gobj linmath graph mathutil sgraph \
    pstatclient

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 
 
  #define SOURCES  \
     config_display.h displayRegion.I displayRegion.h  \
     displayRegionStack.I \
     displayRegionStack.h \
     drawCullHandler.h drawCullHandler.I \
     frameBufferStack.I frameBufferStack.h \
     geomContext.I geomContext.h geomNodeContext.I geomNodeContext.h \
     graphicsChannel.I graphicsChannel.h \
     graphicsEngine.I graphicsEngine.h \
     graphicsLayer.I  \
     graphicsLayer.h graphicsPipe.I graphicsPipe.N graphicsPipe.h  \
     graphicsStateGuardian.I graphicsStateGuardian.N  \
     graphicsStateGuardian.h graphicsWindow.I graphicsWindow.N  \
     graphicsWindow.h graphicsWindowInputDevice.I  \
     graphicsWindowInputDevice.h hardwareChannel.I  \
     hardwareChannel.h interactiveGraphicsPipe.I  \
     interactiveGraphicsPipe.h \
     lensStack.I lensStack.h \
     noninteractiveGraphicsPipe.I  \
     noninteractiveGraphicsPipe.h pipeSpec.I pipeSpec.h  \
     savedFrameBuffer.I savedFrameBuffer.h
     
 #define INCLUDED_SOURCES  \
     config_display.cxx displayRegion.cxx \
     drawCullHandler.cxx \
     geomContext.cxx geomNodeContext.cxx graphicsChannel.cxx  \
     graphicsEngine.cxx \
     graphicsLayer.cxx graphicsPipe.cxx graphicsStateGuardian.cxx  \
     graphicsWindow.cxx graphicsWindowInputDevice.cxx  \
     hardwareChannel.cxx interactiveGraphicsPipe.cxx  \
     noninteractiveGraphicsPipe.cxx pipeSpec.cxx  \
     savedFrameBuffer.cxx

  #define INSTALL_HEADERS \
    config_display.h \
    displayRegion.I displayRegion.h displayRegionStack.I \
    displayRegionStack.h \
    drawCullHandler.h drawCullHandler.I \
    frameBufferStack.I frameBufferStack.h \
    geomContext.I geomContext.h geomNodeContext.I geomNodeContext.h \
    graphicsChannel.I graphicsChannel.h \
    graphicsEngine.I graphicsEngine.h \
    graphicsLayer.I graphicsLayer.h \
    graphicsPipe.I graphicsPipe.h graphicsStateGuardian.I \
    graphicsStateGuardian.h graphicsWindow.I graphicsWindow.h \
    graphicsWindowInputDevice.I graphicsWindowInputDevice.h \
    hardwareChannel.I hardwareChannel.h interactiveGraphicsPipe.I \
    interactiveGraphicsPipe.h \
    lensStack.I lensStack.h \
    noninteractiveGraphicsPipe.I \
    noninteractiveGraphicsPipe.h pipeSpec.I pipeSpec.h renderBuffer.h \
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

