#begin bin_target
  #define TARGET egg-palettize
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    egg:c loader:c linmath:c putil:c express:c pnmimage:c pnmimagetypes:c \
    pandaegg:m panda:m pandaexpress:m \
    dtoolutil:c dconfig:c dtool:m pystub

  #define SOURCES \
    config_egg_palettize.cxx config_egg_palettize.h \
    eggFile.cxx eggFile.h eggPalettize.cxx eggPalettize.h \
    imageFile.cxx imageFile.h omitReason.cxx omitReason.h \
    paletteGroup.h paletteGroup.cxx \
    paletteGroups.h paletteGroups.cxx paletteImage.h paletteImage.cxx \
    palettePage.cxx palettePage.h \
    palettizer.cxx palettizer.h \
    sourceTextureImage.cxx sourceTextureImage.h string_utils.cxx \
    string_utils.h textureImage.cxx textureImage.h \
    texturePlacement.cxx texturePlacement.h \
    texturePosition.cxx texturePosition.h \
    textureProperties.cxx textureProperties.h textureReference.cxx \
    textureReference.h textureRequest.h textureRequest.cxx \
    txaFile.cxx txaFile.h \
    txaLine.cxx txaLine.h

#end bin_target

