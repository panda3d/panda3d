// Filename: ropeNode.h
// Created by:  drose (04Dec02)
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
    // Render the rope as a one-pixel thread using a linestrip.
    RM_thread,

    // Render the rope as a triangle strip oriented to be
    // perpendicular to the tube_up vector.
    RM_tape,

    // Render the rope as a triangle strip oriented to be
    // perpendicular to the view vector.
    RM_billboard,

    // Render the rope as a hollow tube extruded along its length.
    RM_tube
  };

  enum UVMode {
    // Don't generate UV's along the curve.
    UV_none,

    // Generate UV's based on the parametric coordinates along the
    // curve.
    UV_parametric,

    // Generate UV's in proportion to spatial distance along the
    // curve, by using the distance function to compute the length of
    // each segment.
    UV_distance,

    // As above, but don't bother to take the square root of each
    // segment.  The distance is then in proportion to the
    // sum-of-squares of the segments along the rope.  If the segments
    // are similar in length, this approximates the proportion of
    // UV_distance while avoiding hundreds of square root operations.
    UV_distance2,
  };

  enum NormalMode {
    // Don't generate normals.
    NM_none,

    // Generate vertex (smooth-shaded) normals.
    NM_vertex
  };

  INLINE void set_curve(NurbsCurveEvaluator *curve);
  INLINE NurbsCurveEvaluator *get_curve() const;

  INLINE void set_render_mode(RenderMode render_mode);
  INLINE RenderMode get_render_mode() const;

  INLINE void set_uv_mode(UVMode uv_mode);
  INLINE UVMode get_uv_mode() const;

  INLINE void set_uv_direction(bool u_dominant);
  INLINE bool get_uv_direction() const;

  INLINE void set_uv_scale(float scale);
  INLINE float get_uv_scale() const;

  INLINE void set_normal_mode(NormalMode normal_mode);
  INLINE NormalMode get_normal_mode() const;

  INLINE void set_tube_up(const LVector3f &tube_up);
  INLINE const LVector3f &get_tube_up() const;

  INLINE void set_use_vertex_color(bool flag);
  INLINE bool get_use_vertex_color() const;

  INLINE void set_num_subdiv(int num_subdiv);
  INLINE int get_num_subdiv() const;

  INLINE void set_num_slices(int num_slices);
  INLINE int get_num_slices() const;

  INLINE void set_thickness(float thickness);
  INLINE float get_thickness() const;

  INLINE void set_matrix(const LMatrix4f &matrix);
  INLINE const LMatrix4f &get_matrix() const;

  void reset_bound(const NodePath &rel_to);

protected:
  virtual BoundingVolume *recompute_internal_bound();

private:
  BoundingVolume *do_recompute_bound(const NodePath &rel_to);
  void render_thread(CullTraverser *trav, CullTraverserData &data, 
                     NurbsCurveResult *result) const;
  void render_tape(CullTraverser *trav, CullTraverserData &data, 
                   NurbsCurveResult *result) const;
  void render_billboard(CullTraverser *trav, CullTraverserData &data, 
                        NurbsCurveResult *result) const;
  void render_tube(CullTraverser *trav, CullTraverserData &data, 
                   NurbsCurveResult *result) const;

  class CurveVertex {
  public:
    LPoint3f _p;
    Colorf _c;
    float _t;
  };
  typedef pvector<CurveVertex> CurveSegment;
  typedef pvector<CurveSegment> CurveSegments;

  void get_connected_segments(CurveSegments &curve_segments,
                              const NurbsCurveResult *result) const;

  void compute_thread_vertices(PTA_Vertexf &verts, PTA_TexCoordf &uvs, 
                               PTA_Colorf &colors,
                               const CurveSegments &curve_segments) const;
  void compute_billboard_vertices(PTA_Vertexf &verts, PTA_TexCoordf &uvs, 
                                  PTA_Colorf &colors,
                                  const LVector3f &camera_vec,
                                  const CurveSegments &curve_segments,
                                  NurbsCurveResult *result) const;
  void compute_tube_vertices(PTA_Vertexf &verts, PTA_Normalf &normals,
                             PTA_TexCoordf &uvs, PTA_Colorf &colors,
                             int &num_verts_per_slice,
                             const CurveSegments &curve_segments,
                             NurbsCurveResult *result) const;

  static void compute_tangent(LVector3f &tangent, const CurveSegment &segment,
                              size_t j, NurbsCurveResult *result);
  static float compute_uv_t(float &dist, const UVMode &uv_mode,
                            float uv_scale, const CurveSegment &segment,
                            size_t j);


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
    bool _u_dominant;
    float _uv_scale;
    NormalMode _normal_mode;
    LVector3f _tube_up;
    LMatrix4f _matrix;
    bool _use_vertex_color;
    int _num_subdiv;
    int _num_slices;
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
