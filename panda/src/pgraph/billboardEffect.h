/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file billboardEffect.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef BILLBOARDEFFECT_H
#define BILLBOARDEFFECT_H

#include "pandabase.h"

#include "renderEffect.h"
#include "luse.h"
#include "nodePath.h"

/**
 * Indicates that geometry at this node should automatically rotate to face
 * the camera, or any other arbitrary node.
 */
class EXPCL_PANDA_PGRAPH BillboardEffect : public RenderEffect {
private:
  INLINE BillboardEffect();

PUBLISHED:
  static CPT(RenderEffect) make(const LVector3 &up_vector,
                                bool eye_relative,
                                bool axial_rotate,
                                PN_stdfloat offset,
                                const NodePath &look_at,
                                const LPoint3 &look_at_point,
                                bool fixed_depth = false);
  INLINE static CPT(RenderEffect) make_axis();
  INLINE static CPT(RenderEffect) make_point_eye();
  INLINE static CPT(RenderEffect) make_point_world();

  INLINE bool is_off() const;
  INLINE const LVector3 &get_up_vector() const;
  INLINE bool get_eye_relative() const;
  INLINE bool get_axial_rotate() const;
  INLINE bool get_fixed_depth() const;
  INLINE PN_stdfloat get_offset() const;
  INLINE const NodePath &get_look_at() const;
  INLINE const LPoint3 &get_look_at_point() const;

public:
  virtual bool safe_to_transform() const;
  virtual CPT(TransformState) prepare_flatten_transform(const TransformState *net_transform) const;
  virtual void output(std::ostream &out) const;

  virtual bool has_cull_callback() const;
  virtual void cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const;

  virtual bool has_adjust_transform() const;
  virtual void adjust_transform(CPT(TransformState) &net_transform,
                                CPT(TransformState) &node_transform,
                                const PandaNode *node) const;

protected:
  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  void compute_billboard(CPT(TransformState) &node_transform,
                         const TransformState *net_transform,
                         const TransformState *camera_transform) const;

private:
  bool _off;
  bool _eye_relative;
  bool _axial_rotate;
  bool _fixed_depth;
  LVector3 _up_vector;
  PN_stdfloat _offset;
  NodePath _look_at;
  LPoint3 _look_at_point;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderEffect::init_type();
    register_type(_type_handle, "BillboardEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "billboardEffect.I"

#endif
