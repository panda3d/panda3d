#begin bin_target
  #define TARGET egg-palettize
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    egg:c pgraph:c linmath:c putil:c express:c pnmimage:c pnmimagetypes:c \
    event:c mathutil:c \
    pandaegg:m panda:m pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS m
  
  #define COMBINED_SOURCES $[TARGET]_composite1.cxx     

  #define SOURCES \
     config_egg_palettize.h destTextureImage.h eggFile.h \
     eggPalettize.h filenameUnifier.h imageFile.h omitReason.h \
     pal_string_utils.h paletteGroup.h \
     paletteGroups.h paletteImage.h \
     palettePage.h palettizer.h sourceTextureImage.h \
     textureImage.h textureMemoryCounter.h texturePlacement.h \
     texturePosition.h textureProperties.h \
     textureReference.h textureRequest.h \
     txaFile.h txaLine.h 

  #define INCLUDED_SOURCES \
     config_egg_palettize.cxx destTextureImage.cxx eggFile.cxx \
     eggPalettize.cxx filenameUnifier.cxx imageFile.cxx \
     omitReason.cxx pal_string_utils.cxx paletteGroup.cxx \   
     paletteGroups.cxx paletteImage.cxx palettePage.cxx \
     palettizer.cxx sourceTextureImage.cxx textureImage.cxx \
     textureMemoryCounter.cxx texturePlacement.cxx \
     texturePosition.cxx textureProperties.cxx \
     textureReference.cxx textureRequest.cxx txaFile.cxx \
     txaLine.cxx 


#end bin_target

