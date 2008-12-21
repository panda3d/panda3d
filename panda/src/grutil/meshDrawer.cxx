// Filename: meshDrawer.cxx
// Created by:  treeform (19dec08)
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

#include "meshDrawer.h"

#include "geomVertexFormat.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomVertexRewriter.h"
#include "camera.h"
#include "boundingSphere.h"
#include "geomTristrips.h"
#include "geomTriangles.h"
#include "geom.h"
#include "geomNode.h"
#include "pnmPainter.h"
#include "pnmBrush.h"
#include "lvecBase4.h"
#include "lvector3.h"
#include "pandaNode.h"

TypeHandle MeshDrawer::_type_handle;

float randFloat() {
  return ((float) rand() / (float) 0x7fffffff);
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::generator
//       Access: Private
//  Description: Creates a system with a given budget.
////////////////////////////////////////////////////////////////////
void MeshDrawer::generator(int budget) {
  // create enough triangles for budget:
  _vdata = new GeomVertexData(_root.get_name(), GeomVertexFormat::get_v3n3c4t2(), Geom::UH_static);//UH_dynamic);
  GeomVertexWriter *tvertex = new GeomVertexWriter(_vdata, "vertex");
  GeomVertexWriter *tnormal = new GeomVertexWriter(_vdata, "normal");
  GeomVertexWriter *tuv = new GeomVertexWriter(_vdata, "texcoord");
  GeomVertexWriter *tcolor = new GeomVertexWriter(_vdata, "color");
  _prim = new GeomTriangles(Geom::UH_static);

  // iterate and fill _up a geom with random data so that it will
  // not be optimized out by panda3d system
  for(int i = 0; i < budget; i++) {
    for( int vert = 0; vert < 3; vert++) {
      LVector3f vec3 = LVector3f(randFloat()+1000,randFloat(),randFloat())*.001;
      LVector4f vec4 = LVector4f(1,1,1,randFloat());
      LVector2f vec2 = LVector2f(0,randFloat());
      tvertex->add_data3f(vec3);
      tcolor->add_data4f(vec4);
      tuv->add_data2f(vec2);
      tnormal->add_data3f(vec3);
    }
    _prim->add_vertices(i * 3, i * 3 + 1, i * 3 + 2);
  }
  // create our node and attach it to this node path
  _prim->close_primitive();
  _geom = new Geom(_vdata);
  _geom->add_primitive(_prim);
  _geomnode = new GeomNode("__MeshDrawer_GeomNode");
  _geomnode->add_geom(_geom);
  _root.attach_new_node(_geomnode);
  _last_clear_index = budget;

  delete tvertex;
  delete tnormal;
  delete tuv;
  delete tcolor;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::begin
//       Access: Published
//  Description: Pass the current camera node and the root node.
////////////////////////////////////////////////////////////////////
void MeshDrawer::begin(NodePath camera, NodePath render) {
  // sanity check
  assert(render.get_error_type() == NodePath::ET_ok);
  assert(camera.get_error_type() == NodePath::ET_ok);

  // remember our arguments
  _camera = camera;
  _render = render;

  // compute some help vectors
  _eyePos = camera.get_pos();
  _up = _render.get_relative_vector(camera, LVector3f(0, 0, 1));
  _right = _render.get_relative_vector(camera, LVector3f(1, 0, 0));
  _b1 = - _right - _up;
  _b2 =   _right - _up;
  _b3 =   _right + _up;
  _b4 = - _right + _up;

  // recreate our rewriters
  if (_vertex != NULL) delete _vertex;
  if (_normal != NULL) delete _normal;
  if (_uv != NULL)     delete _uv;
  if (_color != NULL)  delete _color;
  _vertex = new GeomVertexRewriter(_vdata, "vertex");
  _uv = new GeomVertexRewriter(_vdata, "texcoord");
  _normal = new GeomVertexRewriter(_vdata, "normal");
  _color = new GeomVertexRewriter(_vdata, "color");
  _dprim = _prim->decompose();

  // reseta our clearning indexes
  _start_clear_index = 0;
  _end_clear_index = _budget;
  _clear_index = _start_clear_index;

}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::end
//       Access: Published
//  Description: Finish the drawing and clearing off the remaining
//               vertexes.
////////////////////////////////////////////////////////////////////
void MeshDrawer::end() {

  // clear the unused triangles at the end of the buffer
  for(int i = _clear_index ; i < _last_clear_index; i ++ ) {
    _vertex->add_data3f(0,0,0);
    _vertex->add_data3f(0,0,0);
    _vertex->add_data3f(0,0,0);
  }
  // dont clear more then you have too
  _last_clear_index = _clear_index;

  // delete the re writers
  delete _vertex; _vertex = NULL;
  delete _uv;     _uv     = NULL;
  delete _normal; _normal = NULL;
  delete _color;  _color  = NULL;

}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::particle
//       Access: Published
//  Description: Draws a particle that is sort of like a bill board
//               but has an extra rotation component.
////////////////////////////////////////////////////////////////////
void MeshDrawer::particle(LVector3f pos, int frame, float size, LVector4f color, float rotation) {

  rotation = rotation / 57.29578;

  LVector3f v1 = pos + _b1*size*sin(rotation) + _b2*size*cos(rotation);
  LVector3f v2 = pos + _b2*size*sin(rotation) + _b3*size*cos(rotation);
  LVector3f v3 = pos + _b3*size*sin(rotation) + _b4*size*cos(rotation);
  LVector3f v4 = pos + _b4*size*sin(rotation) + _b1*size*cos(rotation);

  float u = float(int(frame%_plate_size))*_frame_size;
  float v = 1.0f-float(int(frame/_plate_size+1))*_frame_size;

  tri(
    v1, color, LVector2f(u,v),
    v2, color, LVector2f(u+_frame_size,v),
    v3, color, LVector2f(u+_frame_size,v+_frame_size));
  tri(
    v3, color, LVector2f(u+_frame_size,v+_frame_size),
    v4, color, LVector2f(u,v+_frame_size),
    v1, color, LVector2f(u,v));
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::blended_particle
//       Access: Published
//  Description: Works just like particle but accepts 2 frames and
//               a blend (from 0 to 1) component between them
////////////////////////////////////////////////////////////////////
void MeshDrawer::blended_particle(LVector3f pos, int frame1, int frame2,
         float blend, float size, LVector4f color, float rotation) {

  float original_w = color.get_w();
  color.set_w((1.f-blend)*original_w);
  particle(pos,frame1,size,color,rotation);
  color.set_w(blend*original_w);
  particle(pos,frame2,size,color,rotation);

}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::billboard
//       Access: Published
//  Description: Draws a billboard - particle with no rotation.
//               Billboards always face the camera.
////////////////////////////////////////////////////////////////////
void MeshDrawer::billboard(LVector3f pos, int frame, float size, LVector4f _color) {

  LVector3f v1 = pos + _b1*size;
  LVector3f v2 = pos + _b2*size;
  LVector3f v3 = pos + _b3*size;
  LVector3f v4 = pos + _b4*size;

  float u = float(int(frame%_plate_size))*_frame_size;
  float v = 1.0f-float(int(frame/_plate_size+1))*_frame_size;

  tri(
    v1, _color, LVector2f(u,v),
    v2, _color, LVector2f(u+_frame_size,v),
    v3, _color, LVector2f(u+_frame_size,v+_frame_size));
  tri(
    v3, _color, LVector2f(u+_frame_size,v+_frame_size),
    v4, _color, LVector2f(u,v+_frame_size),
    v1, _color, LVector2f(u,v));
}


////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::segment
//       Access: Published
//  Description: Draws a segment a line with a thickness. That has
//               billboarding effect.
////////////////////////////////////////////////////////////////////
void MeshDrawer::segment(LVector3f start, LVector3f stop, int frame,
           float thickness, LVector4f color) {
	link_segment(start, frame, thickness, color);
	link_segment(stop, frame, thickness, color);
	link_segment_end(frame, color);
}
////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::cross_segment
//       Access: Published
//  Description: Draws a segment a line with a thickness.  This
//               segment does not use the bill boarding behavior
//               and instead draws 2 planes in a cross.
////////////////////////////////////////////////////////////////////
void MeshDrawer::cross_segment(LVector3f start, LVector3f stop, int frame,
           float thickness, LVector4f color) {

  float u = float(int(frame%_plate_size))*_frame_size;
  float v = 1.0f-float(int(frame/_plate_size+1))*_frame_size;

  LVector3f v1 = start - _up*thickness;
  LVector3f v2 = stop - _up*thickness;
  LVector3f v3 = stop + _up*thickness;
  LVector3f v4 = start + _up*thickness;

  tri(v1, color, LVector2f(u,v),
      v2, color, LVector2f(u+_frame_size,v),
      v3, color, LVector2f(u+_frame_size,v+_frame_size));
  tri(v3, color, LVector2f(u+_frame_size,v+_frame_size),
      v4, color, LVector2f(u,v+_frame_size),
      v1, color, LVector2f(u,v));

  v1 = start - _right*thickness;
  v2 = stop - _right*thickness;
  v3 = stop + _right*thickness;
  v4 = start + _right*thickness;

  tri(v1, color, LVector2f(u,v),
      v2, color, LVector2f(u+_frame_size,v),
      v3, color, LVector2f(u+_frame_size,v+_frame_size));
  tri(v3, color, LVector2f(u+_frame_size,v+_frame_size),
      v4, color, LVector2f(u,v+_frame_size),
      v1, color, LVector2f(u,v));

}




////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::uneven_segment
//       Access: Published
//  Description: Draws a segment a line with different thickness
//               and color on both sides.
////////////////////////////////////////////////////////////////////
void MeshDrawer::uneven_segment(LVector3f start, LVector3f stop,
		int frame, int multi_frame,
		float thickness_start, LVector4f color_start,
		float thickness_stop, LVector4f color_stop) {

	float u = float(int(frame%_plate_size))*_frame_size;
	float v = 1.0f-float(int(frame/_plate_size+1))*_frame_size;

	LVector3f v1 = start - _up*thickness_start;
	LVector3f v2 = stop - _up*thickness_stop;
	LVector3f v3 = stop + _up*thickness_stop;
	LVector3f v4 = start + _up*thickness_start;

	tri(v1, color_start, LVector2f(u,v),
		  v2, color_stop, LVector2f(u+_frame_size*multi_frame,v),
		  v3, color_stop, LVector2f(u+_frame_size*multi_frame,v+_frame_size));
	tri(v3, color_stop, LVector2f(u+_frame_size*multi_frame,v+_frame_size),
		  v4, color_start, LVector2f(u,v+_frame_size),
		  v1, color_start, LVector2f(u,v));

	v1 = start - _right*thickness_start;
	v2 = stop - _right*thickness_stop;
	v3 = stop + _right*thickness_stop;
	v4 = start + _right*thickness_start;

	tri(v1, color_start, LVector2f(u,v),
		  v2, color_stop, LVector2f(u+_frame_size*multi_frame,v),
		  v3, color_stop, LVector2f(u+_frame_size*multi_frame,v+_frame_size));
	tri(v3, color_stop, LVector2f(u+_frame_size*multi_frame,v+_frame_size),
		  v4, color_start, LVector2f(u,v+_frame_size),
		  v1, color_start, LVector2f(u,v));

}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::explosion
//       Access: Published
//  Description: Draws number of particles in a sphere like emitter.
////////////////////////////////////////////////////////////////////
void MeshDrawer::explosion(
  LVector3f pos, int frame, float size, LVector4f _color,
  int seed, int number, float distance) {
  srand(seed);
  LVector3f relative_pos;
  for(int i = 0; i < number; i++) {
    relative_pos = LVector3f(randFloat()-.5f,randFloat()-.5f,randFloat()-.5f);
    relative_pos.normalize();
    relative_pos *= randFloat()*distance;
    particle(relative_pos+pos,frame,size,_color,randFloat()*360.0f);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::stream
//       Access: Published
//  Description: Draws a number of particles in a big line with a
//               shift dictated by the offset.
////////////////////////////////////////////////////////////////////
void MeshDrawer::stream(LVector3f start, LVector3f stop, int frame, float size, LVector4f _color,
        int number, float offset) {

  offset = offset-floor(offset);
  LVector3f relative_pos = stop;
  LVector3f vec = stop - start;
  float distance = vec.length();
  for(int i = 0; i < number; i++) {
    relative_pos = stop + vec * ((i-offset)*(distance/float(number)));
    billboard(relative_pos,frame,size,_color);
  }
}



////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::geometry
//       Access: Published
//  Description: Draws the geometry that is inside this node path into
//               the MeshDrawer object.  This performs a similar
//               functions as RigidBodyCombiner but for very
//               dynamic situations that share the same texture
//               like physcal chunks of explosions.
////////////////////////////////////////////////////////////////////
void MeshDrawer::geometry(NodePath draw_node) {
  assert(_render.get_error_type() == NodePath::ET_ok);

  LVector4f color = LVector4f(1,1,1,1);
  LVector3f vec[3];
  LVector2f uv[3];

  // process node
  NodePathCollection geom_collection = draw_node.find_all_matches("**/+GeomNode");
  for(int i=0; i < geom_collection.get_num_paths(); i++ ) {
    NodePath current_node_path = geom_collection.get_path(i);
    PT(GeomNode) geomNode = DCAST(GeomNode, current_node_path.node());

    // process geom node
    for(int j=0; j<geomNode->get_num_geoms(); j++) {
      CPT(Geom) geom = geomNode->get_geom(j);
      CPT(GeomVertexData) v_data = geom->get_vertex_data();
      GeomVertexReader *prim_vertex_reader = new GeomVertexReader(v_data, "vertex");
      GeomVertexReader *prim_uv_reader = new GeomVertexReader(v_data, "texcoord");
      for(int k=0; k <geom->get_num_primitives(); k++) {
        CPT(GeomPrimitive) prim1 = geom->get_primitive(k);
        CPT(GeomPrimitive) _prim  = prim1->decompose();

        // process primitive
        for(int p=0; p < _prim->get_num_primitives();p++) {
          int s = _prim->get_primitive_start(p);
          int e = _prim->get_primitive_end(p);
          int indx_over = 0;

          // process polygon
          for(int idx=s; idx<e; idx++) {
            int vidx = _prim->get_vertex(idx);
            prim_vertex_reader->set_row(vidx);
            prim_uv_reader->set_row(vidx);
            vec[indx_over] = _render.get_relative_point(
              current_node_path,prim_vertex_reader->get_data3f());
            uv[indx_over] = prim_uv_reader->get_data2f();
            indx_over++;
            if (indx_over > 2) break;
          }

          // draw polygon
          tri(vec[0],color,uv[0],
            vec[1],color,uv[1],
            vec[2],color,uv[2]);
        }
        // if we are over budget just quit
        if( _clear_index > _end_clear_index) return;
      }
      // delete our reders
      delete prim_vertex_reader;
      delete prim_uv_reader;
    }
  }
}



////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::link_segment
//       Access: Published
//  Description: Stars or continues linked segment.
//               Control position, frame, thickness and color with
//               parameters.
////////////////////////////////////////////////////////////////////
void MeshDrawer::link_segment(LVector3f pos, int frame,
		float thickness, LVector4f color) {
  assert(_render.get_error_type() == NodePath::ET_ok);
  assert(_camera.get_error_type() == NodePath::ET_ok);
	/*
	 * X
	 * ---X
	 * ===0---X
	 * ===0===0---X
	 * ===0===0===O---X
	 * ===0===0===0===End
	 *
	 * first call marks position X
	 * second call moves position and promises to draw segment
	 * it can't draw it yet because next segment might bend it
	 * third call finally draws segment
	 * and the chain continues till
	 * link_segment_end to flush the linking segments is called.
	 */

	// mark 1st position
	if(_at_start==0) {
		_last_pos = pos;
		_last_thickness = thickness;
		_last_color = color;
		_at_start=1;
		return;
	}

	LVector3f start = _last_pos;
	LVector3f stop = pos;

  LVector3f cam_start3d = _camera.get_relative_point(_render, start);
  LPoint2f cam_start2d = LVector2f();
  LVector3f cam_stop3d = _camera.get_relative_point(_render, stop);
  LPoint2f cam_stop2d = LVector2f();

  PT(Camera) camera = DCAST(Camera, _camera.node());
  PT(Lens) lens = camera->get_lens();

  bool start_good = lens->project(cam_start3d, cam_start2d);
  bool stop_good = lens->project(cam_stop3d, cam_stop2d);
  //if start_good and stop_good:

  LVector2f dif =  cam_stop2d - cam_start2d;
  float rotation = atan2(dif.get_x(),dif.get_y());

  LVector3f now_v1 = start + _b1*(float)(thickness*sin(rotation)) + _b2*(float)(thickness*cos(rotation));
  LVector3f now_v4 = start + _b4*(float)(thickness*sin(rotation)) + _b1*(float)(thickness*cos(rotation));
  LVector3f now_v2 = stop + _b2*(float)(thickness*sin(rotation)) + _b3*(float)(thickness*cos(rotation));
  LVector3f now_v3 = stop + _b3*(float)(thickness*sin(rotation)) + _b4*(float)(thickness*cos(rotation));

  // mark the segment we going to draw
  // we need to draw it when we know what the next segment looks like
  // because it can bend it a little
  if(_at_start==1) {
		_last_v1 = now_v1;
		_last_v2 = now_v2;
		_last_v3 = now_v3;
		_last_v4 = now_v4;
		_at_start = 2;
		return;
  }

  // draw the last segment a little bent
  LVector3f v1 = _last_v1;
  LVector3f v2 = (_last_v2+now_v1)/2.0f;
  LVector3f v3 = (_last_v3+now_v4)/2.0f;
  LVector3f v4 = _last_v4;

  // compute this frame
  float u = float(int(frame%_plate_size))*_frame_size;
  float v = 1.0f-float(int(frame/_plate_size+1))*_frame_size;

  tri(v1, _last_color, LVector2f(u,v),
    v2, color, LVector2f(u+_frame_size,v),
    v3, color, LVector2f(u+_frame_size,v+_frame_size));
  tri(v3, color, LVector2f(u+_frame_size,v+_frame_size),
    v4, _last_color, LVector2f(u,v+_frame_size),
    v1, _last_color, LVector2f(u,v));

  // save this segment
  _last_v1 = v2;
  _last_v2 = now_v2;
  _last_v3 = now_v3;
  _last_v4 = v3;

  // make this position
  _last_pos = pos;
  _last_thickness = thickness;
  _last_color = color;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer::link_segment_end
//       Access: Published
//  Description: Finish drawing linked segments, needs at least
//               two calls to link_segment before it can end
//               the linked segment.
////////////////////////////////////////////////////////////////////
void MeshDrawer::link_segment_end(int frame, LVector4f color)
{
  float u = float(int(frame%_plate_size))*_frame_size;
  float v = 1.0f-float(int(frame/_plate_size+1))*_frame_size;

  tri(_last_v1, _last_color, LVector2f(u,v),
      _last_v2, color, LVector2f(u+_frame_size,v),
      _last_v3, color, LVector2f(u+_frame_size,v+_frame_size));
  tri(_last_v3, color, LVector2f(u+_frame_size,v+_frame_size),
      _last_v4, _last_color, LVector2f(u,v+_frame_size),
      _last_v1, _last_color, LVector2f(u,v));

  _at_start = 0;
}
