/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file ropeNode.h
 * @author drose
 * @date 2002-12-04
 */

#ifndef ROPENODE_H
#define ROPENODE_H

#include "pandabase.h"
#include "nurbsCurveEvaluator.h"
#include "pandaNode.h"
#include "pStatCollector.h"
#include "geomVertexFormat.h"

class GeomVertexData;

/**
 * This class draws a visible representation of the NURBS curve stored in its
 * NurbsCurveEvaluator.  It automatically recomputes the curve every frame.
 *
 * This is not related to NurbsCurve, CubicCurveseg or any of the
 * ParametricCurve-derived objects in this module.  It is a completely
 * parallel implementation of NURBS curves, and will probably eventually
 * replace the whole ParametricCurve class hierarchy.
 */
class EXPCL_PANDA_PARAMETRICS RopeNode : public PandaNode {
PUBLISHED:
  explicit RopeNode(const std::string &name);

protected:
  RopeNode(const RopeNode &copy);
public:
  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level = 0) const;

  virtual PandaNode *make_copy() const;

  virtual bool safe_to_transform() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual bool is_renderable() const;

PUBLISHED:
  enum RenderMode {
    // Render the rope as a one-pixel thread using a linestrip.
    RM_thread,

    // Render the rope as a triangle strip oriented to be perpendicular to the
    // tube_up vector.
    RM_tape,

    // Render the rope as a triangle strip oriented to be perpendicular to the
    // view vector.
    RM_billboard,

    // Render the rope as a hollow tube extruded along its length.
    RM_tube
  };

  enum UVMode {
    // Don't generate UV's along the curve.
    UV_none,

    // Generate UV's based on the parametric coordinates along the curve.
    UV_parametric,

    // Generate UV's in proportion to spatial distance along the curve, by
    // using the distance function to compute the length of each segment.
    UV_distance,

    // As above, but don't bother to take the square root of each segment.
    // The distance is then in proportion to the sum-of-squares of the
    // segments along the rope.  If the segments are similar in length, this
    // approximates the proportion of UV_distance while avoiding hundreds of
    // square root operations.
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

  INLINE void set_uv_scale(PN_stdfloat scale);
  INLINE PN_stdfloat get_uv_scale() const;

  INLINE void set_normal_mode(NormalMode normal_mode);
  INLINE NormalMode get_normal_mode() const;

  INLINE void set_tube_up(const LVector3 &tube_up);
  INLINE const LVector3 &get_tube_up() const;

  INLINE void set_use_vertex_color(bool flag);
  INLINE bool get_use_vertex_color() const;
  INLINE static int get_vertex_color_dimension();

  INLINE void set_num_subdiv(int num_subdiv);
  INLINE int get_num_subdiv() const;

  INLINE void set_num_slices(int num_slices);
  INLINE int get_num_slices() const;

  INLINE void set_use_vertex_thickness(bool flag);
  INLINE bool get_use_vertex_thickness() const;
  INLINE static int get_vertex_thickness_dimension();

  INLINE void set_thickness(PN_stdfloat thickness);
  INLINE PN_stdfloat get_thickness() const;

  INLINE void set_matrix(const LMatrix4 &matrix);
  INLINE void clear_matrix();
  INLINE bool has_matrix() const;
  INLINE const LMatrix4 &get_matrix() const;

  void reset_bound(const NodePath &rel_to);

PUBLISHED:
  MAKE_PROPERTY(curve, get_curve, set_curve);
  MAKE_PROPERTY(render_mode, get_render_mode, set_render_mode);
  MAKE_PROPERTY(uv_mode, get_uv_mode, set_uv_mode);
  MAKE_PROPERTY(uv_direction, get_uv_direction, set_uv_direction);
  MAKE_PROPERTY(uv_scale, get_uv_scale, set_uv_scale);
  MAKE_PROPERTY(normal_mode, get_normal_mode, set_normal_mode);
  MAKE_PROPERTY(tube_up, get_tube_up, set_tube_up);
  MAKE_PROPERTY(use_vertex_color, get_use_vertex_color, set_use_vertex_color);
  MAKE_PROPERTY(vertex_color_dimension, get_vertex_color_dimension);
  MAKE_PROPERTY(num_subdiv, get_num_subdiv, set_num_subdiv);
  MAKE_PROPERTY(num_slices, get_num_slices, set_num_slices);
  MAKE_PROPERTY(use_vertex_thickness, get_use_vertex_thickness, set_use_vertex_thickness);
  MAKE_PROPERTY(vertex_thickness_dimension, get_vertex_thickness_dimension);
  MAKE_PROPERTY(thickness, get_thickness, set_thickness);
  MAKE_PROPERTY2(matrix, has_matrix, get_matrix, set_matrix, clear_matrix);

protected:
  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

private:
  CPT(GeomVertexFormat) get_format(bool support_normals) const;

  PT(BoundingVolume) do_recompute_bounds(const NodePath &rel_to,
                                         int pipeline_stage,
                                         Thread *current_thread) const;
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
    LPoint3 _p;
    UnalignedLVecBase4 _c;
    PN_stdfloat _thickness;
    PN_stdfloat _t;
  };
  typedef pvector<CurveVertex> CurveSegment;
  typedef pvector<CurveSegment> CurveSegments;

  int get_connected_segments(CurveSegments &curve_segments,
                             const NurbsCurveResult *result) const;

  void compute_thread_vertices(GeomVertexData *vdata,
                               const CurveSegments &curve_segments,
                               int num_curve_verts) const;
  void compute_billboard_vertices(GeomVertexData *vdata,
                                  const LVector3 &camera_vec,
                                  const CurveSegments &curve_segments,
                                  int num_curve_verts,
                                  NurbsCurveResult *result) const;
  void compute_tube_vertices(GeomVertexData *vdata,
                             int &num_verts_per_slice,
                             const CurveSegments &curve_segments,
                             int num_curve_verts,
                             NurbsCurveResult *result) const;

  static void compute_tangent(LVector3 &tangent, const CurveSegment &segment,
                              size_t j, NurbsCurveResult *result);
  static PN_stdfloat compute_uv_t(PN_stdfloat &dist, const UVMode &uv_mode,
                            PN_stdfloat uv_scale, const CurveSegment &segment,
                            size_t j);


private:
  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_PARAMETRICS CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return RopeNode::get_class_type();
    }

    PT(NurbsCurveEvaluator) _curve;
    RenderMode _render_mode;
    UVMode _uv_mode;
    bool _u_dominant;
    PN_stdfloat _uv_scale;
    NormalMode _normal_mode;
    LVector3 _tube_up;
    LMatrix4 _matrix;
    bool _has_matrix;
    bool _use_vertex_color;
    int _num_subdiv;
    int _num_slices;
    bool _use_vertex_thickness;
    PN_stdfloat _thickness;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

  static PStatCollector _rope_node_pcollector;

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
