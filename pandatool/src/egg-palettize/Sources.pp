#begin bin_target
  #define TARGET egg-palettize
  #define LOCAL_LIBS \
    eggbase progbase
  #define OTHER_LIBS \
    egg:c loader:c linmath:c putil:c express:c pnmimage:c pnmimagetypes:c \
    pandaegg:m panda:m pandaexpress:m \
    dtoolutil:c dtoolbase:c dconfig:c dtoolconfig:m dtool:m pystub
  #define UNIX_SYS_LIBS \
    m

  #define SOURCES \
    config_egg_palettize.cxx config_egg_palettize.h \
    destTextureImage.cxx destTextureImage.h \
    eggFile.cxx eggFile.h eggPalettize.cxx eggPalettize.h \
    filenameUnifier.cxx filenameUnifier.h \
    imageFile.cxx imageFile.h omitReason.cxx omitReason.h \
    pal_string_utils.cxx pal_string_utils.h \
    paletteGroup.h paletteGroup.cxx \
    paletteGroups.h paletteGroups.cxx paletteImage.h paletteImage.cxx \
    palettePage.cxx palettePage.h \
    palettizer.cxx palettizer.h \
    sourceTextureImage.cxx sourceTextureImage.h \
    textureImage.cxx textureImage.h \
    textureMemoryCounter.cxx textureMemoryCounter.h \
    texturePlacement.cxx texturePlacement.h \
    texturePosition.cxx texturePosition.h \
    textureProperties.cxx textureProperties.h textureReference.cxx \
    textureReference.h textureRequest.h textureRequest.cxx \
    txaFile.cxx txaFile.h \
    txaLine.cxx txaLine.h

#end bin_target

