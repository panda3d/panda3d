/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file occluderNode.h
 * @author jenes
 * @date 2011-03-11
 */

#ifndef OCCLUDERNODE_H
#define OCCLUDERNODE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "nodePath.h"
#include "pvector.h"
#include "geom.h"
#include "texture.h"

/**
 * A node in the scene graph that can hold an occluder polygon, which must be
 * a rectangle.  When the occluder is activated with something like
 * render.set_occluder(), then objects whose bouding volume lies entirely
 * behind the occluder will not be rendered.
 */
class EXPCL_PANDA_PGRAPH OccluderNode : public PandaNode {
PUBLISHED:
  explicit OccluderNode(const std::string &name);

protected:
  OccluderNode(const OccluderNode &copy);

public:
  virtual ~OccluderNode();
  virtual PandaNode *make_copy() const;
  virtual bool preserve_name() const;
  virtual void xform(const LMatrix4 &mat);

  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool is_renderable() const;

  virtual void output(std::ostream &out) const;

PUBLISHED:
  INLINE void set_double_sided(bool value);
  INLINE bool is_double_sided();
  INLINE void set_min_coverage(PN_stdfloat value);
  INLINE PN_stdfloat get_min_coverage();
  INLINE void set_vertices(const LPoint3 &v0, const LPoint3 &v1,
                           const LPoint3 &v2, const LPoint3 &v3);
  INLINE size_t get_num_vertices() const;
  INLINE const LPoint3 &get_vertex(size_t n) const;
  INLINE void set_vertex(size_t n, const LPoint3 &v);
  MAKE_SEQ(get_vertices, get_num_vertices, get_vertex);

  MAKE_PROPERTY(double_sided, is_double_sided, set_double_sided);
  MAKE_PROPERTY(min_coverage, get_min_coverage, set_min_coverage);
  MAKE_SEQ_PROPERTY(vertices, get_num_vertices, get_vertex, set_vertex);

protected:
  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;
  PT(Geom) get_occluder_viz(CullTraverser *trav, CullTraverserData &data);
  CPT(RenderState) get_occluder_viz_state(CullTraverser *trav, CullTraverserData &data);
  CPT(RenderState) get_frame_viz_state(CullTraverser *trav, CullTraverserData &data);

private:
  bool _double_sided;
  PN_stdfloat _min_coverage;
  typedef pvector<LPoint3> Vertices;
  Vertices _vertices;

  PT(Geom) _occluder_viz, _frame_viz;
  static PT(Texture) _viz_tex;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "OccluderNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "occluderNode.I"

#endif
