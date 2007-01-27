
#include "directbase.h"
#include "cMotionTrail.h"


// TypeHandle CMotionTrail::_type_handle;

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

  _vertex_array = 0;
  _last_update_time = 0.0f;
  
  _use_texture = false;


//

//


  _calculate_relative_matrix = false;

  _playing = false;

  // default options
  _continuous_motion_trail = true;
  _color_scale = 1.0;
  _time_window = 1.0;
  _sampling_time = 0.0;
  _square_t = true;

//  _task_transform = false;

//  root_node_path = None

  // node path states
  _geom_node = 0;
//  geom_node_path = 

  _relative_to_render = false;

  // nurbs parameters
  _use_nurbs = !false;
  _resolution_distance = 0.5f;

  // real-time data
  _vertex_index = 0;
  _vertex_data = 0;
  _triangles = 0;  
}

CMotionTrail::
~CMotionTrail ( ) {

}

void CMotionTrail::
reset ( ) {
  _frame_list.clear ( );
}

void CMotionTrail::
reset_vertex_list ( ) {
  _vertex_list.clear ( );
}

void CMotionTrail::
enable (int enable) {
  _enable = enable;
}

void CMotionTrail::
set_geom_node (PT(GeomNode) geom_node) {
  _geom_node = geom_node;
}

void CMotionTrail::
use_nurbs (int enable) {
  _use_nurbs = enable;
}

void CMotionTrail::
use_texture (int enable) {
  _use_texture = enable;
}

void CMotionTrail::
calculate_relative_matrix (int enable) {
  _calculate_relative_matrix = enable;
}

void CMotionTrail::
add_vertex (LVector4f *vertex, LVector4f *start_color, LVector4f *end_color, float v) {

  CMotionTrailVertex motion_trail_vertex;

  motion_trail_vertex._vertex = *vertex;
  motion_trail_vertex._start_color = *start_color;
  motion_trail_vertex._end_color = *end_color;
  motion_trail_vertex._v = v;
  
  motion_trail_vertex._nurbs_curve_evaluator = new NurbsCurveEvaluator ( );

  _vertex_list.push_back (motion_trail_vertex);
}

int CMotionTrail::
check_for_update (float current_time) {

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

float one_minus_x (float x) {
  x = 1.0 - x;
  if (x < 0.0) {
    x = 0.0;
  }

  return x;
}

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

  _vertex_data = new GeomVertexData ("vertices", format, Geom::UH_static);
  _vertex_writer = GeomVertexWriter (_vertex_data, "vertex");
  _color_writer = GeomVertexWriter (_vertex_data, "color");
  if (_use_texture) {
    _texture_writer = GeomVertexWriter (_vertex_data, "texcoord");
  }
  
  _triangles = new GeomTriangles (Geom::UH_static);
}

void CMotionTrail::
add_geometry_quad (LVector3f &v0, LVector3f &v1, LVector3f &v2, LVector3f &v3, LVector4f &c0, LVector4f &c1, LVector4f &c2, LVector4f &c3, LVector2f &t0, LVector2f &t1, LVector2f &t2, LVector2f &t3) {

  _vertex_writer.add_data3f (v0);
  _vertex_writer.add_data3f (v1);
  _vertex_writer.add_data3f (v2);
  _vertex_writer.add_data3f (v3);

  _color_writer.add_data4f (c0);
  _color_writer.add_data4f (c1);
  _color_writer.add_data4f (c2);
  _color_writer.add_data4f (c3);

  if (_use_texture) {
    _texture_writer.add_data2f (t0);
    _texture_writer.add_data2f (t1);
    _texture_writer.add_data2f (t2);
    _texture_writer.add_data2f (t3);
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

void CMotionTrail::end_geometry ( ) {    

  PT(Geom) geometry;
  
  geometry = new Geom (_vertex_data);
  geometry -> add_primitive (_triangles);

  if (_geom_node) {
    _geom_node -> remove_all_geoms ( );
    _geom_node -> add_geom (geometry);
  }
}

void CMotionTrail::
update_motion_trail (float current_time, LMatrix4f *transform) {

  int debug;
  int total_frames;
  
  debug = true;

  total_frames = _frame_list.size ( );
  if (total_frames >= 1) {
    list <CMotionTrailFrame>::iterator frame_iterator;
    CMotionTrailFrame motion_trail_frame;
 
    frame_iterator = _frame_list.begin ( );
    motion_trail_frame = *frame_iterator;
    if (*transform == motion_trail_frame._transform) {
      // duplicate transform
      return;
    }
  }

  int total_vertices;
  float color_scale;
  LMatrix4f start_transform;
  LMatrix4f end_transform;
  LMatrix4f inverse_matrix;

  total_vertices = _vertex_list.size ( );
  color_scale = _color_scale;
  if (_fade)
  {
    float elapsed_time;
    
    elapsed_time = current_time - _fade_start_time;
    if (elapsed_time < 0.0)
    {
      elapsed_time = 0.0;
    }
    if (elapsed_time < _fade_time)
    {
      color_scale = (1.0f - (elapsed_time / _fade_time)) * color_scale;
    }
    else
    {
      color_scale = 0.0;
      _fade_end = true;
    }
  }

  _last_update_time = current_time;

  // remove expired frames
  float minimum_time;
  
  minimum_time = current_time - _time_window;
  
  int index;
  int last_frame_index;
  list <CMotionTrailFrame>::iterator frame_iterator;
  CMotionTrailFrame motion_trail_frame;
  
  index = 0;
  last_frame_index = total_frames - 1; 
  frame_iterator = _frame_list.end ( );
  while (index <= last_frame_index)
  {
    frame_iterator--;
    motion_trail_frame = *frame_iterator;
    if (motion_trail_frame._time >= minimum_time)
    {
      break;
    }
    else
    {
      _frame_list.pop_back ( );
    }
    
    index += 1;
  }
  
  // add new frame to beginning of list
  {
    CMotionTrailFrame motion_trail_frame;

    motion_trail_frame._time = current_time;
    motion_trail_frame._transform = *transform;

    _frame_list.push_front (motion_trail_frame);
  }

  // convert frames and vertices to geometry
  total_frames = _frame_list.size ( );

if (debug)
{
  printf ("update_motion_trail, total_frames = %d, total_vertices = %d, nurbs = %d, _calculate_relative_matrix = %d \n", total_frames, total_vertices, _use_nurbs, _calculate_relative_matrix);
}
  if ((total_frames >= 2) && (total_vertices >= 2))
  {
    int total_segments;
    float minimum_time;
    float delta_time;
    CMotionTrailFrame last_motion_trail_frame;

    list <CMotionTrailVertex>::iterator vertex_iterator;    

    // vertex list to vertex array
    index = 0;
    _vertex_array = new CMotionTrailVertex [total_vertices];
    for (vertex_iterator = _vertex_list.begin ( ); vertex_iterator != _vertex_list.end ( ); vertex_iterator++)
    {   
      _vertex_array [index] = *vertex_iterator;        
      index++;
    }
    
    // begin geometry
    this -> begin_geometry ( );

    total_segments = total_frames - 1;
      
    frame_iterator = _frame_list.end ( );
    frame_iterator--;
    last_motion_trail_frame = *frame_iterator;
    
    minimum_time = last_motion_trail_frame._time;
    delta_time = current_time - minimum_time;

_calculate_relative_matrix = true;

    if (_calculate_relative_matrix)      
    {
      inverse_matrix = *transform;
      inverse_matrix.invert_in_place ( );
    }
//    if (_use_nurbs && (total_frames >= 5))
    if ((total_frames >= 5))
    {
      // nurbs version

if (debug)
{
  printf ("nurbs\n");
//  __debugbreak();
}
      
      float total_distance;  
      LVector3f vector;
      LVector4f v;
      LVector4f v0;
      LVector4f v1;
      LVector4f v2;
      LVector4f v3;

      total_distance = 0.0f;

      int total_vertex_segments;

      total_vertex_segments = total_vertices - 1;

      // reset NurbsCurveEvaluators for each vertex (the starting point for the trail)

      for (index = 0; index < total_vertices; index++)
      {   
          CMotionTrailVertex *motion_trail_vertex;
          PT(NurbsCurveEvaluator) nurbs_curve_evaluator;
      
          motion_trail_vertex = &_vertex_array [index];
          nurbs_curve_evaluator = motion_trail_vertex -> _nurbs_curve_evaluator;
          nurbs_curve_evaluator -> reset (total_segments);
      }

      // add vertices to each NurbsCurveEvaluator            

      int segment_index;
      
      segment_index = 0;

      frame_iterator = _frame_list.begin ( );
      while (segment_index < total_segments)
      {
          int vertex_segement_index;
          CMotionTrailFrame motion_trail_frame_start;
          CMotionTrailFrame motion_trail_frame_end;
                  
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

//                        print "nurbs_curve_evaluator", nurbs_curve_evaluator, "index", (vertex_segement_index)
          nurbs_curve_evaluator -> set_vertex (segment_index, v0);

          vertex_segement_index = 0;
          while (vertex_segement_index < total_vertex_segments)
          {
              motion_trail_vertex_start = &_vertex_array [vertex_segement_index];
              motion_trail_vertex_end = &_vertex_array [vertex_segement_index + 1];

              v1 = start_transform.xform (motion_trail_vertex_end -> _vertex);
              v3 = end_transform.xform (motion_trail_vertex_end -> _vertex);

              nurbs_curve_evaluator = motion_trail_vertex_end -> _nurbs_curve_evaluator;


//                            print "nurbs_curve_evaluator", nurbs_curve_evaluator, "index", (vertex_segement_index + 1)


              nurbs_curve_evaluator -> set_vertex (segment_index, v1);

/*

              """
              print v0
              print v1
              print v2
              print v3
              """
*/

              if (vertex_segement_index == (total_vertex_segments - 1))
              {
                  float distance;
            
                  v = v1 - v3;
                  vector.set (v[0], v[1], v[2]);
                  distance = vector.length();
                  total_distance += distance;
// #                                print "DISTANCE", distance
              }
              
              vertex_segement_index += 1;
          }

//                    print "TOTAL DISTANCE", total_distance, "SEGMENTS", total_distance / self.resolution_distance


          segment_index += 1;
      }
      
      // evaluate NurbsCurveEvaluator for each vertex
      PT(NurbsCurveResult) *nurbs_curve_result_array;
      
      nurbs_curve_result_array = new PT(NurbsCurveResult) [total_vertices];

      for (index = 0; index < total_vertices; index++)
      {          
          CMotionTrailVertex *motion_trail_vertex;
          PT(NurbsCurveEvaluator) nurbs_curve_evaluator;
          PT(NurbsCurveResult) nurbs_curve_result;
          
          motion_trail_vertex = &_vertex_array [index];

          nurbs_curve_evaluator = motion_trail_vertex -> _nurbs_curve_evaluator;
          nurbs_curve_result = nurbs_curve_evaluator -> evaluate ( );
          nurbs_curve_result_array [index] = nurbs_curve_result;

          if (debug)
          {
              float nurbs_start_t;
              float nurbs_end_t;

              nurbs_start_t = nurbs_curve_result -> get_start_t();
              nurbs_end_t = nurbs_curve_result -> get_end_t();

              printf ("nurbs_start_t %f, nurbs_end_t %f \n", nurbs_start_t, nurbs_end_t);
          }
      }

      // create quads from NurbsCurveResult                    
      float total_curve_segments;
      
      total_curve_segments = (total_distance / _resolution_distance);
      if (total_curve_segments < total_segments)
      {
          total_curve_segments = total_segments;
      }

      {
        LVector3f v0;
        LVector3f v1;
        LVector3f v2;
        LVector3f v3;

        LVector4f c0;
        LVector4f c1;
        LVector4f c2;
        LVector4f c3;

        LVector2f t0;
        LVector2f t1;
        LVector2f t2;
        LVector2f t3;

        LVector4f vertex_start_color;
        LVector4f vertex_end_color;

        float curve_segment_index;

        curve_segment_index = 0.0;
        while (curve_segment_index < total_curve_segments)
        {   
            float st;
            float et;
            float start_t;
            float end_t;
            float color_start_t;
            float color_end_t;

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

            if (_square_t)
            {
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

            while (vertex_segement_index < total_vertex_segments)
            {
                float start_nurbs_start_t;
                float start_nurbs_end_t;
                float end_nurbs_start_t;
                float end_nurbs_end_t;
            
                motion_trail_vertex_start = &_vertex_array [vertex_segement_index];
                motion_trail_vertex_end = &_vertex_array [vertex_segement_index + 1];

                start_nurbs_curve_result = nurbs_curve_result_array [vertex_segement_index];
                end_nurbs_curve_result = nurbs_curve_result_array [vertex_segement_index + 1];

                start_nurbs_start_t = start_nurbs_curve_result -> get_start_t();
                start_nurbs_end_t = start_nurbs_curve_result -> get_end_t();
                end_nurbs_start_t = end_nurbs_curve_result -> get_start_t();
                end_nurbs_end_t = end_nurbs_curve_result -> get_end_t();

                float start_delta_t;
                float end_delta_t;

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

      delete nurbs_curve_result_array;
    }
    else
    {
      // non-nurbs version
  
  
    }

    // end geometry
    {
      this -> end_geometry ( );   
    }

    delete _vertex_array;
    _vertex_array = 0;
  }
}
