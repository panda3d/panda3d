#define OTHER_LIBS interrogatedb:c dconfig:c dtoolconfig:m \
                   dtoolutil:c dtoolbase:c dtool:m
                   
#begin lib_target
  #define TARGET display
  #define LOCAL_LIBS \
    putil gsgbase gobj linmath graph mathutil sgraph \
    pstatclient

  #define COMBINED_SOURCES $[TARGET]_composite1.cxx $[TARGET]_composite2.cxx 
 
  #define SOURCES  \
     config_display.h displayRegion.I displayRegion.h  \
     graphicsChannel.I graphicsChannel.h graphicsLayer.I  \
     graphicsLayer.h graphicsPipe.I graphicsPipe.N graphicsPipe.h  \
     graphicsStateGuardian.I graphicsStateGuardian.N  \
     graphicsStateGuardian.h graphicsWindow.I graphicsWindow.N  \
     graphicsWindow.h graphicsWindowInputDevice.I  \
     graphicsWindowInputDevice.h hardwareChannel.I  \
     hardwareChannel.h interactiveGraphicsPipe.I  \
     interactiveGraphicsPipe.h noninteractiveGraphicsPipe.I  \
     noninteractiveGraphicsPipe.h pipeSpec.I pipeSpec.h  \
     savedFrameBuffer.I savedFrameBuffer.h textureContext.I  \
     textureContext.h  
     
 #define INCLUDED_SOURCES  \
     config_display.cxx displayRegion.cxx graphicsChannel.cxx  \
     graphicsLayer.cxx graphicsPipe.cxx graphicsStateGuardian.cxx  \
     graphicsWindow.cxx graphicsWindowInputDevice.cxx  \
     hardwareChannel.cxx interactiveGraphicsPipe.cxx  \
     noninteractiveGraphicsPipe.cxx pipeSpec.cxx  \
     savedFrameBuffer.cxx textureContext.cxx 

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

//  #define PRECOMPILED_HEADER display_headers.h    

  #define IGATESCAN all

#end lib_target

#begin test_bin_target
  #define TARGET test_display
  #define LOCAL_LIBS \
    display putil

  #define SOURCES \
    test_display.cxx

#end test_bin_target

