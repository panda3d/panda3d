// Filename: ropeNode.h
// Created by:  drose (04Dec02)
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

#ifndef ROPENODE_H
#define ROPENODE_H

#include "pandabase.h"
#include "nurbsCurveEvaluator.h"
#include "pandaNode.h"

////////////////////////////////////////////////////////////////////
//       Class : RopeNode
// Description : This class draws a visible representation of the
//               NURBS curve stored in its NurbsCurveEvaluator.  It
//               automatically recomputes the curve every frame.
//
//               This is not related to NurbsCurve, ClassicNurbsCurve,
//               CubicCurveseg or any of the ParametricCurve-derived
//               objects in this module.  It is a completely parallel
//               implementation of NURBS curves, and will probably
//               eventually replace the whole ParametricCurve class
//               hierarchy.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RopeNode : public PandaNode {
PUBLISHED:
  RopeNode(const string &name);

protected:
  RopeNode(const RopeNode &copy);
public:
  virtual void output(ostream &out) const;
  virtual void write(ostream &out, int indent_level = 0) const;

  virtual PandaNode *make_copy() const;

  virtual bool safe_to_transform() const;
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

PUBLISHED:
  enum RenderMode {
    RM_thread,
    RM_billboard
  };
  enum UVMode {
    UV_none,
    UV_parametric,
    UV_distance,
    UV_distance2,
  };

  INLINE void set_curve(NurbsCurveEvaluator *curve);
  INLINE NurbsCurveEvaluator *get_curve() const;

  INLINE void set_render_mode(RenderMode render_mode);
  INLINE RenderMode get_render_mode() const;

  INLINE void set_uv_mode(UVMode uv_mode);
  INLINE UVMode get_uv_mode() const;

  INLINE void set_uv_scale(const LVecBase2f &uv_scale);
  INLINE const LVecBase2f &get_uv_scale() const;

  INLINE void set_num_segs(int num_segs);
  INLINE int get_num_segs() const;

  INLINE void set_thickness(float thickness);
  INLINE float get_thickness() const;

  BoundingVolume *reset_bound(const NodePath &rel_to);

protected:
  virtual BoundingVolume *recompute_internal_bound();

private:
  void render_thread(CullTraverser *trav, CullTraverserData &data, 
                     NurbsCurveResult *result);
  void render_billboard(CullTraverser *trav, CullTraverserData &data, 
                        NurbsCurveResult *result);

private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    PT(NurbsCurveEvaluator) _curve;
    RenderMode _render_mode;
    UVMode _uv_mode;
    LVecBase2f _uv_scale;
    int _num_segs;
    float _thickness;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

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
    PandaNode::init_type();
    register_type(_type_handle, "RopeNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "ropeNode.I"

#endif
