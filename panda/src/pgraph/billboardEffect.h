// Filename: billboardEffect.h
// Created by:  drose (14Mar02)
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

#ifndef BILLBOARDEFFECT_H
#define BILLBOARDEFFECT_H

#include "pandabase.h"

#include "renderEffect.h"
#include "luse.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : BillboardEffect
// Description : Indicates that geometry at this node should
//               automatically rotate to face the camera, or any other
//               arbitrary node.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA BillboardEffect : public RenderEffect {
private:
  INLINE BillboardEffect();

PUBLISHED:
  static CPT(RenderEffect) make(const LVector3f &up_vector,
                                bool eye_relative,
                                bool axial_rotate,
                                float offset,
                                const NodePath &look_at,
                                const LPoint3f &look_at_point);
  INLINE static CPT(RenderEffect) make_axis();
  INLINE static CPT(RenderEffect) make_point_eye();
  INLINE static CPT(RenderEffect) make_point_world();

  INLINE bool is_off() const;
  INLINE const LVector3f &get_up_vector() const;
  INLINE bool get_eye_relative() const;
  INLINE bool get_axial_rotate() const;
  INLINE float get_offset() const;
  INLINE const NodePath &get_look_at() const;
  INLINE const LPoint3f &get_look_at_point() const;

public:
  virtual bool safe_to_transform() const;
  virtual void output(ostream &out) const;

  virtual bool has_cull_callback() const;
  virtual void cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const;

  virtual bool has_net_transform() const;
  virtual CPT(TransformState) net_transform(const TransformState *orig_net_transform) const;

protected:
  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  CPT(TransformState) compute_billboard(const TransformState *net_transform, 
                                        const TransformState *camera_transform) const;

private:
  bool _off;
  LVector3f _up_vector;
  bool _eye_relative;
  bool _axial_rotate;
  float _offset;
  NodePath _look_at;
  LPoint3f _look_at_point;

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

