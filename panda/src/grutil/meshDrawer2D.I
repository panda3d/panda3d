/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file meshDrawer2D.I
 * @author treeform
 * @date 2008-12-19
 */

#include "lpoint2.h"

/**
 * Creates the MeshDrawer2D low level system.
 */
INLINE MeshDrawer2D::
MeshDrawer2D() {
  _root = NodePath("MeshDrawer");
  _bv = nullptr;
  _vertex = nullptr;
  _uv = nullptr;
  _color = nullptr;
  _budget = 5000;

  _clip_x = -1000000;
  _clip_y = -1000000;
  _clip_w = 1000000;
  _clip_h = 1000000;
}

/**
 * Destroys the MeshDrawer2D low level system.
 */
INLINE MeshDrawer2D::
~MeshDrawer2D() {
  _root.remove_node();
  if (_vertex != nullptr) delete _vertex;
  if (_uv != nullptr)     delete _uv;
  if (_color != nullptr)  delete _color;
}

/**
 * Returns the root NodePath.
 */
INLINE NodePath MeshDrawer2D::
get_root() {
  return _root;
}

/**
 * Sets the total triangle budget of the drawer.
 */
INLINE void MeshDrawer2D::
set_budget(int total_budget) {
  _budget = total_budget;
  generator(_budget);
}

/**
 * Gets the total triangle budget of the drawer
 */
INLINE int MeshDrawer2D::
get_budget() {
  return _budget;
}

/**
 * Sets clipping rectangle
 */
INLINE void MeshDrawer2D::
set_clip(PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h) {
  _clip_x = x;
  _clip_y = y;
  _clip_w = w;
  _clip_h = h;
}

/**
 * Draws a 2d rectangle.  Ignores the cliping rectangle
 */
INLINE void MeshDrawer2D::
quad_raw(const LVector3 &v1, const LVector4 &c1, const LVector2 &uv1,
     const LVector3 &v2, const LVector4 &c2, const LVector2 &uv2,
     const LVector3 &v3, const LVector4 &c3, const LVector2 &uv3,
     const LVector3 &v4, const LVector4 &c4, const LVector2 &uv4
) {

  if( _clear_index > _end_clear_index) return;

  _vertex->add_data3(v1);
  _color->add_data4(c1);
  _uv->add_data2(uv1);

  _vertex->add_data3(v2);
  _color->add_data4(c2);
  _uv->add_data2(uv2);

  _vertex->add_data3(v3);
  _color->add_data4(c3);
  _uv->add_data2(uv3);

  _vertex->add_data3(v4);
  _color->add_data4(c4);
  _uv->add_data2(uv4);

  _clear_index += 1;
}


INLINE void MeshDrawer2D::
rectangle_raw(PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h,
     PN_stdfloat u, PN_stdfloat v, PN_stdfloat us, PN_stdfloat vs,
     const LVector4 &color
) {

  quad_raw(
    LVector3(x,   0, y),   color, LVector2(u   , v),
    LVector3(x,   0, y+h), color, LVector2(u   , v+vs),
    LVector3(x+w, 0, y),   color, LVector2(u+us, v),
    LVector3(x+w, 0, y+h), color, LVector2(u+us, v+vs)
    );
}

/**
 * Draws a 2d rectangle, that can be cliped
 */
INLINE void MeshDrawer2D::
rectangle(PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h,
     PN_stdfloat u, PN_stdfloat v, PN_stdfloat us, PN_stdfloat vs,
     const LVector4 &color
) {

  if( w == 0 &&  h == 0 ) return; // no size return
  if (x > _clip_x+_clip_w) return;  // we are left of the clip
  if (y > _clip_y+_clip_h) return;  // we are above of the clip
  if (x+w < _clip_x) return; // we are right of the clip
  if (y+h < _clip_y) return; // we are bellow clip

  // the rectange fits but it might need to be cliped

  PN_stdfloat x_uv_ratio = us/w;
  PN_stdfloat y_uv_ratio = vs/h;
  PN_stdfloat dt = 0;

  if (x < _clip_x){
    // clip right
    dt = _clip_x-x;
    x  += dt;
    w  -= dt;
    u  += dt*x_uv_ratio;
    us -= dt*x_uv_ratio;
  }

  if (y < _clip_y){
    // clip bottom
    dt = _clip_y-y;
    y  += dt;
    h  -= dt;
    v  += dt*y_uv_ratio;
    vs -= dt*y_uv_ratio;
  }

  if (x+w > _clip_x+_clip_w){
    // clip left
    dt = x+w - (_clip_x+_clip_w);
    w  -= dt;
    us -= dt*x_uv_ratio;
  }

  if (y+h > _clip_y+_clip_h){
    // clip top
    dt = y+h - (_clip_y+_clip_h);
    h  -= dt;
    vs -= dt*y_uv_ratio;
  }

  // we made it lets draw the quad
  rectangle_raw(x,y,w,h,u,v,us,vs,color);

}
