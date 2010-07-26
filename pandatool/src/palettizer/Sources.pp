#begin ss_lib_target
  #define TARGET palettizer
  #define LOCAL_LIBS \
    pandatoolbase

  #define OTHER_LIBS \
    egg:c pandaegg:m \
    pipeline:c event:c pstatclient:c panda:m \
    pandabase:c pnmimage:c mathutil:c linmath:c putil:c express:c \
    interrogatedb:c prc:c dconfig:c dtoolconfig:m \
    dtoolutil:c dtoolbase:c dtool:m \
    $[if $[WANT_NATIVE_NET],nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],net:c downloader:c]
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     

  #define SOURCES \
     config_palettizer.h destTextureImage.h eggFile.h \
     filenameUnifier.h imageFile.h omitReason.h \
     pal_string_utils.h paletteGroup.h \
     paletteGroups.h paletteImage.h \
     palettePage.h palettizer.h sourceTextureImage.h \
     textureImage.h textureMemoryCounter.h texturePlacement.h \
     texturePosition.h textureProperties.h \
     textureReference.h textureRequest.h \
     txaFile.h txaLine.h 

  #define INCLUDED_SOURCES \
     config_palettizer.cxx destTextureImage.cxx eggFile.cxx \
     filenameUnifier.cxx imageFile.cxx \
     omitReason.cxx pal_string_utils.cxx paletteGroup.cxx \   
     paletteGroups.cxx paletteImage.cxx palettePage.cxx \
     palettizer.cxx sourceTextureImage.cxx textureImage.cxx \
     textureMemoryCounter.cxx texturePlacement.cxx \
     texturePosition.cxx textureProperties.cxx \
     textureReference.cxx textureRequest.cxx txaFile.cxx \
     txaLine.cxx 

#end ss_lib_target
