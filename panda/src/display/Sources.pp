#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
				   
#begin lib_target
  #define TARGET display
  #define LOCAL_LIBS \
    putil gsgbase gobj linmath graph mathutil sgraph \
    pstatclient

  #define SOURCES \
    config_display.cxx config_display.h displayRegion.I \
    displayRegion.cxx displayRegion.h graphicsChannel.I \
    graphicsChannel.cxx graphicsChannel.h graphicsLayer.I \
    graphicsLayer.cxx graphicsLayer.h graphicsPipe.I graphicsPipe.N \
    graphicsPipe.cxx graphicsPipe.h graphicsStateGuardian.I \
    graphicsStateGuardian.N graphicsStateGuardian.cxx \
    graphicsStateGuardian.h graphicsWindow.I graphicsWindow.N \
    graphicsWindow.cxx graphicsWindow.h graphicsWindowInputDevice.I \
    graphicsWindowInputDevice.cxx graphicsWindowInputDevice.h \
    hardwareChannel.I hardwareChannel.cxx hardwareChannel.h \
    interactiveGraphicsPipe.I interactiveGraphicsPipe.cxx \
    interactiveGraphicsPipe.h noninteractiveGraphicsPipe.I \
    noninteractiveGraphicsPipe.cxx noninteractiveGraphicsPipe.h \
    pipeSpec.I pipeSpec.cxx pipeSpec.h savedFrameBuffer.I \
    savedFrameBuffer.cxx savedFrameBuffer.h textureContext.I \
    textureContext.cxx textureContext.h

  #define INSTALL_HEADERS \
    config_display.h \
    displayRegion.I displayRegion.h displayRegionStack.I \
    displayRegionStack.h frameBufferStack.I frameBufferStack.h \
    graphicsChannel.I graphicsChannel.h graphicsLayer.I graphicsLayer.h \
    graphicsPipe.I graphicsPipe.h graphicsStateGuardian.I \
    graphicsStateGuardian.h graphicsWindow.I graphicsWindow.h \
    graphicsWindowInputDevice.I graphicsWindowInputDevice.h \
    hardwareChannel.I hardwareChannel.h interactiveGraphicsPipe.I \
    interactiveGraphicsPipe.h noninteractiveGraphicsPipe.I \
    noninteractiveGraphicsPipe.h pipeSpec.I pipeSpec.h renderBuffer.h \
    savedFrameBuffer.I savedFrameBuffer.h textureContext.I \
    textureContext.h

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_display
  #define LOCAL_LIBS \
    display putil

  #define SOURCES \
    test_display.cxx

#end test_bin_target

