/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cMotionTrail.h
 * @author aignacio
 * @date 2007-01-29
 */

#ifndef CMOTIONTRAIL_H
#define CMOTIONTRAIL_H

#include "directbase.h"

#include "geomNode.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomTriangles.h"
#include "luse.h"
#include "nurbsCurveEvaluator.h"
#include "plist.h"
#include "epvector.h"

class CMotionTrailVertex {
public:
  LPoint4 _vertex;
  LVecBase4 _start_color;
  LVecBase4 _end_color;
  PN_stdfloat _v;

  PT(NurbsCurveEvaluator) _nurbs_curve_evaluator;
};

class CMotionTrailFrame {
public:
  UnalignedLMatrix4 _transform;
  PN_stdfloat _time;
};

/**
 * The method used in creating the motion trail is based on taking samples of
 * time and transformations (the position and orientation matrix) in real-
 * time.  The method also requires a number of vertices (positions) that
 * determines "shape" of the motion trail (i.e.  the edge of a blade).  A
 * start color and end color is also required for each vertex.  The color is
 * interpolated as function of time.  The colors are typically used to fade
 * the motion trail so the end color is typically black.
 *
 * The vertices are submitted via the "add_vertex" function.  For each frame,
 * a sample is submited via the "update_motion_trail" function.  During the
 * "update_motion_trail" function, the motion trail geometry is created
 * dynamically from the sample history and the vertices.
 *
 * The user must specifiy a GeomNode via "set_geom_node".
 *
 * The duration of the sample history is specified by a time window.  A larger
 * time window creates longer motion trails (given constant speed).  Samples
 * that are no longer within the time window are automatically discarded.
 *
 * The nurbs option can be used to create smooth interpolated curves from the
 * samples.  The nurbs option is useful for animations that lack sampling to
 * begin with, animations that move very quickly, or low frame rates.
 *
 * The texture option be used to create variation to the motion trail.  The u
 * coordinate of the texture corresponds to time and the v coordinate
 * corresponds to the "shape" of the motion trail.
 */
class EXPCL_DIRECT_MOTIONTRAIL CMotionTrail : public TypedReferenceCount {
PUBLISHED:
  CMotionTrail();
  ~CMotionTrail();

  void reset();
  void reset_vertex_list();

  void enable(bool enable);

  void set_geom_node(GeomNode *geom_node);
  void add_vertex(LVector4 *vertex, LVector4 *start_color, LVector4 *end_color, PN_stdfloat v);

  void set_parameters(PN_stdfloat sampling_time, PN_stdfloat time_window, bool use_texture, bool calculate_relative_matrix, bool use_nurbs, PN_stdfloat resolution_distance);

  int check_for_update(PN_stdfloat current_time);
  void update_motion_trail(PN_stdfloat current_time, LMatrix4 *transform);

public:

  void begin_geometry();
  void add_geometry_quad(LVector3 &v0, LVector3 &v1, LVector3 &v2, LVector3 &v3, LVector4 &c0, LVector4 &c1, LVector4 &c2, LVector4 &c3, LVector2 &t0, LVector2 &t1, LVector2 &t2, LVector2 &t3);
  void add_geometry_quad(LVector4 &v0, LVector4 &v1, LVector4 &v2, LVector4 &v3, LVector4 &c0, LVector4 &c1, LVector4 &c2, LVector4 &c3, LVector2 &t0, LVector2 &t1, LVector2 &t2, LVector2 &t3);
  void end_geometry();

  int _active;
  int _enable;

  int _pause;
  PN_stdfloat _pause_time;

  int _fade;
  int _fade_end;
  PN_stdfloat _fade_time;
  PN_stdfloat _fade_start_time;
  PN_stdfloat _fade_color_scale;

  PN_stdfloat _last_update_time;

  typedef epvector<CMotionTrailVertex> VertexList;
  VertexList _vertex_list;
  typedef plist<CMotionTrailFrame> FrameList;
  FrameList _frame_list;

  // parameters
  PN_stdfloat _color_scale;
  PN_stdfloat _sampling_time;
  PN_stdfloat _time_window;
  bool _square_t;
  bool _use_texture;
  int _calculate_relative_matrix;

  // nurbs parameters
  bool _use_nurbs;
  PN_stdfloat _resolution_distance;

  // geom
  PT(GeomNode) _geom_node;

  // real-time data
  int _vertex_index;
  PT(GeomVertexData) _vertex_data;
  GeomVertexWriter _vertex_writer;
  GeomVertexWriter _color_writer;
  GeomVertexWriter _texture_writer;
  PT(GeomTriangles) _triangles;

  CMotionTrailVertex *_vertex_array;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "CMotionTrail",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#endif
