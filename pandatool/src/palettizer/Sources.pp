#begin ss_lib_target
  #define TARGET p3palettizer
  #define LOCAL_LIBS \
    p3pandatoolbase

  #define OTHER_LIBS \
    p3egg:c pandaegg:m \
    p3pipeline:c p3event:c p3pstatclient:c panda:m \
    p3pandabase:c p3pnmimage:c p3mathutil:c p3linmath:c p3putil:c p3express:c \
    p3interrogatedb:c p3prc:c p3dconfig:c p3dtoolconfig:m \
    p3dtoolutil:c p3dtoolbase:c p3dtool:m \
    $[if $[WANT_NATIVE_NET],p3nativenet:c] \
    $[if $[and $[HAVE_NET],$[WANT_NATIVE_NET]],p3net:c p3downloader:c]
  
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
