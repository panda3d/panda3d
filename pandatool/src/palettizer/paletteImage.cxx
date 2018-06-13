/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file paletteImage.cxx
 * @author drose
 * @date 2000-12-01
 */

#include "paletteImage.h"
#include "palettePage.h"
#include "paletteGroup.h"
#include "texturePlacement.h"
#include "palettizer.h"
#include "textureImage.h"
#include "sourceTextureImage.h"
#include "filenameUnifier.h"

#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "string_utils.h"

#include <algorithm>

TypeHandle PaletteImage::_type_handle;

/**
 * The default constructor is only for the convenience of the bam reader.
 */
PaletteImage::ClearedRegion::
ClearedRegion() {
  _x = 0;
  _y = 0;
  _x_size = 0;
  _y_size = 0;
}

/**
 *
 */
PaletteImage::ClearedRegion::
ClearedRegion(TexturePlacement *placement) {
  _x = placement->get_placed_x();
  _y = placement->get_placed_y();
  _x_size = placement->get_placed_x_size();
  _y_size = placement->get_placed_y_size();
}

/**
 *
 */
PaletteImage::ClearedRegion::
ClearedRegion(const PaletteImage::ClearedRegion &copy) :
  _x(copy._x),
  _y(copy._y),
  _x_size(copy._x_size),
  _y_size(copy._y_size)
{
}

/**
 *
 */
void PaletteImage::ClearedRegion::
operator = (const PaletteImage::ClearedRegion &copy) {
  _x = copy._x;
  _y = copy._y;
  _x_size = copy._x_size;
  _y_size = copy._y_size;
}

/**
 * Sets the appropriate region of the image to black.
 */
void PaletteImage::ClearedRegion::
clear(PNMImage &image) {
  LRGBColorf rgb(pal->_background[0], pal->_background[1], pal->_background[2]);
  float alpha = pal->_background[3];

  for (int y = _y; y < _y + _y_size; y++) {
    for (int x = _x; x < _x + _x_size; x++) {
      image.set_xel(x, y, rgb);
    }
  }
  if (image.has_alpha()) {
    for (int y = _y; y < _y + _y_size; y++) {
      for (int x = _x; x < _x + _x_size; x++) {
        image.set_alpha(x, y, alpha);
      }
    }
  }
}

/**
 * Writes the contents of the ClearedRegion to the indicated datagram.
 */
void PaletteImage::ClearedRegion::
write_datagram(Datagram &datagram) const {
  datagram.add_int32(_x);
  datagram.add_int32(_y);
  datagram.add_int32(_x_size);
  datagram.add_int32(_y_size);
}

/**
 * Extracts the contents of the ClearedRegion from the indicated datagram.
 */
void PaletteImage::ClearedRegion::
fillin(DatagramIterator &scan) {
  _x = scan.get_int32();
  _y = scan.get_int32();
  _x_size = scan.get_int32();
  _y_size = scan.get_int32();
}






/**
 * The default constructor is only for the convenience of the Bam reader.
 */
PaletteImage::
PaletteImage() {
  _page = nullptr;
  _index = 0;
  _new_image = false;
  _got_image = false;

  _swapped_image = 0;
}

/**
 *
 */
PaletteImage::
PaletteImage(PalettePage *page, int index) :
  _page(page),
  _index(index)
{
  _properties = page->get_properties();
  _size_known = true;
  _x_size = pal->_pal_x_size;
  _y_size = pal->_pal_y_size;
  _new_image = true;
  _got_image = false;
  _swapped_image = 0;

  setup_filename();
}

/**
 *
 */
PaletteImage::
PaletteImage(PalettePage *page, int index, unsigned swapIndex) :
  _page(page),
  _index(index),
  _swapped_image(swapIndex)
{
  _properties = page->get_properties();
  _size_known = true;
  _x_size = pal->_pal_x_size;
  _y_size = pal->_pal_y_size;
  _new_image = true;
  _got_image = false;

  setup_filename();
}


/**
 * Returns the particular PalettePage this image is associated with.
 */
PalettePage *PaletteImage::
get_page() const {
  return _page;
}

/**
 * Returns true if there are no textures, or only one "solitary" texture,
 * placed on the image.  In either case, the PaletteImage need not be
 * generated.
 */
bool PaletteImage::
is_empty() const {
  if (_placements.empty()) {
    // The image is genuinely empty.
    return true;

  } else if (_placements.size() == 1) {
    // If the image has exactly one texture, we consider the image empty only
    // if the texture is actually flagged as 'solitary'.
    return (_placements[0]->get_omit_reason() == OR_solitary);

  } else {
    // The image has more than one texture, so it's definitely not empty.
    return false;
  }
}

/**
 * Returns the fraction of the PaletteImage that is actually used by any
 * textures.  This is 1.0 if every pixel in the PaletteImage is used, or 0.0
 * if none are.  Normally it will be somewhere in between.
 */
double PaletteImage::
count_utilization() const {
  int used_pixels = 0;

  Placements::const_iterator pi;
  for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
    TexturePlacement *placement = (*pi);

    int texture_pixels =
      placement->get_placed_x_size() *
      placement->get_placed_y_size();
    used_pixels += texture_pixels;
  }

  int total_pixels = get_x_size() * get_y_size();

  return (double)used_pixels / (double)total_pixels;
}

/**
 * Returns the a weighted average of the fraction of coverage represented by
 * all of the textures placed on the palette.  This number represents the
 * fraction of wasted pixels in the palette image consumed by copying the same
 * pixels multiple times into the palette, or if the number is negative, it
 * represents the fraction of pixels saved by not having to copy the entire
 * texture into the palette.
 */
double PaletteImage::
count_coverage() const {
  int coverage_pixels = 0;

  Placements::const_iterator pi;
  for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
    TexturePlacement *placement = (*pi);
    TextureImage *texture = placement->get_texture();
    nassertr(texture != nullptr, 0.0);

    int orig_pixels =
      texture->get_x_size() *
      texture->get_y_size();
    int placed_pixels =
      placement->get_placed_x_size() *
      placement->get_placed_y_size();

    coverage_pixels += placed_pixels - orig_pixels;
  }

  int total_pixels = get_x_size() * get_y_size();

  return (double)coverage_pixels / (double)total_pixels;
}

/**
 * Attempts to place the indicated texture on the image.  Returns true if
 * successful, or false if there was no available space.
 */
bool PaletteImage::
place(TexturePlacement *placement) {
  nassertr(placement->is_size_known(), true);
  nassertr(!placement->is_placed(), true);

  int x, y;
  if (find_hole(x, y, placement->get_x_size(), placement->get_y_size())) {
    placement->place_at(this, x, y);
    _placements.push_back(placement);

    // [gjeon] create swappedImages
    TexturePlacement::TextureSwaps::iterator tsi;
    for (tsi = placement->_textureSwaps.begin(); tsi != placement->_textureSwaps.end(); ++tsi) {
      if ((tsi - placement->_textureSwaps.begin()) >= (int)_swappedImages.size()) {
        PaletteImage *swappedImage = new PaletteImage(_page, _swappedImages.size(), tsi - placement->_textureSwaps.begin() + 1);
        swappedImage->_masterPlacements = &_placements;
        _swappedImages.push_back(swappedImage);
      }
    }

    return true;
  }

  return false;
}

/**
 * Removes the texture from the image.
 */
void PaletteImage::
unplace(TexturePlacement *placement) {
  nassertv(placement->is_placed() && placement->get_image() == this);

  Placements::iterator pi;
  pi = find(_placements.begin(), _placements.end(), placement);
  while (pi != _placements.end()) {
    _placements.erase(pi);
    pi = find(_placements.begin(), _placements.end(), placement);
  }
  _cleared_regions.push_back(ClearedRegion(placement));
}

/**
 * To be called after all textures have been placed on the image, this checks
 * to see if there is only one texture on the image.  If there is, it is
 * flagged as 'solitary' so that the egg files will not needlessly reference
 * the palettized image.
 *
 * However, if pal->_omit_solitary is false, we generally don't change
 * textures to solitary state.
 */
void PaletteImage::
check_solitary() {
  if (_placements.size() == 1) {
    // How sad, only one.
    TexturePlacement *placement = *_placements.begin();
    nassertv(placement->get_omit_reason() == OR_none ||
             placement->get_omit_reason() == OR_solitary);

    if (pal->_omit_solitary || placement->get_omit_reason() == OR_solitary) {
      // We only omit the solitary texture if (a) we have omit_solitary in
      // effect, or (b) we don't have omit_solitary in effect now, but we did
      // before, and the texture is still flagged as solitary from that
      // previous pass.
      placement->omit_solitary();
    }

  } else {
    // Zero or multiple.
    Placements::const_iterator pi;
    for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
      TexturePlacement *placement = (*pi);
      /*
      if (!(placement->get_omit_reason() == OR_none ||
            placement->get_omit_reason() == OR_solitary)) {
        nout << "texture " << *placement->get_texture() << " is omitted for "
             << placement->get_omit_reason() << "\n";
      }
      */
      nassertv(placement->get_omit_reason() == OR_none ||
               placement->get_omit_reason() == OR_solitary);
      placement->not_solitary();
    }
  }
}

/**
 * Attempts to resize the palette image to as small as it can go.
 */
void PaletteImage::
optimal_resize() {
  if (is_empty()) { // && (_swapped_image == 0)) {
    return;
  }

  bool resized_any = false;
  bool success;
  do {
    success = false;
    nassertv(_x_size > 0 && _y_size > 0);

    // Try to cut it in half in both dimensions, one at a time.
    if (resize_image(_x_size, _y_size / 2)) {
      success = true;
      resized_any = true;
    }
    if (resize_image(_x_size / 2, _y_size)) {
      success = true;
      resized_any = true;
    }

  } while (success);

  if (resized_any) {
    nout << "Resizing "
         << FilenameUnifier::make_user_filename(get_filename()) << " to "
         << _x_size << " " << _y_size << "\n";

    // [gjeon] resize swapped images, also
    SwappedImages::iterator si;
    for (si = _swappedImages.begin(); si != _swappedImages.end(); ++si) {
      PaletteImage *swappedImage = (*si);
      swappedImage->resize_swapped_image(_x_size, _y_size);
    }
  }
}

/**
 * Attempts to resize the palette image, and repack all of the textures within
 * the new size.  Returns true if successful, false otherwise.  If this fails,
 * it will still result in repacking all the textures in the original size.
 */
bool PaletteImage::
resize_image(int x_size, int y_size) {
  // We already know we're going to be generating a new image from scratch
  // after this.
  _cleared_regions.clear();
  remove_image();

  // First, Save the current placement list, while simultaneously clearing it.
  Placements saved;
  saved.swap(_placements);

  // Also save our current size.
  int saved_x_size = _x_size;
  int saved_y_size = _y_size;

  // Then, sort the textures to in order from biggest to smallest, as an aid
  // to optimal packing.
  sort(saved.begin(), saved.end(), SortPlacementBySize());

  // And while we're at it, we need to officially unplace each of these.
  Placements::iterator pi;
  for (pi = saved.begin(); pi != saved.end(); ++pi) {
    (*pi)->force_replace();
  }

  // Finally, apply the new size and try to fit all the textures.
  _x_size = x_size;
  _y_size = y_size;

  bool packed = true;
  for (pi = saved.begin(); pi != saved.end() && packed; ++pi) {
    if (!place(*pi)) {
      packed = false;
    }
  }

  if (!packed) {
    // If it didn't work, phooey.  Put 'em all back.
    _x_size = saved_x_size;
    _y_size = saved_y_size;

    Placements remove;
    remove.swap(_placements);
    for (pi = remove.begin(); pi != remove.end(); ++pi) {
      (*pi)->force_replace();
    }

    bool all_packed = true;
    for (pi = saved.begin(); pi != saved.end(); ++pi) {
      if (!place(*pi)) {
        all_packed = false;
      }
    }
    nassertr(all_packed, false);
  }

  return packed;
}

/**
 * Attempts to resize the palette image, and repack all of the textures within
 * the new size.  Returns true if successful, false otherwise.  If this fails,
 * it will still result in repacking all the textures in the original size.
 */
void PaletteImage::
resize_swapped_image(int x_size, int y_size) {
  // Finally, apply the new size and try to fit all the textures.
  _x_size = x_size;
  _y_size = y_size;
}

/**
 * Writes a list of the textures that have been placed on this image to the
 * indicated output stream, one per line.
 */
void PaletteImage::
write_placements(std::ostream &out, int indent_level) const {
  Placements::const_iterator pi;
  for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
    TexturePlacement *placement = (*pi);
    placement->write_placed(out, indent_level);
  }
}

/**
 * Unpacks each texture that has been placed on this image, resetting the
 * image to empty.
 */
void PaletteImage::
reset_image() {
  // We need a copy so we can modify this list as we traverse it.
  Placements copy_placements = _placements;
  Placements::const_iterator pi;
  for (pi = copy_placements.begin(); pi != copy_placements.end(); ++pi) {
    TexturePlacement *placement = (*pi);
    placement->force_replace();
  }

  _placements.clear();
  _cleared_regions.clear();
  remove_image();
}

/**
 * Ensures the _shadow_image has the correct filename and image types, based
 * on what was supplied on the command line and in the .txa file.
 */
void PaletteImage::
setup_shadow_image() {
  _shadow_image.make_shadow_image(_basename);

  // [gjeon] setup shadoe_image of swappedImages
  SwappedImages::iterator si;
  for (si = _swappedImages.begin(); si != _swappedImages.end(); ++si) {
    PaletteImage *swappedImage = (*si);
    swappedImage->setup_shadow_image();
  }
}

/**
 * If the palette has changed since it was last written out, updates the image
 * and writes out a new one.  If redo_all is true, regenerates the image from
 * scratch and writes it out again, whether it needed it or not.
 */
void PaletteImage::
update_image(bool redo_all) {
  if (is_empty() && pal->_aggressively_clean_mapdir) {
    // If the palette image is 'empty', ensure that it doesn't exist.  No need
    // to clutter up the map directory.
    remove_image();
    return;
  }

  if (redo_all) {
    // If we're redoing everything, throw out the old image anyway.
    remove_image();
  }

  // Check the filename too.
  update_filename();

  // Do we need to update?
  bool needs_update =
    _new_image || !exists() ||
    !_cleared_regions.empty();

  Placements::iterator pi;
  // We must continue to walk through all of the textures on the palette, even
  // after we discover the palette requires an update, so we can determine
  // which source images need to be recopied.
  for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
    TexturePlacement *placement = (*pi);

    if (!placement->is_filled()) {
      needs_update = true;

    } else {
      TextureImage *texture = placement->get_texture();

      // Only check the timestamps on textures that are named (indirectly) on
      // the command line.
      if (texture->is_texture_named()) {
        SourceTextureImage *source = texture->get_preferred_source();

        if (source != nullptr &&
            source->get_filename().compare_timestamps(get_filename()) > 0) {
          // The source image is newer than the palette image; we need to
          // regenerate.
          placement->mark_unfilled();
          needs_update = true;
        }
      }

      // [gjeon] to find out all of the swappable textures is up to date
      TexturePlacement::TextureSwaps::iterator tsi;
      for (tsi = placement->_textureSwaps.begin(); tsi != placement->_textureSwaps.end(); ++tsi) {
        TextureImage *swapTexture = (*tsi);

        if (swapTexture->is_texture_named()) {
          SourceTextureImage *sourceSwapTexture = swapTexture->get_preferred_source();

          if (sourceSwapTexture != nullptr &&
              sourceSwapTexture->get_filename().compare_timestamps(get_filename()) > 0) {
            // The source image is newer than the palette image; we need to
            // regenerate.
            placement->mark_unfilled();
            needs_update = true;
          }
        }
      }

    }
  }

  if (!needs_update) {
    // No sweat; nothing has changed.
    return;
  }

  get_image();
  // [gjeon] get swapped images, too
  get_swapped_images();

  // Set to black any parts of the image that we recently unplaced.
  ClearedRegions::iterator ci;
  for (ci = _cleared_regions.begin(); ci != _cleared_regions.end(); ++ci) {
    ClearedRegion &region = (*ci);
    region.clear(_image);

    // [gjeon] clear swapped images also
    SwappedImages::iterator si;
    for (si = _swappedImages.begin(); si != _swappedImages.end(); ++si) {
      PaletteImage *swappedImage = (*si);
      region.clear(swappedImage->_image);
    }
  }
  _cleared_regions.clear();

  // Now add the recent additions to the image.
  for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
    TexturePlacement *placement = (*pi);
    if (!placement->is_filled()) {
      placement->fill_image(_image);

      // [gjeon] fill swapped images
      SwappedImages::iterator si;
      for (si = _swappedImages.begin(); si != _swappedImages.end(); ++si) {
        PaletteImage *swappedImage = (*si);
        swappedImage->update_filename();
        placement->fill_swapped_image(swappedImage->_image, si - _swappedImages.begin());
      }
    }
  }

  write(_image);

  if (pal->_shadow_color_type != nullptr) {
    _shadow_image.write(_image);
  }

  release_image();

  // [gjeon] write and release swapped images
  SwappedImages::iterator si;
  for (si = _swappedImages.begin(); si != _swappedImages.end(); ++si) {
    PaletteImage *swappedImage = (*si);
    swappedImage->write(swappedImage->_image);
    if (pal->_shadow_color_type != nullptr) {
      swappedImage->_shadow_image.write(swappedImage->_image);
    }
    swappedImage->release_image();
  }
}

/**
 * Changes the image filename to match the current naming scheme, assuming
 * something has changed since the image was created.  Returns true if the
 * image filename changes (which means update_image() should be called).
 */
bool PaletteImage::
update_filename() {
  Filename orig_filename = _filename;
  Filename orig_alpha_filename = _alpha_filename;
  Filename orig_shadow_filename = _shadow_image.get_filename();

  if (setup_filename()) {
    nout << "Renaming " << FilenameUnifier::make_user_filename(orig_filename)
         << " to " << FilenameUnifier::make_user_filename(_filename) << "\n";

    if (!orig_filename.empty() && orig_filename.exists()) {
      nout << "Deleting " << FilenameUnifier::make_user_filename(orig_filename) << "\n";
      orig_filename.unlink();
    }
    if (!orig_alpha_filename.empty() && orig_alpha_filename.exists()) {
      nout << "Deleting " << FilenameUnifier::make_user_filename(orig_alpha_filename) << "\n";
      orig_alpha_filename.unlink();
    }
    if (!orig_shadow_filename.empty() && orig_shadow_filename.exists()) {
      nout << "Deleting " << FilenameUnifier::make_user_filename(orig_shadow_filename) << "\n";
      orig_shadow_filename.unlink();
    }
    _new_image = true;

    // Since the palette filename has changed, we need to mark all of the egg
    // files that referenced the old filename as stale.

    // Marking egg files stale at this late point can cause minor problems;
    // because we might do this, it's necessary for eggPalettize.cxx to call
    // read_stale_eggs() twice.
    Placements::iterator pi;
    for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
      TexturePlacement *placement = (*pi);
      placement->mark_eggs_stale();
    }

    return true;
  }

  return false;
}

/**
 * Sets up the image's filename (and that of the _shadow_pal) according to the
 * specified properties.
 *
 * Returns true if the filename changes from what it was previously, false
 * otherwise.
 */
bool PaletteImage::
setup_filename() {
  // Build up the basename for the palette image, based on the supplied image
  // pattern.
  _basename = std::string();

  std::string::iterator si = pal->_generated_image_pattern.begin();
  while (si != pal->_generated_image_pattern.end()) {
    if ((*si) == '%') {
      // Some keycode.
      ++si;
      if (si != pal->_generated_image_pattern.end()) {
        switch (*si) {
        case '%':
          _basename += '%';
          break;

        case 'g':
          _basename += _page->get_group()->get_name();
          break;

        case 'p':
          _basename += _page->get_name();
          break;

        case 'i':
          _basename += format_string(_index + 1);
          break;

        default:
          _basename += '%';
          _basename += (*si);
        }
        ++si;
      }
    } else {
      // A literal character.
      _basename += (*si);
      ++si;
    }
  }

  if (_swapped_image > 0) {
    _basename += "_swp_";
    _basename += format_string(_swapped_image);
  }

  // We must end the basename with a dot, so that it does not appear to have a
  // filename extension.  Otherwise, an embedded dot in the group's name would
  // make everything following appear to be an extension, which would get lost
  // in the set_filename() call.
  if (_basename.empty() || _basename[_basename.length() - 1] != '.') {
    _basename += '.';
  }

  bool any_changed = false;

  if (set_filename(_page->get_group(), _basename)) {
    any_changed = true;
  }

  if (_shadow_image.make_shadow_image(_basename)) {
    any_changed = true;
  }

  return any_changed;
}

/**
 * Searches for a hole of at least x_size by y_size pixels somewhere within
 * the PaletteImage.  If a suitable hole is found, sets x and y to the top
 * left corner and returns true; otherwise, returns false.
 */
bool PaletteImage::
find_hole(int &x, int &y, int x_size, int y_size) const {
  y = 0;
  while (y + y_size <= _y_size) {
    int next_y = _y_size;
    // Scan along the row at 'y'.
    x = 0;
    while (x + x_size <= _x_size) {
      int next_x = x;

      // Consider the spot at x, y.
      TexturePlacement *overlap = find_overlap(x, y, x_size, y_size);

      if (overlap == nullptr) {
        // Hooray!
        return true;
      }

      next_x = overlap->get_placed_x() + overlap->get_placed_x_size();
      next_y = std::min(next_y, overlap->get_placed_y() + overlap->get_placed_y_size());
      nassertr(next_x > x, false);
      x = next_x;
    }

    nassertr(next_y > y, false);
    y = next_y;
  }

  // Nope, wouldn't fit anywhere.
  return false;
}

/**
 * If the rectangle whose top left corner is x, y and whose size is x_size,
 * y_size describes an empty hole that does not overlap any placed images,
 * returns NULL; otherwise, returns the first placed texture that the image
 * does overlap.  It is assumed the rectangle lies completely within the
 * boundaries of the image itself.
 */
TexturePlacement *PaletteImage::
find_overlap(int x, int y, int x_size, int y_size) const {
  Placements::const_iterator pi;
  for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
    TexturePlacement *placement = (*pi);
    if (placement->is_placed() &&
        placement->intersects(x, y, x_size, y_size)) {
      return placement;
    }
  }

  return nullptr;
}

/**
 * Reads or generates the PNMImage that corresponds to the palette as it is
 * known so far.
 */
void PaletteImage::
get_image() {
  if (_got_image) {
    return;
  }

  if (!_new_image) {
    if (pal->_shadow_color_type != nullptr) {
      if (_shadow_image.get_filename().exists() && _shadow_image.read(_image)) {
        _got_image = true;
        return;
      }
    } else {
      if (get_filename().exists() && read(_image)) {
        _got_image = true;
        return;
      }
    }
  }

  nout << "Generating new "
       << FilenameUnifier::make_user_filename(get_filename()) << "\n";

  // We won't be using this any more.
  _cleared_regions.clear();

  _image.clear(get_x_size(), get_y_size(), _properties.get_num_channels());
  _image.fill(pal->_background[0], pal->_background[1], pal->_background[2]);
  if (_image.has_alpha()) {
    _image.alpha_fill(pal->_background[3]);
  }

  _new_image = false;
  _got_image = true;

  // Now fill up the image.
  Placements::iterator pi;
  for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
    TexturePlacement *placement = (*pi);
    placement->fill_image(_image);
  }
}

/**
 * Reads or generates the PNMImage for swapped textures
 */
void PaletteImage::
get_swapped_image(int index) {
  if (_got_image) {
    return;
  }

  if (!_new_image) {
    if (pal->_shadow_color_type != nullptr) {
      if (_shadow_image.get_filename().exists() && _shadow_image.read(_image)) {
        _got_image = true;
        return;
      }
    } else {
      if (get_filename().exists() && read(_image)) {
        _got_image = true;
        return;
      }
    }
  }

  nout << "Generating new "
       << FilenameUnifier::make_user_filename(get_filename()) << "\n";

  // We won't be using this any more.
  _cleared_regions.clear();

  _image.clear(get_x_size(), get_y_size(), _properties.get_num_channels());
  _image.fill(pal->_background[0], pal->_background[1], pal->_background[2]);
  if (_image.has_alpha()) {
    _image.alpha_fill(pal->_background[3]);
  }

  _new_image = false;
  _got_image = true;

  // Now fill up the image.
  Placements::iterator pi;
  for (pi = _masterPlacements->begin(); pi != _masterPlacements->end(); ++pi) {
    TexturePlacement *placement = (*pi);
    if ((int)placement->_textureSwaps.size() > index) {
      placement->fill_swapped_image(_image, index);
    } else {
      placement->fill_image(_image);
    }
  }
}

/**
 * Reads or generates the PNMImage that corresponds to the palette as it is
 * known so far.
 */
void PaletteImage::
get_swapped_images() {
  SwappedImages::iterator si;
  for (si = _swappedImages.begin(); si != _swappedImages.end(); ++si) {
    PaletteImage *swappedImage = (*si);
    swappedImage->get_swapped_image(si - _swappedImages.begin());
  }
}

/**
 * Deallocates the memory allocated by a previous call to get_image().
 */
void PaletteImage::
release_image() {
  _image.clear();
  _got_image = false;
}

/**
 * Deletes the image file.
 */
void PaletteImage::
remove_image() {
  unlink();
  if (pal->_shadow_color_type != nullptr) {
    _shadow_image.unlink();
  }
  _new_image = true;
}

/**
 * Registers the current object as something that can be read from a Bam file.
 */
void PaletteImage::
register_with_read_factory() {
  BamReader::get_factory()->
    register_factory(get_class_type(), make_PaletteImage);
}

/**
 * Fills the indicated datagram up with a binary representation of the current
 * object, in preparation for writing to a Bam file.
 */
void PaletteImage::
write_datagram(BamWriter *writer, Datagram &datagram) {
  ImageFile::write_datagram(writer, datagram);

  datagram.add_uint32(_cleared_regions.size());
  ClearedRegions::const_iterator ci;
  for (ci = _cleared_regions.begin(); ci != _cleared_regions.end(); ++ci) {
    (*ci).write_datagram(datagram);
  }

  datagram.add_uint32(_placements.size());
  Placements::const_iterator pi;
  for (pi = _placements.begin(); pi != _placements.end(); ++pi) {
    writer->write_pointer(datagram, (*pi));
  }

  writer->write_pointer(datagram, _page);
  datagram.add_uint32(_index);
  datagram.add_string(_basename);
  datagram.add_bool(_new_image);

  // We don't write _got_image or _image.  These are loaded per-session.

  // We don't write _shadow_image.  This is just a runtime convenience for
  // specifying the name of the shadow file, and we redefine this per-session
  // (which allows us to pick up a new pal->_shadow_dirname if it changes).
}

/**
 * Called after the object is otherwise completely read from a Bam file, this
 * function's job is to store the pointers that were retrieved from the Bam
 * file for each pointer object written.  The return value is the number of
 * pointers processed from the list.
 */
int PaletteImage::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int index = ImageFile::complete_pointers(p_list, manager);

  int i;
  _placements.reserve(_num_placements);
  for (i = 0; i < _num_placements; i++) {
    TexturePlacement *placement;
    DCAST_INTO_R(placement, p_list[index], index);
    _placements.push_back(placement);
    index++;
  }

  if (p_list[index] != nullptr) {
    DCAST_INTO_R(_page, p_list[index], index);
  }
  index++;

  return index;
}

/**
 * This method is called by the BamReader when an object of this type is
 * encountered in a Bam file; it should allocate and return a new object with
 * all the data read.
 */
TypedWritable *PaletteImage::
make_PaletteImage(const FactoryParams &params) {
  PaletteImage *me = new PaletteImage;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Reads the binary data from the given datagram iterator, which was written
 * by a previous call to write_datagram().
 */
void PaletteImage::
fillin(DatagramIterator &scan, BamReader *manager) {
  ImageFile::fillin(scan, manager);

  int num_cleared_regions = scan.get_uint32();
  _cleared_regions.reserve(num_cleared_regions);
  for (int i = 0; i < num_cleared_regions; i++) {
    _cleared_regions.push_back(ClearedRegion());
    _cleared_regions.back().fillin(scan);
  }

  _num_placements = scan.get_uint32();
  manager->read_pointers(scan, _num_placements);

  manager->read_pointer(scan);  // _page

  _index = scan.get_uint32();
  _basename = scan.get_string();
  _new_image = scan.get_bool();
}
