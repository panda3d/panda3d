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

#ifndef SHADOWMANAGER_H
#define SHADOWMANAGER_H

#include "pandabase.h"
#include "camera.h"
#include "luse.h"
#include "matrixLens.h"
#include "referenceCount.h"
#include "nodePath.h"
#include "displayRegion.h"
#include "graphicsOutput.h"

#include "tagStateManager.h"
#include "shadowSource.h"
#include "shadowAtlas.h"

NotifyCategoryDecl(shadowmanager, EXPORT_CLASS, EXPORT_TEMPL);


class ShadowManager : public ReferenceCount {
PUBLISHED:
  ShadowManager();
  ~ShadowManager();

  inline void set_max_updates(size_t max_updates);
  inline void set_scene(NodePath scene_parent);
  inline void set_tag_state_manager(TagStateManager* tag_mgr);
  inline void set_atlas_graphics_output(GraphicsOutput* graphics_output);

  inline void set_atlas_size(size_t atlas_size);
  inline size_t get_atlas_size() const;
  MAKE_PROPERTY(atlas_size, get_atlas_size, set_atlas_size);

  inline size_t get_num_update_slots_left() const;
  MAKE_PROPERTY(num_update_slots_left, get_num_update_slots_left);

  inline ShadowAtlas* get_atlas() const;
  MAKE_PROPERTY(atlas, get_atlas);

  void init();
  void update();

public:
  inline bool add_update(const ShadowSource* source);

private:
  size_t _max_updates;
  size_t _atlas_size;
  NodePath _scene_parent;

  pvector<PT(Camera)> _cameras;
  pvector<NodePath> _camera_nps;
  pvector<PT(DisplayRegion)> _display_regions;

  ShadowAtlas* _atlas;
  TagStateManager* _tag_state_mgr;
  GraphicsOutput* _atlas_graphics_output;

  typedef pvector<const ShadowSource*> UpdateQueue;
  UpdateQueue _queued_updates;
};

#include "shadowManager.I"

#endif // SHADOWMANAGER_H
