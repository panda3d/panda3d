// Filename: config_egg_palettize.cxx
// Created by:  drose (01Dec00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
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

#include "dconfig.h"

Configure(config_egg_palettize);

ConfigureFn(config_egg_palettize) {
  init_egg_palettize();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libegg_palettize
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_egg_palettize() {
  static bool initialized = false;
  if (initialized) {
    return;
  }
  initialized = true;

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
