
#ifndef CMOTIONTRAIL_H
#define CMOTIONTRAIL_H

#include "directbase.h"

#include "geomNode.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomTriangles.h"
#include "lmatrix4.h"
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

class EXPCL_DIRECT CMotionTrail : public TypedReferenceCount {

PUBLISHED:

  CMotionTrail ( );
  ~CMotionTrail ( );

  void reset ( );  
  void reset_vertex_list ( );

  void enable (int enable);
  void set_geom_node (PT(GeomNode) geom_node);
  void use_nurbs (int enable);
  void use_texture (int enable);
  void calculate_relative_matrix (int enable);  
  void add_vertex (LVector4f *vertex, LVector4f *start_color, LVector4f *end_color, float v);

  int check_for_update (float current_time);
  void update_motion_trail (float current_time, LMatrix4f *transform);

public:

  void begin_geometry ( );
  void add_geometry_quad (LVector3f &v0, LVector3f &v1, LVector3f &v2, LVector3f &v3, LVector4f &c0, LVector4f &c1, LVector4f &c2, LVector4f &c3, LVector2f &t0, LVector2f &t1, LVector2f &t2, LVector2f &t3);
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

  CMotionTrailVertex *_vertex_array;

  float _last_update_time;

  int _use_texture;


  list <CMotionTrailVertex> _vertex_list;
  list <CMotionTrailFrame> _frame_list;

//  parent_node_path = parent_node_path

  int _calculate_relative_matrix;

  int _playing;

  // default options
  int _continuous_motion_trail;
  float _color_scale;
  float _time_window;
  float _sampling_time;
  int _square_t;

//  int _task_transform;

//  root_node_path = None

  // node path states
  PT(GeomNode) _geom_node;

//  geom_node_path = 

  int _relative_to_render;

  // nurbs parameters
  int _use_nurbs;
  float _resolution_distance;
  
    
  // real-time data
  int _vertex_index;
  PT(GeomVertexData) _vertex_data;
  GeomVertexWriter _vertex_writer;
  GeomVertexWriter _color_writer;
  GeomVertexWriter _texture_writer;
  PT(GeomTriangles) _triangles;  

/*
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
*/

};

#endif
