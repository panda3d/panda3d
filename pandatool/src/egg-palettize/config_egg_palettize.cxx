// Filename: config_egg_palettize.cxx
// Created by:  drose (01Dec00)
// 
////////////////////////////////////////////////////////////////////

#include "config_egg_palettize.h"
#include "palettizer.h"
#include "eggFile.h"
#include "paletteGroup.h"
#include "paletteGroups.h"
#include "textureReference.h"
#include "textureProperties.h"
#include "imageFile.h"
#include "sourceTextureImage.h"
#include "destTextureImage.h"
#include "textureImage.h"
#include "paletteImage.h"
#include "texturePlacement.h"
#include "texturePosition.h"
#include "palettePage.h"

#include <dconfig.h>

Configure(config_egg_palettize);

ConfigureFn(config_egg_palettize) {
  Palettizer::init_type();
  EggFile::init_type();
  PaletteGroup::init_type();
  PaletteGroups::init_type();
  TextureReference::init_type();
  TextureProperties::init_type();
  ImageFile::init_type();
  SourceTextureImage::init_type();
  DestTextureImage::init_type();
  TextureImage::init_type();
  PaletteImage::init_type();
  TexturePlacement::init_type();
  TexturePosition::init_type();
  PalettePage::init_type();

  // Registration of writable object's creation functions with
  // BamReader's factory
  Palettizer::register_with_read_factory();
  EggFile::register_with_read_factory();
  PaletteGroup::register_with_read_factory();
  PaletteGroups::register_with_read_factory();
  TextureReference::register_with_read_factory();
  TextureProperties::register_with_read_factory();
  SourceTextureImage::register_with_read_factory();
  DestTextureImage::register_with_read_factory();
  TextureImage::register_with_read_factory();
  PaletteImage::register_with_read_factory();
  TexturePlacement::register_with_read_factory();
  TexturePosition::register_with_read_factory();
  PalettePage::register_with_read_factory();
}
