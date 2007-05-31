// Filename: cullTraverser.h
// Created by:  drose (23Feb02)
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

#ifndef CULLTRAVERSER_H
#define CULLTRAVERSER_H

#include "pandabase.h"

#include "geom.h"
#include "sceneSetup.h"
#include "renderState.h"
#include "transformState.h"
#include "geometricBoundingVolume.h"
#include "pointerTo.h"
#include "camera.h"
#include "drawMask.h"
#include "typedReferenceCount.h"
#include "pStatCollector.h"

class GraphicsStateGuardian;
class PandaNode;
class CullHandler;
class CullableObject;
class CullTraverserData;
class PortalClipper;
class NodePath;

////////////////////////////////////////////////////////////////////
//       Class : CullTraverser
// Description : This object performs a depth-first traversal of the
//               scene graph, with optional view-frustum culling,
//               collecting CullState and searching for GeomNodes.
//               Each renderable Geom encountered is passed along with
//               its associated RenderState to the CullHandler object.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullTraverser : public TypedReferenceCount {
PUBLISHED:
  CullTraverser();
  CullTraverser(const CullTraverser &copy);

public:
  INLINE GraphicsStateGuardianBase *get_gsg() const;
  INLINE Thread *get_current_thread() const;

  virtual void set_scene(SceneSetup *scene_setup,
                         GraphicsStateGuardianBase *gsg);
  INLINE SceneSetup *get_scene() const;
  INLINE bool has_tag_state_key() const;
  INLINE const string &get_tag_state_key() const;

  INLINE void set_camera_mask(const DrawMask &camera_mask);
  INLINE const DrawMask &get_camera_mask() const;

  INLINE const TransformState *get_camera_transform() const;
  INLINE const TransformState *get_world_transform() const;

  INLINE const RenderState *get_initial_state() const;
  INLINE bool get_depth_offset_decals() const;

  INLINE void set_view_frustum(GeometricBoundingVolume *view_frustum);
  INLINE GeometricBoundingVolume *get_view_frustum() const;

  INLINE void set_cull_handler(CullHandler *cull_handler);
  INLINE CullHandler *get_cull_handler() const;

  INLINE void set_portal_clipper(PortalClipper *portal_clipper);
  INLINE PortalClipper *get_portal_clipper() const;

  void traverse(const NodePath &root);
  void traverse(CullTraverserData &data);
  virtual void traverse_below(CullTraverserData &data);

  virtual void end_traverse();

  INLINE static void flush_level();

  void draw_bounding_volume(const BoundingVolume *vol, 
                            const TransformState *net_transform,
                            const TransformState *modelview_transform);

protected:
  virtual bool is_in_view(CullTraverserData &data);

public:
  // Statistics
  static PStatCollector _nodes_pcollector;
  static PStatCollector _geom_nodes_pcollector;
  static PStatCollector _geoms_pcollector;
  static PStatCollector _geoms_occluded_pcollector;

private:
  void show_bounds(CullTraverserData &data, bool tight);
  PT(Geom) make_bounds_viz(const BoundingVolume *vol);
  PT(Geom) make_tight_bounds_viz(PandaNode *node);
  static Vertexf compute_point(const BoundingSphere *sphere, 
                               float latitude, float longitude);
  CPT(RenderState) get_bounds_outer_viz_state();
  CPT(RenderState) get_bounds_inner_viz_state();
  CPT(RenderState) get_depth_offset_state();
  void start_decal(const CullTraverserData &data);
  CullableObject *r_get_decals(CullTraverserData &data,
                               CullableObject *decals);

  GraphicsStateGuardianBase *_gsg;
  Thread *_current_thread;
  PT(SceneSetup) _scene_setup;
  DrawMask _camera_mask;
  bool _has_tag_state_key;
  string _tag_state_key;
  CPT(RenderState) _initial_state;
  bool _depth_offset_decals;
  PT(GeometricBoundingVolume) _view_frustum;
  CullHandler *_cull_handler;
  PortalClipper *_portal_clipper;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "CullTraverser",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "cullTraverser.I"

#endif


  
