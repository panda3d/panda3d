// Filename: config_palettizer.cxx
// Created by:  drose (12Sep03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "config_palettizer.h"
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

Configure(config_palettizer);

ConfigureFn(config_palettizer) {
  init_palettizer();
}

////////////////////////////////////////////////////////////////////
//     Function: init_libpalettizer
//  Description: Initializes the library.  This must be called at
//               least once before any of the functions or classes in
//               this library can be used.  Normally it will be
//               called by the static initializers and need not be
//               called explicitly, but special cases exist.
////////////////////////////////////////////////////////////////////
void
init_palettizer() {
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
