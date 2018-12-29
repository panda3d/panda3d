if(NOT HAVE_EGG)
  return()
endif()

set(P3PALETTIZER_HEADERS
  config_palettizer.h
  destTextureImage.h
  eggFile.h
  filenameUnifier.h
  imageFile.h
  omitReason.h
  paletteGroup.h
  paletteGroups.h
  paletteImage.h
  palettePage.h
  palettizer.h
  pal_string_utils.h
  sourceTextureImage.h
  textureImage.h
  textureMemoryCounter.h
  texturePlacement.h
  texturePosition.h
  textureProperties.h
  textureReference.h
  textureRequest.h
  txaFile.h
  txaLine.h
)

set(P3PALETTIZER_SOURCES
  config_palettizer.cxx
  destTextureImage.cxx
  eggFile.cxx
  filenameUnifier.cxx
  imageFile.cxx
  omitReason.cxx
  paletteGroup.cxx
  paletteGroups.cxx
  paletteImage.cxx
  palettePage.cxx
  palettizer.cxx
  pal_string_utils.cxx
  sourceTextureImage.cxx
  textureImage.cxx
  textureMemoryCounter.cxx
  texturePlacement.cxx
  texturePosition.cxx
  textureProperties.cxx
  textureReference.cxx
  textureRequest.cxx
  txaFile.cxx
  txaLine.cxx
)

composite_sources(p3palettizer P3PALETTIZER_SOURCES)
add_library(p3palettizer STATIC ${P3PALETTIZER_HEADERS} ${P3PALETTIZER_SOURCES})
target_link_libraries(p3palettizer p3progbase p3converter)

# This is only needed for binaries in the pandatool package. It is not useful
# for user applications, so it is not installed.
