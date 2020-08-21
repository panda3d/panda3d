/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file scissorEffect.h
 * @author drose
 * @date 2008-07-30
 */

#ifndef SCISSOREFFECT_H
#define SCISSOREFFECT_H

#include "pandabase.h"

#include "renderEffect.h"
#include "luse.h"
#include "nodePath.h"

// Forward declarations
class Lens;

/**
 * This provides a higher-level wrapper around ScissorAttrib.  It allows for
 * the scissor region to be defined via points relative to the current node,
 * and also performs culling based on the scissor region.
 */
class EXPCL_PANDA_PGRAPH ScissorEffect : public RenderEffect {
private:
  class PointDef {
  public:
    LPoint3 _p;
    NodePath _node;
  };

  ScissorEffect(bool screen, const LVecBase4 &frame,
                const PointDef *points, int num_points, bool clip);
  ScissorEffect(const ScissorEffect &copy);

PUBLISHED:
  static CPT(RenderEffect) make_screen(const LVecBase4 &frame, bool clip = true);
  static CPT(RenderEffect) make_node(bool clip = true);
  static CPT(RenderEffect) make_node(const LPoint3 &a, const LPoint3 &b, const NodePath &node = NodePath());
  static CPT(RenderEffect) make_node(const LPoint3 &a, const LPoint3 &b, const LPoint3 &c, const LPoint3 &d, const NodePath &node = NodePath());
  CPT(RenderEffect) add_point(const LPoint3 &point, const NodePath &node = NodePath()) const;

  INLINE bool is_screen() const;
  INLINE const LVecBase4 &get_frame() const;

  INLINE int get_num_points() const;
  INLINE const LPoint3 &get_point(int n) const;
  MAKE_SEQ(get_points, get_num_points, get_point);
  INLINE NodePath get_node(int n) const;
  MAKE_SEQ(get_nodes, get_num_points, get_node);

  INLINE bool get_clip() const;

public:
  virtual CPT(RenderEffect) xform(const LMatrix4 &mat) const;
  virtual void output(std::ostream &out) const;

  virtual bool has_cull_callback() const;
  virtual void cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const;

protected:
  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  PT(GeometricBoundingVolume) make_frustum(const Lens *lens, const LVecBase4 &frame) const;

private:
  bool _screen;
  LVecBase4 _frame;
  typedef pvector<PointDef> Points;
  Points _points;
  bool _clip;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderEffect::init_type();
    register_type(_type_handle, "ScissorEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "scissorEffect.I"

#endif
