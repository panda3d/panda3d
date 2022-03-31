/**
 *
 * RenderPipeline
 *
 * Copyright (c) 2014-2016 tobspr <tobias.springer1@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */


#include "shadowAtlas.h"
#include <string.h>

NotifyCategoryDef(shadowatlas, "");

/**
 * @brief Constructs a new shadow atlas.
 * @details This constructs a new shadow atlas with the given size and tile size.
 *
 *   The size determines the total size of the atlas in pixels. It should be a
 *   power-of-two to favour the GPU.
 *
 *   The tile_size determines the smallest unit of tiles the atlas can store.
 *   If, for example, a tile_size of 32 is used, then every entry stored must
 *   have a resolution of 32 or greater, and the resolution must be a multiple
 *   of 32. This is to optimize the search in the atlas, so the atlas does not
 *   have to check every pixel, and instead can just check whole tiles.
 *
 *   If you want to disable the use of tiles, set the tile_size to 1, which
 *   will make the shadow atlas use pixels instead of tiles.
 *
 * @param size Atlas-size in pixels
 * @param tile_size tile-size in pixels, or 1 to use no tiles.
 */
ShadowAtlas::ShadowAtlas(size_t size, size_t tile_size) {
  nassertv(size > 1 && tile_size >= 1);
  nassertv(tile_size < size && size % tile_size == 0);
  _size = size;
  _tile_size = tile_size;
  _num_used_tiles = 0;
  init_tiles();
}

/**
 * @brief Destructs the shadow atlas.
 * @details This destructs the shadow atlas, freeing all used resources.
 */
ShadowAtlas::~ShadowAtlas() {
  delete [] _flags;
}

/**
 * @brief Internal method to init the storage.
 * @details This method setups the storage used for storing the tile flags.
 */
void ShadowAtlas::init_tiles() {
  _num_tiles = _size / _tile_size;
  _flags = new bool[_num_tiles * _num_tiles];
  memset(_flags, 0x0, sizeof(bool) * _num_tiles * _num_tiles);
}

/**
 * @brief Internal method to reserve a region in the atlas.
 * @details This reserves a given region in the shadow atlas. The region should
 *   be in tile space.This is called by the ShadowAtlas::find_and_reserve_region.
 *   It sets all flags in that region to true, indicating that those are used.
 *   When an invalid region is passed, an assertion is triggered. If assertions
 *   are optimized out, undefined behaviour occurs.
 *
 * @param x x- start positition of the region
 * @param y y- start position of the region
 * @param w width of the region
 * @param h height of the region
 */
void ShadowAtlas::reserve_region(size_t x, size_t y, size_t w, size_t h) {
  // Check if we are out of bounds, this should be disabled for performance
  // reasons at some point.
  nassertv(x >= 0 && y >= 0 && x + w <= _num_tiles && y + h <= _num_tiles);

  _num_used_tiles += w * h;

  // Iterate over every tile in the region and mark it as used
  for (size_t cx = 0; cx < w; ++cx) {
    for (size_t cy = 0; cy < h; ++cy) {
      set_tile(cx + x, cy + y, true);
    }
  }
}

/**
 * @brief Finds space for a map of the given size in the atlas.
 * @details This methods searches for a space to store a region of the given
 *   size in the atlas. tile_width and tile_height should be already in tile
 *   space. They can be converted using ShadowAtlas::get_required_tiles.
 *
 *   If no region is found, or an invalid size is passed, an integer vector with
 *   all components set to -1 is returned.
 *
 *  If a region is found, an integer vector with the given layout is returned:
 *   x: x- Start of the region
 *   y: y- Start of the region
 *   z: width of the region
 *   w: height of the region
 *
 *   The layout is in tile space, and can get converted to uv space using
 *   ShadowAtlas::region_to_uv.
 *
 * @param tile_width Width of the region in tile space
 * @param tile_height Height of the region in tile space
 *
 * @return Region, see description, or -1 when no region is found.
 */
LVecBase4i ShadowAtlas::find_and_reserve_region(size_t tile_width, size_t tile_height) {

  // Check for empty region
  if (tile_width < 1 || tile_height < 1) {
    shadowatlas_cat.error() << "Called find_and_reserve_region with null-region!" << std::endl;
    return LVecBase4i(-1);
  }

  // Check for region bigger than the shadow atlas
  if (tile_width > _num_tiles || tile_height > _num_tiles) {
    shadowatlas_cat.error() << "Requested region exceeds shadow atlas size!" << std::endl;
    return LVecBase4i(-1);
  }

  // Iterate over every possible region and check if its still free
  for (size_t x = 0; x <= _num_tiles - tile_width; ++x) {
    for (size_t y = 0; y <= _num_tiles - tile_height; ++y) {
      if (region_is_free(x, y, tile_width, tile_height)) {
        // Found free region, now reserve it
        reserve_region(x, y, tile_width, tile_height);
        return LVecBase4i(x, y, tile_width, tile_height);
      }
    }
  }

  // When we reached this part, we couldn't find a free region, so the atlas
  // seems to be full.
  shadowatlas_cat.error() << "Failed to find a free region of size " << tile_width
              << " x " << tile_height << "!"  << std::endl;
  return LVecBase4i(-1);
}

/**
 * @brief Frees a given region
 * @details This frees a given region, marking it as free so that other shadow
 *   maps can use the space again. The region should be the same as returned
 *   by ShadowAtlas::find_and_reserve_region.
 *
 *   If an invalid region is passed, an assertion is triggered. If assertions
 *   are compiled out, undefined behaviour will occur.
 *
 * @param region Region to free
 */
void ShadowAtlas::free_region(const LVecBase4i& region) {
  // Out of bounds check, can't hurt
  nassertv(region.get_x() >= 0 && region.get_y() >= 0);
  nassertv(region.get_x() + region.get_z() <= (int)_num_tiles && region.get_y() + region.get_w() <= (int)_num_tiles);

  _num_used_tiles -= region.get_z() * region.get_w();

  for (int x = 0; x < region.get_z(); ++x) {
    for (int y = 0; y < region.get_w(); ++y) {
      // Could do an assert here, that the tile should have been used (=true) before
      set_tile(region.get_x() + x, region.get_y() + y, false);
    }
  }
}
