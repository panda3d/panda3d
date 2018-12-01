/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cullTraverser.h
 * @author drose
 * @date 2002-02-23
 */

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
#include "fogAttrib.h"

class GraphicsStateGuardian;
class PandaNode;
class CullHandler;
class CullableObject;
class CullTraverserData;
class PortalClipper;
class NodePath;

/**
 * This object performs a depth-first traversal of the scene graph, with
 * optional view-frustum culling, collecting CullState and searching for
 * GeomNodes.  Each renderable Geom encountered is passed along with its
 * associated RenderState to the CullHandler object.
 */
class EXPCL_PANDA_PGRAPH CullTraverser : public TypedReferenceCount {
PUBLISHED:
  CullTraverser();
  CullTraverser(const CullTraverser &copy);

  INLINE GraphicsStateGuardianBase *get_gsg() const;
  INLINE Thread *get_current_thread() const;

  virtual void set_scene(SceneSetup *scene_setup,
                         GraphicsStateGuardianBase *gsg,
                         bool dr_incomplete_render);
  INLINE SceneSetup *get_scene() const;
  INLINE bool has_tag_state_key() const;
  INLINE const std::string &get_tag_state_key() const;

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

  INLINE bool get_effective_incomplete_render() const;

  void traverse(const NodePath &root);
  void traverse(CullTraverserData &data);
  virtual void traverse_below(CullTraverserData &data);

  virtual void end_traverse();

  INLINE static void flush_level();

  void draw_bounding_volume(const BoundingVolume *vol,
                            const TransformState *internal_transform) const;

protected:
  INLINE void do_traverse(CullTraverserData &data);

  virtual bool is_in_view(CullTraverserData &data);

public:
  // Statistics
  static PStatCollector _nodes_pcollector;
  static PStatCollector _geom_nodes_pcollector;
  static PStatCollector _geoms_pcollector;
  static PStatCollector _geoms_occluded_pcollector;

private:
  void show_bounds(CullTraverserData &data, bool tight);
  static PT(Geom) make_bounds_viz(const BoundingVolume *vol);
  PT(Geom) make_tight_bounds_viz(PandaNode *node) const;
  static LVertex compute_point(const BoundingSphere *sphere,
                               PN_stdfloat latitude, PN_stdfloat longitude);
  static CPT(RenderState) get_bounds_outer_viz_state();
  static CPT(RenderState) get_bounds_inner_viz_state();
  static CPT(RenderState) get_depth_offset_state();

  GraphicsStateGuardianBase *_gsg;
  Thread *_current_thread;
  PT(SceneSetup) _scene_setup;
  DrawMask _camera_mask;
  bool _has_tag_state_key;
  std::string _tag_state_key;
  CPT(RenderState) _initial_state;
  PT(GeometricBoundingVolume) _view_frustum;
  CullHandler *_cull_handler;
  PortalClipper *_portal_clipper;
  bool _effective_incomplete_render;

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

#include "cullTraverserData.h"

#include "cullTraverser.I"

#endif
