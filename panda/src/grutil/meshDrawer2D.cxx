/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file meshDrawer2D.cxx
 * @author treeform
 * @date 2008-12-19
 */

#include "meshDrawer2D.h"

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

TypeHandle MeshDrawer2D::_type_handle;


#define RANDF ((PN_stdfloat) rand() / (PN_stdfloat) 0x7fffffff)


/**
 * Creates a system with a given budget.
 */
void MeshDrawer2D::generator(int budget) {
  // create enough triangles for budget:
  _vdata = new GeomVertexData(_root.get_name(), GeomVertexFormat::get_v3c4t2(), Geom::UH_static);//UH_dynamic);
  GeomVertexWriter *tvertex = new GeomVertexWriter(_vdata, "vertex");
  GeomVertexWriter *tuv = new GeomVertexWriter(_vdata, "texcoord");
  GeomVertexWriter *tcolor = new GeomVertexWriter(_vdata, "color");
  _prim = new GeomTriangles(Geom::UH_static);

  // iterate and fill _up a geom with random data so that it will not be
  // optimized out by panda3d system
  for(int i = 0; i < budget; i++) {
    for( int vert = 0; vert < 4; vert++) {

      LVector3 vec3 = LVector3(RANDF+10000,RANDF,RANDF);
      LVector4 vec4 = LVector4(RANDF,RANDF,RANDF,0);
      LVector2 vec2 = LVector2(RANDF,RANDF);

      tvertex->add_data3(vec3);
      tcolor->add_data4(vec4);
      tuv->add_data2(vec2);

    }

    _prim->add_vertices(i*4+0, i*4+1, i*4+2);
    _prim->close_primitive();

    _prim->add_vertices(i*4+1, i*4+2, i*4+3);
    _prim->close_primitive();

  }
  // create our node and attach it to this node path
  _geom = new Geom(_vdata);
  _geom->add_primitive(_prim);
  _geomnode = new GeomNode("__MeshDrawer_GeomNode");
  _geomnode->add_geom(_geom);
  _root.attach_new_node(_geomnode);
  _last_clear_index = budget;

  delete tvertex;
  delete tuv;
  delete tcolor;
}

/**
 * Opens up the geom for drawing, don't forget to call MeshDrawer2D::end()
 */
void MeshDrawer2D::begin() {

  // recreate our rewriters
  if (_vertex != nullptr) delete _vertex;
  if (_uv != nullptr)     delete _uv;
  if (_color != nullptr)  delete _color;

  _vertex = new GeomVertexRewriter(_vdata, "vertex");
  _uv = new GeomVertexRewriter(_vdata, "texcoord");
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
void MeshDrawer2D::end() {

  // clear the unused triangles at the end of the buffer
  for(int i = _clear_index ; i < _last_clear_index; i ++ ) {
    _vertex->add_data3(0,0,0);
    _vertex->add_data3(0,0,0);
    _vertex->add_data3(0,0,0);
    _vertex->add_data3(0,0,0);
  }
  // don't clear more then you have too
  _last_clear_index = _clear_index;

  // delete the re writers
  delete _vertex; _vertex = nullptr;
  delete _uv;     _uv     = nullptr;
  delete _color;  _color  = nullptr;

}



/**
 * Draws a tiled rectangle, size of tiles is in us and vs
 */
void MeshDrawer2D::
rectangle_tiled(PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h,
     PN_stdfloat u, PN_stdfloat v, PN_stdfloat us, PN_stdfloat vs,
     const LVector4 &color
) {

  PN_stdfloat x_fit = w/us;
  PN_stdfloat y_fit = h/vs;
  PN_stdfloat x_pos = x;

  while (x_fit > 0){
    PN_stdfloat y_pos = y;
    y_fit = h/vs;
    while (y_fit > 0){

      PN_stdfloat fixed_us = us;
      PN_stdfloat fixed_vs = vs;

      // we are cuttin in the middle of a tile x direction
      if (x_fit < 1){
        fixed_us = w;
        while (fixed_us > us){
          fixed_us -= us;
        }
      }

      // we are cuttin in the middel of a tile y directon
      if (y_fit < 1){
        fixed_vs = h;
        while (fixed_vs > vs){
          fixed_vs -= vs;
        }
      }

      rectangle(x_pos,y_pos,fixed_us,fixed_vs,u,v,fixed_us,fixed_vs,color);

      y_pos += vs;
      y_fit -= 1;
    }
    x_pos += us;
    x_fit -= 1;
  }


}


/**
 * Draws a 2d rectangle, with borders and corders, taken from the surrounding
 * texture
 */
void MeshDrawer2D::
rectangle_border(
    PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h,
    PN_stdfloat r, PN_stdfloat t, PN_stdfloat l, PN_stdfloat b,
    PN_stdfloat tr, PN_stdfloat tt, PN_stdfloat tl, PN_stdfloat tb,
    PN_stdfloat u, PN_stdfloat v, PN_stdfloat us, PN_stdfloat vs,
    const LVector4 &color){

    rectangle(x,y,w,h,u,v,us,vs,color); // center

    // --------------   -----------------  ------
    rectangle(x,   y+h, w, t,  u,  v+vs, us,  tt, color); // N
    rectangle(x,   y-b, w, b,  u,  v-tb, us,  tb, color); // S


    rectangle(x-l, y,   l, h,  u-tl, v,    tl,  vs, color); // W
    rectangle(x+w, y,   r, h,  r,    v,    tr,  vs, color); // E

/*
    rectangle(x-l, y+h, l, t,  u-tl, v,    tl,  tt, color); // NW
    rectangle(x-l, y-b, l, b,  u-tl, v-tb, tl,  tb, color); // SW
    rectangle(x+w, y+h, r, t,  u,    v,    tr,  tt, color); // NE
    rectangle(x+w, y-b, r, b,  u,    v-tb, tr,  tb, color); // SE
*/
}

/**
 * Draws a 2d rectangle, with borders and corders, taken from the surrounding
 * texture
 */
void MeshDrawer2D::
rectangle_border_tiled(
    PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h,
    PN_stdfloat r, PN_stdfloat t, PN_stdfloat l, PN_stdfloat b,
    PN_stdfloat tr, PN_stdfloat tt, PN_stdfloat tl, PN_stdfloat tb,
    PN_stdfloat u, PN_stdfloat v, PN_stdfloat us, PN_stdfloat vs,
    const LVector4 &color){

    rectangle_tiled(x,y,w,h,u,v,us,vs,color); // center

    rectangle_tiled(x,   y+h, w, t,  u,   v+t, us, t,  color); // N
    rectangle_tiled(x,   y-b, w, b,  u,   v-b, us, b,  color); // S
    rectangle_tiled(x-l, y,   l, h,  u-l, v,   l,  vs, color); // W
    rectangle_tiled(x+w, y,   r, h,  r,   v,   r,  vs, color); // E

    rectangle_tiled(x-l, y+h, l, t,  u-l, v,   l,  t, color); // NW
    rectangle_tiled(x-l, y-b, l, b,  u-l, v-b, l,  b, color); // SW
    rectangle_tiled(x+w, y+h, r, t,  u,   v,   r,  t, color); // NE
    rectangle_tiled(x+w, y-b, r, b,  u,   v-b, r,  b, color); // SE
}
