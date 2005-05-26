// Filename: sceneSetup.h
// Created by:  drose (27Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef SCENESETUP_H
#define SCENESETUP_H

#include "pandabase.h"

#include "referenceCount.h"
#include "nodePath.h"
#include "camera.h"
#include "transformState.h"
#include "lens.h"
#include "pointerTo.h"

////////////////////////////////////////////////////////////////////
//       Class : SceneSetup
// Description : This object holds the camera position, etc., and
//               other general setup information for rendering a
//               particular scene.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA SceneSetup : public ReferenceCount {
public:
  INLINE SceneSetup();

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

  INLINE void set_initial_state(const RenderState *initial_state);
  INLINE const RenderState *get_initial_state() const;

  INLINE void set_camera_transform(const TransformState *camera_transform);
  INLINE const TransformState *get_camera_transform() const;

  INLINE void set_world_transform(const TransformState *world_transform);
  INLINE const TransformState *get_world_transform() const;

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
};

#include "sceneSetup.I"

#endif
