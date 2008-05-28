// Filename: cMotionTrail.h
// Created by:  aignacio (29Jan07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef CMOTIONTRAIL_H
#define CMOTIONTRAIL_H

#include "directbase.h"

#include "geomNode.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomTriangles.h"
#include "luse.h"
#include "nurbsCurveEvaluator.h"

#include <list>

class CMotionTrailVertex {

public:
  LVector4f _vertex;
  LVector4f _start_color;
  LVector4f _end_color;
  float _v;
  
  PT(NurbsCurveEvaluator) _nurbs_curve_evaluator;
};

class CMotionTrailFrame {

public:
  float _time;
  LMatrix4f _transform;
};

////////////////////////////////////////////////////////////////////
//       Class : CMotionTrail
// Description : The method used in creating the motion trail is 
//               based on taking samples of time and transformations 
//               (the position and orientation matrix) in real-time.
//               The method also requires a number of vertices 
//               (positions) that determines "shape" of the motion 
//               trail (i.e. the edge of a blade).  A start color 
//               and end color is also required for each vertex. 
//               The color is interpolated as function of time.
//               The colors are typically used to fade the motion
//               trail so the end color is typically black.
//
//               The vertices are submitted via the "add_vertex" 
//               function.  For each frame, a sample is submited via 
//               the "update_motion_trail" function.  During the 
//               "update_motion_trail" function, the motion trail 
//               geometry is created dynamically from the sample 
//               history and the vertices.  
//
//               The user must specifiy a GeomNode via 
//               "set_geom_node".
//
//               The duration of the sample history is specified by 
//               a time window. A larger time window creates longer 
//               motion trails (given constant speed).  Samples that 
//               are no longer within the time window are 
//               automatically discarded.
//
//               The nurbs option can be used to create smooth 
//               interpolated curves from the samples.  The nurbs 
//               option is useful for animations that lack sampling 
//               to begin with, animations that move very quickly, 
//               or low frame rates.
//
//               The texture option be used to create variation to 
//               the motion trail.  The u coordinate of the texture 
//               corresponds to time and the v coordinate 
//               corresponds to the "shape" of the motion trail.
////////////////////////////////////////////////////////////////////

class EXPCL_DIRECT CMotionTrail : public TypedReferenceCount {

PUBLISHED:

  CMotionTrail ( );
  ~CMotionTrail ( );

  void reset ( );  
  void reset_vertex_list ( );

  void enable (bool enable);

  void set_geom_node (PT(GeomNode) geom_node);
  void add_vertex (LVector4f *vertex, LVector4f *start_color, LVector4f *end_color, float v);

  void set_parameters (float sampling_time, float time_window, bool use_texture, bool calculate_relative_matrix, bool use_nurbs, float resolution_distance);

  int check_for_update (float current_time);
  void update_motion_trail (float current_time, LMatrix4f *transform);

public:

  void begin_geometry ( );
  void add_geometry_quad (LVector3f &v0, LVector3f &v1, LVector3f &v2, LVector3f &v3, LVector4f &c0, LVector4f &c1, LVector4f &c2, LVector4f &c3, LVector2f &t0, LVector2f &t1, LVector2f &t2, LVector2f &t3);
  void add_geometry_quad (LVector4f &v0, LVector4f &v1, LVector4f &v2, LVector4f &v3, LVector4f &c0, LVector4f &c1, LVector4f &c2, LVector4f &c3, LVector2f &t0, LVector2f &t1, LVector2f &t2, LVector2f &t3);
  void end_geometry ( );

  int _active;
  int _enable;

  int _pause;
  float _pause_time;

  int _fade;
  int _fade_end;
  float _fade_time;
  float _fade_start_time;
  float _fade_color_scale;

  float _last_update_time;

  list <CMotionTrailVertex> _vertex_list;
  list <CMotionTrailFrame> _frame_list;

  // parameters
  float _color_scale;
  float _sampling_time;
  float _time_window;
  bool _square_t;
  bool _use_texture;
  int _calculate_relative_matrix;

  // nurbs parameters
  bool _use_nurbs;
  float _resolution_distance;

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
