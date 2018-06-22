/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file meshDrawer.cxx
 * @author treeform
 * @date 2008-12-19
 */

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

PN_stdfloat randFloat() {
  return ((PN_stdfloat)rand() / (PN_stdfloat)RAND_MAX);
}

/**
 * Creates a system with a given budget.
 */
void MeshDrawer::generator(int budget) {
  // create enough triangles for budget:
  _vdata = new GeomVertexData(_root.get_name(), GeomVertexFormat::get_v3n3c4t2(), Geom::UH_static);//UH_dynamic);
  _vdata->unclean_set_num_rows(budget * 3);

  {
    GeomVertexWriter tvertex(_vdata, "vertex");
    GeomVertexWriter tnormal(_vdata, "normal");
    GeomVertexWriter tuv(_vdata, "texcoord");
    GeomVertexWriter tcolor(_vdata, "color");

    // iterate and fill _up a geom with random data so that it will not be
    // optimized out by panda3d system
    for (int i = 0; i < budget; i++) {
      for (int vert = 0; vert < 3; vert++) {
        LVector3 vec3 = LVector3(randFloat()+1000,randFloat(),randFloat())*.001;
        LVector4 vec4 = LVector4(1,1,1,randFloat());
        LVector2 vec2 = LVector2(0,randFloat());
        tvertex.set_data3(vec3);
        tcolor.set_data4(vec4);
        tuv.set_data2(vec2);
        tnormal.set_data3(vec3);
      }
    }
  }

  // create our node and attach it to this node path
  _prim = new GeomTriangles(Geom::UH_static);
  _prim->add_next_vertices(budget * 3);
  _prim->close_primitive();
  _geom = new Geom(_vdata);
  _geom->add_primitive(_prim);
  if (_geomnode == nullptr) {
    _geomnode = new GeomNode("__MeshDrawer_GeomNode");
    _root.attach_new_node(_geomnode);
  } else {
    _geomnode->remove_all_geoms();
  }
  _geomnode->add_geom(_geom);
  _last_clear_index = budget;
}

/**
 * Pass the current camera node and the root node.  Passing the camera is
 * required to generate bill boards that face it.
 */
void MeshDrawer::begin(NodePath camera, NodePath render) {
  // sanity check
  assert(render.get_error_type() == NodePath::ET_ok);
  assert(camera.get_error_type() == NodePath::ET_ok);

  // remember our arguments
  _camera = camera;
  _render = render;

  // compute some help vectors
  _eyePos = camera.get_pos();
  _up = _render.get_relative_vector(camera, LVector3(0, 0, 1));
  _right = _render.get_relative_vector(camera, LVector3(1, 0, 0));
  _b1 = - _right - _up;
  _b2 =   _right - _up;
  _b3 =   _right + _up;
  _b4 = - _right + _up;

  // recreate our rewriters
  if (_vertex != nullptr) delete _vertex;
  if (_normal != nullptr) delete _normal;
  if (_uv != nullptr)     delete _uv;
  if (_color != nullptr)  delete _color;

  if (_vdata == nullptr) {
    generator(_budget);
  }

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

/**
 * Finish the drawing and clearing off the remaining vertexes.
 */
void MeshDrawer::end() {

  // clear the unused triangles at the end of the buffer
  for(int i = _clear_index ; i < _last_clear_index; i ++ ) {
    _vertex->add_data3(0,0,0);
    _vertex->add_data3(0,0,0);
    _vertex->add_data3(0,0,0);
  }
  // don't clear more then you have too
  _last_clear_index = _clear_index;

  // delete the re writers
  delete _vertex; _vertex = nullptr;
  delete _uv;     _uv     = nullptr;
  delete _normal; _normal = nullptr;
  delete _color;  _color  = nullptr;

}

/**
 * Draws a particle that is sort of like a bill board but has an extra
 * rotation component.  Frame contains u,v,u-size,v-size quadruple.
 */
void MeshDrawer::particle(const LVector3 &pos, const LVector4 &frame, PN_stdfloat size,
  const LVector4 &color, PN_stdfloat rotation) {

  rotation = rotation / 57.29578;

  LVector3 v1 = pos + _b1*size*sin(rotation) + _b2*size*cos(rotation);
  LVector3 v2 = pos + _b2*size*sin(rotation) + _b3*size*cos(rotation);
  LVector3 v3 = pos + _b3*size*sin(rotation) + _b4*size*cos(rotation);
  LVector3 v4 = pos + _b4*size*sin(rotation) + _b1*size*cos(rotation);

  PN_stdfloat u = frame.get_x();
  PN_stdfloat v = frame.get_y();
  PN_stdfloat us = frame.get_z();
  PN_stdfloat vs = frame.get_w();

  tri(
    v1, color, LVector2(u,v),
    v2, color, LVector2(u+us,v),
    v3, color, LVector2(u+us,v+vs));
  tri(
    v3, color, LVector2(u+us,v+vs),
    v4, color, LVector2(u,v+vs),
    v1, color, LVector2(u,v));
}

/**
 * Works just like particle but accepts 2 frames and a blend (from 0 to 1)
 * component between them Frame contains u,v,u-size,v-size quadruple.
 */
void MeshDrawer::blended_particle(const LVector3 &pos, const LVector4 &frame1,
  const LVector4 &frame2, PN_stdfloat blend, PN_stdfloat size, const LVector4 &color, PN_stdfloat rotation) {

  LVector4 c2 = color;
  PN_stdfloat original_w = c2.get_w();
  c2.set_w((1.f-blend)*original_w);
  particle(pos,frame1,size,c2,rotation);
  c2.set_w(blend*original_w);
  particle(pos,frame2,size,c2,rotation);

}

/**
 * Draws a billboard - particle with no rotation.  Billboards always face the
 * camera.  Frame contains u,v,u-size,v-size quadruple.
 */
void MeshDrawer::billboard(const LVector3 &pos, const LVector4 &frame, PN_stdfloat size,
  const LVector4 &_color) {

  LVector3 v1 = pos + _b1*size;
  LVector3 v2 = pos + _b2*size;
  LVector3 v3 = pos + _b3*size;
  LVector3 v4 = pos + _b4*size;

  PN_stdfloat u = frame.get_x();
  PN_stdfloat v = frame.get_y();
  PN_stdfloat us = frame.get_z();
  PN_stdfloat vs = frame.get_w();

  tri(
    v1, _color, LVector2(u,v),
    v2, _color, LVector2(u+us,v),
    v3, _color, LVector2(u+us,v+vs));
  tri(
    v3, _color, LVector2(u+us,v+vs),
    v4, _color, LVector2(u,v+vs),
    v1, _color, LVector2(u,v));
}


/**
 * Draws a segment a line with a thickness.  That has billboarding effect.
 * Frame contains u,v,u-size,v-size quadruple.
 */
void MeshDrawer::segment(const LVector3 &start, const LVector3 &stop, const LVector4 &frame,
                         PN_stdfloat thickness, const LVector4 &color) {
  link_segment(start, frame, thickness, color);
  link_segment(stop, frame, thickness, color);
  link_segment_end(frame, color);
}
/**
 * Draws a segment a line with a thickness.  This segment does not use the
 * bill boarding behavior and instead draws 2 planes in a cross.  Stars at
 * start and ends at stop.  Frame contains u,v,u-size,v-size quadruple.
 */
void MeshDrawer::cross_segment(const LVector3 &start, const LVector3 &stop, const LVector4 &frame,
                               PN_stdfloat thickness, const LVector4 &color) {

  PN_stdfloat u = frame.get_x();
  PN_stdfloat v = frame.get_y();
  PN_stdfloat us = frame.get_z();
  PN_stdfloat vs = frame.get_w();

  LVector3 v1 = start - _up*thickness;
  LVector3 v2 = stop - _up*thickness;
  LVector3 v3 = stop + _up*thickness;
  LVector3 v4 = start + _up*thickness;

  tri(v1, color, LVector2(u,v),
      v2, color, LVector2(u+us,v),
      v3, color, LVector2(u+us,v+vs));
  tri(v3, color, LVector2(u+us,v+vs),
      v4, color, LVector2(u,v+vs),
      v1, color, LVector2(u,v));

  v1 = start - _right*thickness;
  v2 = stop - _right*thickness;
  v3 = stop + _right*thickness;
  v4 = start + _right*thickness;

  tri(v1, color, LVector2(u,v),
      v2, color, LVector2(u+us,v),
      v3, color, LVector2(u+us,v+vs));
  tri(v3, color, LVector2(u+us,v+vs),
      v4, color, LVector2(u,v+vs),
      v1, color, LVector2(u,v));

}




/**
 * Draws a segment a line with different thickness and color on both sides.
 * Stars at start and ends at stop.  Frame contains u,v,u-size,v-size
 * quadruple.
 */
void MeshDrawer::uneven_segment(const LVector3 &start, const LVector3 &stop,
  const LVector4 &frame, PN_stdfloat thickness_start, const LVector4 &color_start,
  PN_stdfloat thickness_stop, const LVector4 &color_stop) {

  PN_stdfloat u = frame.get_x();
  PN_stdfloat v = frame.get_y();
  PN_stdfloat us = frame.get_z();
  PN_stdfloat vs = frame.get_w();

  LVector3 v1 = start - _up*thickness_start;
  LVector3 v2 = stop - _up*thickness_stop;
  LVector3 v3 = stop + _up*thickness_stop;
  LVector3 v4 = start + _up*thickness_start;

  tri(v1, color_start, LVector2(u,v),
      v2, color_stop, LVector2(u+us,v),
      v3, color_stop, LVector2(u+us,v+vs));
  tri(v3, color_stop, LVector2(u+us,v+vs),
      v4, color_start, LVector2(u,v+vs),
      v1, color_start, LVector2(u,v));

  v1 = start - _right*thickness_start;
  v2 = stop - _right*thickness_stop;
  v3 = stop + _right*thickness_stop;
  v4 = start + _right*thickness_start;

  tri(v1, color_start, LVector2(u,v),
      v2, color_stop, LVector2(u+us,v),
      v3, color_stop, LVector2(u+us,v+vs));
  tri(v3, color_stop, LVector2(u+us,v+vs),
      v4, color_start, LVector2(u,v+vs),
      v1, color_start, LVector2(u,v));
}

/**
 * Draws number of particles in a sphere like emitter.  Frame contains
 * u,v,u-size,v-size quadruple.
 */
void MeshDrawer::explosion(
  const LVector3 &pos, const LVector4 &frame, PN_stdfloat size, const LVector4 &_color,
  int seed, int number, PN_stdfloat distance) {
  srand(seed);
  LVector3 relative_pos;
  for(int i = 0; i < number; i++) {
    relative_pos = LVector3(randFloat()-.5f,randFloat()-.5f,randFloat()-.5f);
    relative_pos.normalize();
    relative_pos *= randFloat()*distance;
    particle(relative_pos+pos,frame,size,_color,randFloat()*360.0f);
  }
}

/**
 * Draws a number of particles in a big line with a shift dictated by the
 * offset.  Frame contains u,v,u-size,v-size quadruple.
 */
void MeshDrawer::stream(const LVector3 &start, const LVector3 &stop, const LVector4 &frame, PN_stdfloat size, const LVector4 &_color,
        int number, PN_stdfloat offset) {

  offset = offset-floor(offset);
  LVector3 relative_pos = stop;
  LVector3 vec = stop - start;
  PN_stdfloat distance = vec.length();
  vec.normalize();
  for(int i = 0; i < number; i++) {
    relative_pos = start + vec * ((i+offset)*(distance/PN_stdfloat(number)));
    billboard(relative_pos,frame,size,_color);
  }
}



/**
 * Draws the geometry that is inside this node path into the MeshDrawer
 * object.  This performs a similar functions as RigidBodyCombiner but for
 * very dynamic situations that share the same texture like physcal chunks of
 * explosions.  It can be a little slow
 */
void MeshDrawer::geometry(NodePath draw_node) {
  assert(_render.get_error_type() == NodePath::ET_ok);

  LVector4 color = LVector4(1,1,1,1);
  LVector3 vec[3];
  LVector2 uv[3];

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
      for (size_t k = 0; k < geom->get_num_primitives(); ++k) {
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
            prim_vertex_reader->set_row_unsafe(vidx);
            prim_uv_reader->set_row_unsafe(vidx);
            vec[indx_over] = _render.get_relative_point(
                            current_node_path,prim_vertex_reader->get_data3());
            uv[indx_over] = prim_uv_reader->get_data2();
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



/**
 * Stars or continues linked segment.  Control position, frame, thickness and
 * color with parameters.  Frame contains u,v,u-size,v-size quadruple.
 */
void MeshDrawer::
link_segment(const LVector3 &pos, const LVector4 &frame,
         PN_stdfloat thickness, const LVector4 &color) {
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

  LVector3 start = _last_pos;
  LVector3 stop = pos;

  LVector3 cam_start3d = _camera.get_relative_point(_render, start);
  LPoint2 cam_start2d = LVector2();
  LVector3 cam_stop3d = _camera.get_relative_point(_render, stop);
  LPoint2 cam_stop2d = LVector2();

  const Camera *camera;
  DCAST_INTO_V(camera, _camera.node());
  const Lens *lens = camera->get_lens();

  lens->project(cam_start3d, cam_start2d);
  lens->project(cam_stop3d, cam_stop2d);

  LVector2 dif =  cam_stop2d - cam_start2d;
  PN_stdfloat rotation = atan2(dif.get_x(),dif.get_y());

  LVector3 now_v1 = start + _b1*(PN_stdfloat)(thickness*sin(rotation)) + _b2*(PN_stdfloat)(thickness*cos(rotation));
  LVector3 now_v4 = start + _b4*(PN_stdfloat)(thickness*sin(rotation)) + _b1*(PN_stdfloat)(thickness*cos(rotation));
  LVector3 now_v2 = stop + _b2*(PN_stdfloat)(thickness*sin(rotation)) + _b3*(PN_stdfloat)(thickness*cos(rotation));
  LVector3 now_v3 = stop + _b3*(PN_stdfloat)(thickness*sin(rotation)) + _b4*(PN_stdfloat)(thickness*cos(rotation));

  // mark the segment we going to draw we need to draw it when we know what
  // the next segment looks like because it can bend it a little
  if(_at_start==1) {
    _last_v1 = now_v1;
    _last_v2 = now_v2;
    _last_v3 = now_v3;
    _last_v4 = now_v4;
    _at_start = 2;
    return;
  }

  // draw the last segment a little bent
  LVector3 v1 = _last_v1;
  LVector3 v2 = (_last_v2+now_v1)/2.0f;
  LVector3 v3 = (_last_v3+now_v4)/2.0f;
  LVector3 v4 = _last_v4;

  // compute this frame
  PN_stdfloat u = frame.get_x();
  PN_stdfloat v = frame.get_y();
  PN_stdfloat us = frame.get_z();
  PN_stdfloat vs = frame.get_w();

  tri(v1, _last_color, LVector2(u,v),
      v2, color, LVector2(u+us,v),
      v3, color, LVector2(u+us,v+vs));
  tri(v3, color, LVector2(u+us,v+vs),
      v4, _last_color, LVector2(u,v+vs),
      v1, _last_color, LVector2(u,v));

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

/**
 * Finish drawing linked segments, needs at least two calls to link_segment
 * before it can end the linked segment.  Frame contains u,v,u-size,v-size
 * quadruple.
 */
void MeshDrawer::link_segment_end(const LVector4 &frame, const LVector4 &color)
{
  PN_stdfloat u = frame.get_x();
  PN_stdfloat v = frame.get_y();
  PN_stdfloat us = frame.get_z();
  PN_stdfloat vs = frame.get_w();

  tri(_last_v1, _last_color, LVector2(u,v),
      _last_v2, color, LVector2(u+us,v),
      _last_v3, color, LVector2(u+us,v+vs));
  tri(_last_v3, color, LVector2(u+us,v+vs),
      _last_v4, _last_color, LVector2(u,v+vs),
      _last_v1, _last_color, LVector2(u,v));

  _at_start = 0;
}
