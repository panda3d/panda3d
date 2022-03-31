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

#ifndef SHADOWATLAS_H
#define SHADOWATLAS_H

#include "pandabase.h"
#include "luse.h"

NotifyCategoryDecl(shadowatlas, EXPORT_CLASS, EXPORT_TEMPL);


/**
 * @brief Class which manages distributing shadow maps in an atlas.
 * @details This class manages the shadow atlas. It handles finding and reserving
 *   space for new shadow maps.
 */
class ShadowAtlas {
PUBLISHED:
  ShadowAtlas(size_t size, size_t tile_size = 32);
  ~ShadowAtlas();

  inline int get_num_used_tiles() const;
  inline float get_coverage() const;

  MAKE_PROPERTY(num_used_tiles, get_num_used_tiles);
  MAKE_PROPERTY(coverage, get_coverage);

public:

  LVecBase4i find_and_reserve_region(size_t tile_width, size_t tile_height);
  void free_region(const LVecBase4i& region);
  inline LVecBase4 region_to_uv(const LVecBase4i& region);

  inline int get_tile_size() const;
  inline int get_required_tiles(size_t resolution) const;

protected:

  void init_tiles();

  inline void set_tile(size_t x, size_t y, bool flag);
  inline bool get_tile(size_t x, size_t y) const;

  inline bool region_is_free(size_t x, size_t y, size_t w, size_t h) const;
  void reserve_region(size_t x, size_t y, size_t w, size_t h);

  size_t _size;
  size_t _num_tiles;
  size_t _tile_size;
  size_t _num_used_tiles;
  bool* _flags;
};

#include "shadowAtlas.I"

#endif // SHADOWATLAS_H
