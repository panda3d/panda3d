// Filename: sceneSetup.h
// Created by:  drose (27Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef SCENESETUP_H
#define SCENESETUP_H

#include "pandabase.h"

#include "referenceCount.h"
#include "qpnodePath.h"
#include "qpcamera.h"
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

  INLINE void set_scene_root(const qpNodePath &scene_root);
  INLINE const qpNodePath &get_scene_root() const;

  INLINE void set_camera(const qpCamera *camera);
  INLINE const qpCamera *get_camera() const;

  INLINE void set_lens(const Lens *lens);
  INLINE const Lens *get_lens() const;

  INLINE void set_camera_transform(const TransformState *camera_transform);
  INLINE const TransformState *get_camera_transform() const;

  INLINE void set_render_transform(const TransformState *render_transform);
  INLINE const TransformState *get_render_transform() const;

private:
  qpNodePath _scene_root;
  CPT(qpCamera) _camera;
  CPT(Lens) _lens;
  CPT(TransformState) _camera_transform;
  CPT(TransformState) _render_transform;
};

#include "sceneSetup.I"

#endif
