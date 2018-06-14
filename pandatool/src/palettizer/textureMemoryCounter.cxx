/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file textureMemoryCounter.cxx
 * @author drose
 * @date 2000-12-19
 */

#include "textureMemoryCounter.h"
#include "paletteImage.h"
#include "textureImage.h"
#include "destTextureImage.h"
#include "omitReason.h"
#include "texturePlacement.h"

#include "indent.h"
#include <math.h>

/**
 *
 */
TextureMemoryCounter::
TextureMemoryCounter() {
  reset();
}

/**
 * Resets the count to zero.
 */
void TextureMemoryCounter::
reset() {
  _num_textures = 0;
  _num_unplaced = 0;
  _num_placed = 0;
  _num_palettes = 0;

  _bytes = 0;
  _unused_bytes = 0;
  _duplicate_bytes = 0;
  _coverage_bytes = 0;
  _textures.clear();
  _palettes.clear();
}

/**
 * Adds the indicated TexturePlacement to the counter.
 */
void TextureMemoryCounter::
add_placement(TexturePlacement *placement) {
  TextureImage *texture = placement->get_texture();
  nassertv(texture != nullptr);

  if (placement->get_omit_reason() == OR_none) {
    PaletteImage *image = placement->get_image();
    nassertv(image != nullptr);
    add_palette(image);

    int bytes = count_bytes(image, placement->get_placed_x_size(),
                            placement->get_placed_y_size());
    add_texture(texture, bytes);
    _num_placed++;

  } else {
    DestTextureImage *dest = placement->get_dest();
    if (dest != nullptr) {
      int bytes = count_bytes(dest);
      add_texture(texture, bytes);

      _bytes += bytes;
      _num_unplaced++;
    }
  }
}

/**
 * Reports the measured texture memory usage.
 */
void TextureMemoryCounter::
report(std::ostream &out, int indent_level) {
  indent(out, indent_level)
    << _num_placed << " of " << _num_textures << " textures appear on "
    << _num_palettes << " palette images with " << _num_unplaced
    << " unplaced.\n";

  indent(out, indent_level)
    << (_bytes + 512) / 1024 << "k estimated texture memory required.\n";

  if (_bytes != 0) {
    if (_unused_bytes != 0) {
      indent(out, indent_level + 2);
      format_memory_fraction(out, _unused_bytes, _bytes)
        << " is wasted because of unused palette space.\n";
    }

    if (_coverage_bytes > 0) {
      indent(out, indent_level + 2);
      format_memory_fraction(out, _coverage_bytes, _bytes)
        << " is wasted for repeating textures and margins.\n";

    } else if (_coverage_bytes < 0) {
      indent(out, indent_level + 2);
      format_memory_fraction(out, -_coverage_bytes, _bytes)
        << " is *saved* for palettizing partial textures.\n";
    }

    if (_duplicate_bytes != 0) {
      indent(out, indent_level + 2);
      format_memory_fraction(out, _duplicate_bytes, _bytes)
        << " is wasted because of a texture appearing in multiple groups.\n";
    }
  }
}

/**
 * Writes to the indicated ostream an indication of the fraction of the total
 * memory usage that is represented by fraction_bytes.
 */
std::ostream &TextureMemoryCounter::
format_memory_fraction(std::ostream &out, int fraction_bytes, int palette_bytes) {
  out << floor(1000.0 * (double)fraction_bytes / (double)palette_bytes + 0.5) / 10.0
      << "% (" << (fraction_bytes + 512) / 1024 << "k)";
  return out;
}

/**
 * Adds the indicated PaletteImage to the count.  If this is called twice for
 * a given PaletteImage it does nothing.
 */
void TextureMemoryCounter::
add_palette(PaletteImage *image) {
  bool inserted = _palettes.insert(image).second;
  if (!inserted) {
    // We've already added this palette image.
    return;
  }

  int bytes = count_bytes(image);
  double unused = 1.0 - image->count_utilization();
  double coverage = image->count_coverage();

  _bytes += bytes;
  _unused_bytes += (int)(unused * bytes);
  _coverage_bytes += (int)(coverage * bytes);

  _num_palettes++;
}

/**
 * Adds the given TextureImage to the counter.  If the texture image has
 * already been added, this counts the smaller of the two as duplicate bytes.
 */
void TextureMemoryCounter::
add_texture(TextureImage *texture, int bytes) {
  std::pair<Textures::iterator, bool> result;
  result = _textures.insert(Textures::value_type(texture, bytes));
  if (result.second) {
    // If it was inserted, no problem--no duplicates.
    _num_textures++;
    return;
  }

  // If it was not inserted, we have a duplicate.
  Textures::iterator ti = result.first;

  _duplicate_bytes += std::min(bytes, (*ti).second);
  (*ti).second = std::max(bytes, (*ti).second);
}

/**
 * Attempts to estimate the number of bytes the given image file will use in
 * texture memory.
 */
int TextureMemoryCounter::
count_bytes(ImageFile *image) {
  return count_bytes(image, image->get_x_size(), image->get_y_size());
}

/**
 * Attempts to estimate the number of bytes the given image file will use in
 * texture memory.
 */
int TextureMemoryCounter::
count_bytes(ImageFile *image, int x_size, int y_size) {
  int pixels = x_size * y_size;

  // Try to guess the number of bytes per pixel this texture will consume in
  // texture memory, based on its requested format.  This is only a loose
  // guess, because this depends of course on the pecularities of the
  // particular rendering engine.
  int bpp = 0;
  switch (image->get_properties()._format) {
  case EggTexture::F_rgba12:
    bpp = 6;
    break;

  case EggTexture::F_rgba:
  case EggTexture::F_rgbm:
  case EggTexture::F_rgba8:
    bpp = 4;
    break;

  case EggTexture::F_rgb:
  case EggTexture::F_rgb12:
    bpp = 3;
    break;

  case EggTexture::F_rgba4:
  case EggTexture::F_rgba5:
  case EggTexture::F_rgb8:
  case EggTexture::F_rgb5:
  case EggTexture::F_luminance_alpha:
  case EggTexture::F_luminance_alphamask:
    bpp = 2;
    break;

  case EggTexture::F_rgb332:
  case EggTexture::F_red:
  case EggTexture::F_green:
  case EggTexture::F_blue:
  case EggTexture::F_alpha:
  case EggTexture::F_luminance:
    bpp = 1;
    break;

  default:
    bpp = image->get_num_channels();
  }

  int bytes = pixels * bpp;

  // If we're mipmapping, it's worth 13 more bytes.
  switch (image->get_properties()._minfilter) {
  case EggTexture::FT_nearest_mipmap_nearest:
  case EggTexture::FT_linear_mipmap_nearest:
  case EggTexture::FT_nearest_mipmap_linear:
  case EggTexture::FT_linear_mipmap_linear:
    bytes = (bytes * 4) / 3;
    break;

  default:
    break;
  }

  return bytes;
}
