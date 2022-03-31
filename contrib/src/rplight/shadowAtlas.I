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


/**
 * @brief Returns the tile size of the atlas.
 * @details This returns the tile size of the atlas, which was set in the
 *   constructor. This is the smalles unit of the atlas, and every resolution
 *   has to be a multiple of the tile size.
 * @return Tile size in pixels
 */
inline int ShadowAtlas::get_tile_size() const {
  return _tile_size;
}

/**
 * @brief Sets a specific tile status.
 * @details This marks a tile to either reserved or free, depending on the flag.
 *   If flag is true, the tile gets marked as reserved. If flag is false, the
 *   tile gets marked as free.
 *
 *   No bounds checking is done for performance reasons. Passing an invalid tile
 *   index causes a crash. The coordinates are expected to be in tile space.
 *
 * @param x x-position of the tile
 * @param y y-position of the tile
 * @param flag Flag to set the tile to
 */
inline void ShadowAtlas::set_tile(size_t x, size_t y, bool flag) {
  _flags[x + y * _num_tiles] = flag;
}

/**
 * @brief Returns the status of a given tile.
 * @details This returns the value of a tile. If the tile is true, this means
 *   the tile is already reserved. If the tile is false, the tile can be
 *   used, and is not reserved.
 *
 *   No bounds checking is done for performance reasons. Passing an invalid tile
 *   index causes a crash. The coordinates are expected to be in tile space.
 *
 * @param x x-position of the tile
 * @param y y-position of the tile
 *
 * @return Tile-Status
 */
inline bool ShadowAtlas::get_tile(size_t x, size_t y) const {
  return _flags[x + y * _num_tiles];
}

/**
 * @brief Checks wheter a given region is free.
 * @details This checks whether a given region in the atlas is still free. This
 *   is true if *all* tiles in that region are false, and thus are not taken yet.
 *   The coordinates are expected to be in tile space.
 *
 *   Passing an invalid region, causes an assertion, in case those are enabled.
 *   If assertions are optimized out, this method will crash when passing invalid
 *   bounds.
 *
 * @param x x- start position of the region
 * @param y y- start position of the region
 * @param w width of the region
 * @param h height of the region
 * @return true if the region is completely free, else false
 */
inline bool ShadowAtlas::region_is_free(size_t x, size_t y, size_t w, size_t h) const {
  // Check if we are out of bounds, this should be disabled for performance
  // reasons at some point.
  nassertr(x >= 0 && y >= 0 && x + w <= _num_tiles && y + h <= _num_tiles, false);

  // Iterate over every tile in that region and check if it is still free.
  for (size_t cx = 0; cx < w; ++cx) {
    for (size_t cy = 0; cy < h; ++cy) {
      if (get_tile(cx + x, cy + y)) return false;
    }
  }
  return true;
}

/**
 * @brief Returns the amount of tiles required to store a resolution.
 * @details Returns the amount of tiles which would be required to store a
 *   given resolution. This basically just returns resolution / tile_size.
 *
 *   When an invalid resolution is passed (not a multiple of the tile size),
 *   an error is printed and 1 is returned.
 *   When a negative or zero resolution is passed, undefined behaviour occurs.
 *
 * @param resolution The resolution to compute the amount of tiles for
 * @return Amount of tiles to store the resolution
 */
inline int ShadowAtlas::get_required_tiles(size_t resolution) const {
  nassertr(resolution > 0, -1);

  if (resolution % _tile_size != 0) {
    shadowatlas_cat.error() << "Resolution " << resolution << " is not a multiple "
                << "of the shadow atlas tile size (" << _tile_size << ")!" << std::endl;
    return 1;
  }
  return resolution / _tile_size;
}

/**
 * @brief Converts a tile-space region to uv space.
 * @details This converts a region (presumably from ShadowAtlas::find_and_reserve_region)
 *   to uv space (0 .. 1 range). This can be used in shaders, since they expect
 *   floating point coordinates instead of integer coordinates.
 *
 * @param region tile-space region
 * @return uv-space region
 */
inline LVecBase4 ShadowAtlas::region_to_uv(const LVecBase4i& region) {
  LVecBase4 flt = LVecBase4(region.get_x(), region.get_y(), region.get_z(), region.get_w());
  return flt * ((float)_tile_size / (float)_size);
}

/**
 * @brief Returns the amount of used tiles
 * @details Returns the amount of used tiles in the atlas
 * @return Amount of used tiles
 */
inline int ShadowAtlas::get_num_used_tiles() const {
  return _num_used_tiles;
}

/**
 * @brief Returns the amount of used tiles in percentage
 * @details This returns in percentage from 0 to 1 how much space of the atlas
 *   is used right now. A value of 1 means the atlas is completely full, whereas
 *   a value of 0 means the atlas is completely free.
 * @return Atlas usage in percentage
 */
inline float ShadowAtlas::get_coverage() const {
  return float(_num_used_tiles) / float(_num_tiles * _num_tiles);
}
