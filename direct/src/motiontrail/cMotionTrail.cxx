/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cMotionTrail.cxx
 * @author aignacio
 * @date 2007-01-29
 */

#include "directbase.h"
#include "cMotionTrail.h"
#include "renderState.h"
#include "colorAttrib.h"


TypeHandle CMotionTrail::_type_handle;

/**
 * Constructor
 */
CMotionTrail::
CMotionTrail ( ) {

  _active = true;
  _enable = true;

  _pause = false;
  _pause_time = 0.0f;

  _fade = false;
  _fade_end = false;
  _fade_time = 0.0f;
  _fade_start_time = 0.0f;
  _fade_color_scale = 1.0f;

  _last_update_time = 0.0f;

  _vertex_list.clear ( );
  _frame_list.clear ( );

  // parameters
  _color_scale = 1.0;
  _sampling_time = 0.0;
  _time_window = 1.0;
  _square_t = true;
  _use_texture = false;
  _calculate_relative_matrix = false;

  // nurbs parameters
  _use_nurbs = false;
  _resolution_distance = 0.5f;

  // node path states
  _geom_node = nullptr;

  // real-time data
  _vertex_index = 0;
  _vertex_data = nullptr;
  _triangles = nullptr;

  _vertex_array = nullptr;
}

/**
 * Destructor
 */
CMotionTrail::
~CMotionTrail ( ) {

}

/**
 * Reset the frame sample history.
 */
void CMotionTrail::
reset ( ) {
  _frame_list.clear ( );
}

/**
 * Reset the vertex list.
 */
void CMotionTrail::
reset_vertex_list ( ) {
  _vertex_list.clear ( );
}

/**
 * Enable/disable the motion trail.
 */
void CMotionTrail::
enable (bool enable) {
  _enable = enable;
}

/**
 * Set the GeomNode.
 */
void CMotionTrail::
set_geom_node (GeomNode *geom_node) {
  _geom_node = geom_node;
}

/**
 * Add a vertex.
 */
void CMotionTrail::
add_vertex (LVector4 *vertex, LVector4 *start_color, LVector4 *end_color, PN_stdfloat v) {

  CMotionTrailVertex motion_trail_vertex;

  motion_trail_vertex._vertex = *vertex;
  motion_trail_vertex._start_color = *start_color;
  motion_trail_vertex._end_color = *end_color;
  motion_trail_vertex._v = v;

  motion_trail_vertex._nurbs_curve_evaluator = new NurbsCurveEvaluator ( );

  _vertex_list.push_back (motion_trail_vertex);
}

/**
 * Set motion trail parameters.
 *
 * sampling_time = Can be used to specify a lower sampling rate than the frame
 * rate.  Use 0.0 with nurbs.
 *
 * time_window = a component for the "length" of the motion trail.  The motion
 * trail length = time_window * velocity of the object.
 *
 * use_texture = texture option on/off.
 *
 * calculate_relative_matrix = calculate relative matrix on/off.
 *
 * use_nurbs = nurbs option on/off
 *
 * resolution_distance = the distance used to determine the number of geometry
 * samples.  samples = motion trail length / resolution_distance.  Applicable
 * only if nurbs is on.
 */
void CMotionTrail::
set_parameters (PN_stdfloat sampling_time, PN_stdfloat time_window, bool use_texture, bool calculate_relative_matrix, bool use_nurbs, PN_stdfloat resolution_distance) {

  _sampling_time = sampling_time;
  _time_window = time_window;
  _use_texture = use_texture;
  _calculate_relative_matrix = calculate_relative_matrix;
  _use_nurbs = use_nurbs;
  _resolution_distance = resolution_distance;
}

/**
 * Check if a sample can be submitted.
 */
int CMotionTrail::
check_for_update (PN_stdfloat current_time) {

  int state;

  state = false;
  if ((current_time - _last_update_time) >= _sampling_time) {
      state = true;
  }
  if (_pause) {
      state = false;
  }
  state = state && _enable;

  return state;
}

PN_stdfloat one_minus_x (PN_stdfloat x) {
  x = 1.0 - x;
  if (x < 0.0) {
    x = 0.0;
  }

  return x;
}

/**
 *
 */
void CMotionTrail::
begin_geometry ( ) {

  const GeomVertexFormat *format;

  _vertex_index = 0;
  if (_use_texture) {
    format = GeomVertexFormat::get_v3c4t2 ( );
  }
  else {
    format = GeomVertexFormat::get_v3c4 ( );
  }

  // Clear the previous writers before we create a new vertex data object--
  // this seems to work around an ordering problem in the low-level vertex
  // data destructors.
  _vertex_writer.clear();
  _color_writer.clear();
  _texture_writer.clear();

  _vertex_data = new GeomVertexData ("vertices", format, Geom::UH_static);
  _vertex_writer = GeomVertexWriter (_vertex_data, "vertex");
  _color_writer = GeomVertexWriter (_vertex_data, "color");
  if (_use_texture) {
    _texture_writer = GeomVertexWriter (_vertex_data, "texcoord");
  }

  _triangles = new GeomTriangles (Geom::UH_static);
}

/**
 * LVector3 vertex version.
 */
void CMotionTrail::
add_geometry_quad (LVector3 &v0, LVector3 &v1, LVector3 &v2, LVector3 &v3, LVector4 &c0, LVector4 &c1, LVector4 &c2, LVector4 &c3, LVector2 &t0, LVector2 &t1, LVector2 &t2, LVector2 &t3) {

  _vertex_writer.add_data3 (v0);
  _vertex_writer.add_data3 (v1);
  _vertex_writer.add_data3 (v2);
  _vertex_writer.add_data3 (v3);

  _color_writer.add_data4 (c0);
  _color_writer.add_data4 (c1);
  _color_writer.add_data4 (c2);
  _color_writer.add_data4 (c3);

  if (_use_texture) {
    _texture_writer.add_data2 (t0);
    _texture_writer.add_data2 (t1);
    _texture_writer.add_data2 (t2);
    _texture_writer.add_data2 (t3);
  }

  int vertex_index;
  vertex_index = _vertex_index;

  _triangles -> add_vertex (vertex_index + 0);
  _triangles -> add_vertex (vertex_index + 1);
  _triangles -> add_vertex (vertex_index + 2);
  _triangles -> close_primitive ( );

  _triangles -> add_vertex (vertex_index + 1);
  _triangles -> add_vertex (vertex_index + 3);
  _triangles -> add_vertex (vertex_index + 2);
  _triangles -> close_primitive ( );

  _vertex_index += 4;
}

/**
 * LVector4 vertex version.
 */
void CMotionTrail::
add_geometry_quad (LVector4 &v0, LVector4 &v1, LVector4 &v2, LVector4 &v3, LVector4 &c0, LVector4 &c1, LVector4 &c2, LVector4 &c3, LVector2 &t0, LVector2 &t1, LVector2 &t2, LVector2 &t3) {

  _vertex_writer.add_data3 (v0 [0], v0 [1], v0 [2]);
  _vertex_writer.add_data3 (v1 [0], v1 [1], v1 [2]);
  _vertex_writer.add_data3 (v2 [0], v2 [1], v2 [2]);
  _vertex_writer.add_data3 (v3 [0], v3 [1], v3 [2]);

  _color_writer.add_data4 (c0);
  _color_writer.add_data4 (c1);
  _color_writer.add_data4 (c2);
  _color_writer.add_data4 (c3);

  if (_use_texture) {
    _texture_writer.add_data2 (t0);
    _texture_writer.add_data2 (t1);
    _texture_writer.add_data2 (t2);
    _texture_writer.add_data2 (t3);
  }

  int vertex_index;
  vertex_index = _vertex_index;

  _triangles -> add_vertex (vertex_index + 0);
  _triangles -> add_vertex (vertex_index + 1);
  _triangles -> add_vertex (vertex_index + 2);
  _triangles -> close_primitive ( );

  _triangles -> add_vertex (vertex_index + 1);
  _triangles -> add_vertex (vertex_index + 3);
  _triangles -> add_vertex (vertex_index + 2);
  _triangles -> close_primitive ( );

  _vertex_index += 4;
}

/**
 *
 */
void CMotionTrail::end_geometry ( ) {
  static CPT(RenderState) state;
  if (state == nullptr) {
    state = RenderState::make(ColorAttrib::make_vertex());
  }

  PT(Geom) geometry;

  geometry = new Geom (_vertex_data);
  geometry -> add_primitive (_triangles);

  if (_geom_node) {
    _geom_node -> remove_all_geoms ( );
    _geom_node -> add_geom (geometry, state);
  }
}

/**
 * See class header comments.
 */
void CMotionTrail::
update_motion_trail (PN_stdfloat current_time, LMatrix4 *transform) {

  int debug;
  int total_frames;

  debug = false;

  total_frames = _frame_list.size ( );
  if (total_frames >= 1) {
    FrameList::iterator frame_iterator;
    CMotionTrailFrame motion_trail_frame;

    frame_iterator = _frame_list.begin ( );
    motion_trail_frame = *frame_iterator;
    if (*transform == motion_trail_frame._transform) {
      // duplicate transform
      return;
    }
  }

  int total_vertices;
  PN_stdfloat color_scale;
  LMatrix4 start_transform;
  LMatrix4 end_transform;
  LMatrix4 inverse_matrix;

  total_vertices = _vertex_list.size ( );
  color_scale = _color_scale;
  if (_fade) {
    PN_stdfloat elapsed_time;

    elapsed_time = current_time - _fade_start_time;
    if (elapsed_time < 0.0) {
      elapsed_time = 0.0;
    }
    if (elapsed_time < _fade_time) {
      color_scale = (1.0f - (elapsed_time / _fade_time)) * color_scale;
    }
    else {
      color_scale = 0.0;
      _fade_end = true;
    }
  }

  _last_update_time = current_time;

  // remove expired frames
  PN_stdfloat minimum_time;

  minimum_time = current_time - _time_window;

  CMotionTrailFrame motion_trail_frame;

  while (!_frame_list.empty()) {
    motion_trail_frame = _frame_list.back();
    if (motion_trail_frame._time >= minimum_time) {
      break;
    }

    _frame_list.pop_back ( );
  }

  // add new frame to beginning of list
  {
    CMotionTrailFrame motion_trail_frame;

    motion_trail_frame._time = current_time;
    motion_trail_frame._transform = *transform;

    _frame_list.push_front(motion_trail_frame);
  }

  // convert frames and vertices to geometry
  total_frames = _frame_list.size ( );

  if (debug) {
    printf ("update_motion_trail, total_frames = %d, total_vertices = %d, nurbs = %d, _calculate_relative_matrix = %d \n", total_frames, total_vertices, _use_nurbs, _calculate_relative_matrix);
  }

  if ((total_frames >= 2) && (total_vertices >= 2)) {
    int total_segments;
    PN_stdfloat minimum_time;
    PN_stdfloat delta_time;
    CMotionTrailFrame last_motion_trail_frame;

    VertexList::iterator vertex_iterator;

    // convert vertex list to vertex array
    int index = 0;
    _vertex_array = new CMotionTrailVertex [total_vertices];
    for (vertex_iterator = _vertex_list.begin ( ); vertex_iterator != _vertex_list.end ( ); vertex_iterator++) {
      _vertex_array [index] = *vertex_iterator;
      index++;
    }

    // begin geometry
    this -> begin_geometry ( );

    total_segments = total_frames - 1;

    last_motion_trail_frame = _frame_list.back();
    minimum_time = last_motion_trail_frame._time;
    delta_time = current_time - minimum_time;

    if (_calculate_relative_matrix) {
      inverse_matrix = *transform;
      inverse_matrix.invert_in_place ( );
    }

    if (_use_nurbs && (total_frames >= 5)) {

      // nurbs version
      int total_vertex_segments;
      PN_stdfloat total_distance;
      LVector3 vector;
      LVector4 v;
      LVector4 v0;
      LVector4 v1;
      LVector4 v2;
      LVector4 v3;

      total_vertex_segments = total_vertices - 1;
      total_distance = 0.0f;

      // reset NurbsCurveEvaluators for each vertex (the starting point for
      // the trail)
      {
        CMotionTrailVertex *motion_trail_vertex;
        PT(NurbsCurveEvaluator) nurbs_curve_evaluator;

        for (index = 0; index < total_vertices; index++) {
          motion_trail_vertex = &_vertex_array [index];
          nurbs_curve_evaluator = motion_trail_vertex -> _nurbs_curve_evaluator;
          nurbs_curve_evaluator -> set_order (4);
          nurbs_curve_evaluator -> reset (total_segments);
        }
      }

      // add vertices to each NurbsCurveEvaluator
      int segment_index;
      CMotionTrailFrame motion_trail_frame_start;
      CMotionTrailFrame motion_trail_frame_end;

      segment_index = 0;

      FrameList::iterator frame_iterator;
      frame_iterator = _frame_list.begin ( );
      while (segment_index < total_segments) {
        int vertex_segement_index;

        motion_trail_frame_start = *frame_iterator;
        frame_iterator++;
        motion_trail_frame_end = *frame_iterator;

        if (_calculate_relative_matrix) {
          start_transform.multiply (motion_trail_frame_start._transform, inverse_matrix);
          end_transform.multiply (motion_trail_frame_end._transform, inverse_matrix);
        }
        else {
          start_transform = motion_trail_frame_start._transform;
          end_transform = motion_trail_frame_end._transform;
        }

        CMotionTrailVertex *motion_trail_vertex_start;
        CMotionTrailVertex *motion_trail_vertex_end;
        PT(NurbsCurveEvaluator) nurbs_curve_evaluator;

        motion_trail_vertex_start = &_vertex_array [0];

        v0 = start_transform.xform (motion_trail_vertex_start -> _vertex);
        v2 = end_transform.xform (motion_trail_vertex_start -> _vertex);

        nurbs_curve_evaluator = motion_trail_vertex_start -> _nurbs_curve_evaluator;
        nurbs_curve_evaluator -> set_vertex (segment_index, v0);

        vertex_segement_index = 0;
        while (vertex_segement_index < total_vertex_segments) {
          motion_trail_vertex_start = &_vertex_array [vertex_segement_index];
          motion_trail_vertex_end = &_vertex_array [vertex_segement_index + 1];

          v1 = start_transform.xform (motion_trail_vertex_end -> _vertex);
          v3 = end_transform.xform (motion_trail_vertex_end -> _vertex);

          nurbs_curve_evaluator = motion_trail_vertex_end -> _nurbs_curve_evaluator;

          nurbs_curve_evaluator -> set_vertex (segment_index, v1);
          if (vertex_segement_index == (total_vertex_segments - 1)) {
            PN_stdfloat distance;

            v = v1 - v3;
            vector.set (v[0], v[1], v[2]);
            distance = vector.length();
            total_distance += distance;
          }

          vertex_segement_index += 1;
        }

        segment_index += 1;
      }

      // evaluate NurbsCurveEvaluator for each vertex
      PT(NurbsCurveResult) *nurbs_curve_result_array;

      nurbs_curve_result_array = new PT(NurbsCurveResult) [total_vertices];
      for (index = 0; index < total_vertices; index++) {

        CMotionTrailVertex *motion_trail_vertex;
        PT(NurbsCurveEvaluator) nurbs_curve_evaluator;
        PT(NurbsCurveResult) nurbs_curve_result;

        motion_trail_vertex = &_vertex_array [index];

        nurbs_curve_evaluator = motion_trail_vertex -> _nurbs_curve_evaluator;
        nurbs_curve_result = nurbs_curve_evaluator -> evaluate ( );
        nurbs_curve_result_array [index] = nurbs_curve_result;

        if (debug) {
          PN_stdfloat nurbs_start_t;
          PN_stdfloat nurbs_end_t;

          nurbs_start_t = nurbs_curve_result -> get_start_t();
          nurbs_end_t = nurbs_curve_result -> get_end_t();

          printf ("nurbs_start_t %f, nurbs_end_t %f \n", nurbs_start_t, nurbs_end_t);
        }
      }

      // create quads from NurbsCurveResult
      PN_stdfloat total_curve_segments;

      total_curve_segments = (total_distance / _resolution_distance);
      if (total_curve_segments < total_segments) {
        total_curve_segments = total_segments;
      }

      {
        LVector3 v0;
        LVector3 v1;
        LVector3 v2;
        LVector3 v3;

        LVector4 c0;
        LVector4 c1;
        LVector4 c2;
        LVector4 c3;

        LVector2 t0;
        LVector2 t1;
        LVector2 t2;
        LVector2 t3;

        LVector4 vertex_start_color;
        LVector4 vertex_end_color;

        PN_stdfloat curve_segment_index;

        curve_segment_index = 0.0;
        while (curve_segment_index < total_curve_segments) {

          PN_stdfloat st;
          PN_stdfloat et;
          PN_stdfloat start_t;
          PN_stdfloat end_t;
          PN_stdfloat color_start_t;
          PN_stdfloat color_end_t;

          int vertex_segement_index;

          CMotionTrailVertex *motion_trail_vertex_start;
          CMotionTrailVertex *motion_trail_vertex_end;
          PT(NurbsCurveResult) start_nurbs_curve_result;
          PT(NurbsCurveResult) end_nurbs_curve_result;

          vertex_segement_index = 0;

          st = curve_segment_index / total_curve_segments;
          et = (curve_segment_index + 1.0) / total_curve_segments;

          start_t = st;
          end_t = et;

          if (_square_t) {
            start_t *= start_t;
            end_t *= end_t;
          }

          motion_trail_vertex_start = &_vertex_array [0];

          vertex_start_color = motion_trail_vertex_start -> _end_color + (motion_trail_vertex_start -> _start_color - motion_trail_vertex_start  -> _end_color);

          color_start_t = color_scale * start_t;
          color_end_t = color_scale * end_t;

          c0 = vertex_start_color * one_minus_x (color_start_t);
          c2 = vertex_start_color * one_minus_x (color_end_t);

          t0.set (one_minus_x (st), motion_trail_vertex_start -> _v);
          t2.set (one_minus_x (et), motion_trail_vertex_start -> _v);

          while (vertex_segement_index < total_vertex_segments) {

            PN_stdfloat start_nurbs_start_t;
            PN_stdfloat start_nurbs_end_t;
            PN_stdfloat end_nurbs_start_t;
            PN_stdfloat end_nurbs_end_t;

            motion_trail_vertex_start = &_vertex_array [vertex_segement_index];
            motion_trail_vertex_end = &_vertex_array [vertex_segement_index + 1];

            start_nurbs_curve_result = nurbs_curve_result_array [vertex_segement_index];
            end_nurbs_curve_result = nurbs_curve_result_array [vertex_segement_index + 1];

            start_nurbs_start_t = start_nurbs_curve_result -> get_start_t();
            start_nurbs_end_t = start_nurbs_curve_result -> get_end_t();
            end_nurbs_start_t = end_nurbs_curve_result -> get_start_t();
            end_nurbs_end_t = end_nurbs_curve_result -> get_end_t();

            PN_stdfloat start_delta_t;
            PN_stdfloat end_delta_t;

            start_delta_t = (start_nurbs_end_t - start_nurbs_start_t);
            end_delta_t = (end_nurbs_end_t - end_nurbs_start_t);

            start_nurbs_curve_result -> eval_point (start_nurbs_start_t + (start_delta_t * st), v0);
            end_nurbs_curve_result -> eval_point (end_nurbs_start_t + (end_delta_t * st), v1);

            start_nurbs_curve_result -> eval_point (start_nurbs_start_t + (start_delta_t * et), v2);
            end_nurbs_curve_result -> eval_point (end_nurbs_start_t + (end_delta_t * et), v3);

            // color
            vertex_end_color = motion_trail_vertex_end -> _end_color + (motion_trail_vertex_end -> _start_color - motion_trail_vertex_end -> _end_color);

            c1 = vertex_end_color * one_minus_x (color_start_t);
            c3 = vertex_end_color * one_minus_x (color_end_t);

            // uv
            t1.set (one_minus_x (st), motion_trail_vertex_end -> _v);
            t3.set (one_minus_x (et), motion_trail_vertex_end -> _v);

            this -> add_geometry_quad (v0, v1, v2, v3, c0, c1, c2, c3, t0, t1, t2, t3);

            // reuse calculations
            c0 = c1;
            c2 = c3;

            t0 = t1;
            t2 = t3;

            vertex_segement_index += 1;
          }

          curve_segment_index += 1.0;
        }
      }

      for (index = 0; index < total_vertices; index++) {
        nurbs_curve_result_array [index] = nullptr;
      }

      delete[] nurbs_curve_result_array;
    }
    else {

      // non-nurbs version
      int segment_index;
      int vertex_segment_index;
      int total_vertex_segments;

      PN_stdfloat st;
      PN_stdfloat et;
      PN_stdfloat start_t;
      PN_stdfloat end_t;
      PN_stdfloat color_start_t;
      PN_stdfloat color_end_t;

      LVector4 v0;
      LVector4 v1;
      LVector4 v2;
      LVector4 v3;

      LVector4 c0;
      LVector4 c1;
      LVector4 c2;
      LVector4 c3;

      LVector2 t0;
      LVector2 t1;
      LVector2 t2;
      LVector2 t3;

      LVector4 vertex_start_color;
      LVector4 vertex_end_color;

      CMotionTrailFrame motion_trail_frame_start;
      CMotionTrailFrame motion_trail_frame_end;

      segment_index = 0;
      FrameList::iterator frame_iterator;
      frame_iterator = _frame_list.begin ( );
      while (segment_index < total_segments) {

        CMotionTrailVertex *motion_trail_vertex_start;
        CMotionTrailVertex *motion_trail_vertex_end;

        motion_trail_frame_start = *frame_iterator;
        frame_iterator++;
        motion_trail_frame_end = *frame_iterator;

        start_t = (motion_trail_frame_start._time - minimum_time) / delta_time;
        end_t = (motion_trail_frame_end._time - minimum_time) / delta_time;

        st = start_t;
        et = end_t;

        if (_square_t) {
          start_t *= start_t;
          end_t *= end_t;
        }

        vertex_segment_index = 0;
        total_vertex_segments = total_vertices - 1;

        if (_calculate_relative_matrix) {
          start_transform.multiply (motion_trail_frame_start._transform, inverse_matrix);
          end_transform.multiply (motion_trail_frame_end._transform, inverse_matrix);
        }
        else {
          start_transform = motion_trail_frame_start._transform;
          end_transform = motion_trail_frame_end._transform;
        }

        motion_trail_vertex_start = &_vertex_array [0];

        v0 = start_transform.xform (motion_trail_vertex_start -> _vertex);
        v2 = end_transform.xform (motion_trail_vertex_start -> _vertex);

        vertex_start_color = motion_trail_vertex_start -> _end_color + (motion_trail_vertex_start -> _start_color - motion_trail_vertex_start -> _end_color);
        color_start_t = color_scale * start_t;
        color_end_t = color_scale * end_t;
        c0 = vertex_start_color * color_start_t;
        c2 = vertex_start_color * color_end_t;

        t0.set (st, motion_trail_vertex_start -> _v);
        t2.set (et, motion_trail_vertex_start -> _v);

        while (vertex_segment_index < total_vertex_segments) {

          motion_trail_vertex_start = &_vertex_array [vertex_segment_index];
          motion_trail_vertex_end = &_vertex_array [vertex_segment_index + 1];

          v1 = start_transform.xform (motion_trail_vertex_end -> _vertex);
          v3 = end_transform.xform (motion_trail_vertex_end -> _vertex);

          // color
          vertex_end_color = motion_trail_vertex_end -> _end_color + (motion_trail_vertex_end -> _start_color - motion_trail_vertex_end -> _end_color);

          c1 = vertex_end_color * color_start_t;
          c3 = vertex_end_color * color_end_t;

          // uv
          t1.set (st, motion_trail_vertex_end -> _v);
          t3.set (et, motion_trail_vertex_end -> _v);

          this -> add_geometry_quad (v0, v1, v2, v3, c0, c1, c2, c3, t0, t1, t2, t3);

          // reuse calculations
          v0 = v1;
          v2 = v3;

          c0 = c1;
          c2 = c3;

          t0 = t1;
          t2 = t3;

          vertex_segment_index += 1;
        }

        segment_index += 1;
      }
    }

    // end geometry
    this -> end_geometry ( );

    delete[] _vertex_array;
    _vertex_array = nullptr;
  }
}
