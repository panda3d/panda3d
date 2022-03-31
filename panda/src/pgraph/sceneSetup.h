/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file sceneSetup.h
 * @author drose
 * @date 2002-03-27
 */

#ifndef SCENESETUP_H
#define SCENESETUP_H

#include "pandabase.h"

#include "typedReferenceCount.h"
#include "nodePath.h"
#include "camera.h"
#include "transformState.h"
#include "lens.h"
#include "pointerTo.h"
#include "geometricBoundingVolume.h"

class DisplayRegion;

/**
 * This object holds the camera position, etc., and other general setup
 * information for rendering a particular scene.
 */
class EXPCL_PANDA_PGRAPH SceneSetup : public TypedReferenceCount {
public:
  INLINE SceneSetup();

PUBLISHED:
  INLINE void set_display_region(DisplayRegion *display_region);
  INLINE DisplayRegion *get_display_region() const;

  INLINE void set_viewport_size(int width, int height);
  INLINE int get_viewport_width() const;
  INLINE int get_viewport_height() const;

  INLINE void set_scene_root(const NodePath &scene_root);
  INLINE const NodePath &get_scene_root() const;

  INLINE void set_camera_path(const NodePath &camera_path);
  INLINE const NodePath &get_camera_path() const;

  INLINE void set_camera_node(Camera *camera_node);
  INLINE Camera *get_camera_node() const;

  INLINE void set_lens(const Lens *lens);
  INLINE const Lens *get_lens() const;

  INLINE void set_inverted(bool inverted);
  INLINE bool get_inverted() const;

  INLINE const NodePath &get_cull_center() const;
  INLINE PT(BoundingVolume) get_cull_bounds() const;

  INLINE void set_view_frustum(PT(GeometricBoundingVolume) view_frustum);
  INLINE GeometricBoundingVolume *get_view_frustum() const;

  INLINE void set_initial_state(const RenderState *initial_state);
  INLINE const RenderState *get_initial_state() const;

  INLINE void set_camera_transform(const TransformState *camera_transform);
  INLINE const TransformState *get_camera_transform() const;

  INLINE void set_world_transform(const TransformState *world_transform);
  INLINE const TransformState *get_world_transform() const;

  INLINE void set_cs_transform(const TransformState *cs_transform);
  INLINE const TransformState *get_cs_transform() const;

  INLINE void set_cs_world_transform(const TransformState *cs_world_transform);
  INLINE const TransformState *get_cs_world_transform() const;

private:
  DisplayRegion *_display_region;
  int _viewport_width;
  int _viewport_height;
  NodePath _scene_root;
  NodePath _camera_path;
  PT(Camera) _camera_node;
  CPT(Lens) _lens;
  bool _inverted;
  CPT(RenderState) _initial_state;
  CPT(TransformState) _camera_transform;
  CPT(TransformState) _world_transform;
  CPT(TransformState) _cs_transform;
  CPT(TransformState) _cs_world_transform;
  PT(GeometricBoundingVolume) _view_frustum;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "SceneSetup",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "sceneSetup.I"

#endif
