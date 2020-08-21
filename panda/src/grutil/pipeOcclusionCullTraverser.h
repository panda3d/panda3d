/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pipeOcclusionCullTraverser.h
 * @author drose
 * @date 2007-05-29
 */

#ifndef PIPEOCCLUSIONCULLTRAVERSER_H
#define PIPEOCCLUSIONCULLTRAVERSER_H

#include "pandabase.h"
#include "cullTraverser.h"
#include "graphicsOutput.h"
#include "displayRegion.h"
#include "cullHandler.h"
#include "texture.h"

class GraphicsEngine;
class GraphicsPipe;
class GraphicsStateGuardian;

/**
 * This specialization of CullTraverser uses the graphics pipe itself to
 * perform occlusion culling.  As such, it's likely to be inefficient (since
 * it interferes with the pipe's normal mode of rendering), and is mainly
 * useful to test other, CPU-based occlusion algorithms.
 *
 * This cannot be used in a multithreaded pipeline environment where cull and
 * draw are operating simultaneously.
 *
 * It can't be defined in the cull subdirectory, because it needs access to
 * GraphicsPipe and DisplayRegion and other classes in display.  So we put it
 * in grutil instead, for lack of any better ideas.
 */
class EXPCL_PANDA_GRUTIL PipeOcclusionCullTraverser : public CullTraverser,
                                               public CullHandler {
PUBLISHED:
  explicit PipeOcclusionCullTraverser(GraphicsOutput *host);
  PipeOcclusionCullTraverser(const PipeOcclusionCullTraverser &copy) = delete;

  virtual void set_scene(SceneSetup *scene_setup,
                         GraphicsStateGuardianBase *gsg,
                         bool dr_incomplete_render);
  virtual void end_traverse();

  INLINE GraphicsOutput *get_buffer() const;
  Texture *get_texture();

  INLINE void set_occlusion_mask(const DrawMask &occlusion_mask);
  INLINE const DrawMask &get_occlusion_mask() const;

protected:
  virtual bool is_in_view(CullTraverserData &data);
  virtual void traverse_below(CullTraverserData &data);

  virtual void record_object(CullableObject *object,
                             const CullTraverser *traverser);

private:
  void make_sphere();
  static LVertex compute_sphere_point(PN_stdfloat latitude, PN_stdfloat longitude);
  void make_box();

  void make_solid_test_state();

  bool get_volume_viz(const BoundingVolume *vol,
                      CPT(Geom) &geom,  // OUT
                      CPT(TransformState) &net_transform, // IN-OUT
                      CPT(TransformState) &internal_transform  // OUT
                      );
  PT(OcclusionQueryContext)
    perform_occlusion_test(const Geom *geom,
                           const TransformState *net_transform,
                           const TransformState *internal_transform);

  void show_results(int num_fragments, const Geom *geom,
                    const TransformState *net_transform,
                    const TransformState *internal_transform);
private:
  bool _live;

  PT(GraphicsOutput) _buffer;
  PT(Texture) _texture;
  PT(DisplayRegion) _display_region;
  DrawMask _occlusion_mask;

  PT(SceneSetup) _scene;
  PT(CullTraverser) _internal_trav;
  CPT(TransformState) _inv_cs_world_transform;

  CullHandler *_internal_cull_handler;
  CullHandler *_true_cull_handler;

  // This is the query that has already been performed on the current node or
  // a parent.
  PT(OcclusionQueryContext) _current_query;

  // This is the query that has been performed for any children.
  PT(OcclusionQueryContext) _next_query;

  PT(Geom) _sphere_geom;
  PT(Geom) _box_geom;
  CPT(RenderState) _solid_test_state;

  class PendingObject {
  public:
    INLINE PendingObject(CullableObject *object);
    INLINE ~PendingObject();

    CullableObject *_object;
    PT(OcclusionQueryContext) _query;
  };
  typedef pvector<PendingObject> PendingObjects;
  PendingObjects _pending_objects;

  static PStatCollector _setup_occlusion_pcollector;
  static PStatCollector _draw_occlusion_pcollector;
  static PStatCollector _test_occlusion_pcollector;
  static PStatCollector _finish_occlusion_pcollector;

  static PStatCollector _occlusion_untested_pcollector;
  static PStatCollector _occlusion_passed_pcollector;
  static PStatCollector _occlusion_failed_pcollector;
  static PStatCollector _occlusion_tests_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    CullTraverser::init_type();
    register_type(_type_handle, "PipeOcclusionCullTraverser",
                  CullTraverser::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pipeOcclusionCullTraverser.I"

#endif
