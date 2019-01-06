/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureMemoryCounter.h
 * @author drose
 * @date 2000-12-19
 */

#ifndef TEXTUREMEMORYCOUNTER_H
#define TEXTUREMEMORYCOUNTER_H

#include "pandatoolbase.h"

class ImageFile;
class PaletteImage;
class TextureImage;
class DestTextureImage;
class TexturePlacement;

#include "pmap.h"
#include "pset.h"

/**
 * This class is used to gather statistics on texture memory usage, etc.  It
 * adds up the total texture memory required by a number of image files, and
 * reports it at the end.
 */
class TextureMemoryCounter {
public:
  TextureMemoryCounter();

  void reset();
  void add_placement(TexturePlacement *placement);

  void report(std::ostream &out, int indent_level);

private:
  static std::ostream &format_memory_fraction(std::ostream &out, int fraction_bytes,
                                         int palette_bytes);
  void add_palette(PaletteImage *image);
  void add_texture(TextureImage *texture, int bytes);
  int count_bytes(ImageFile *image);
  int count_bytes(ImageFile *image, int x_size, int y_size);

  int _num_textures;
  int _num_placed;
  int _num_unplaced;
  int _num_palettes;

  int _bytes;
  int _unused_bytes;
  int _duplicate_bytes;
  int _coverage_bytes;

  typedef pmap<TextureImage *, int> Textures;
  Textures _textures;

  typedef pset<PaletteImage *> Palettes;
  Palettes _palettes;
};

#endif
