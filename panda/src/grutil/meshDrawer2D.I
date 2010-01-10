// Filename: meshDrawer.I
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

#include "lpoint2.h"

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer2D::Constructor
//       Access: Published
//  Description: Creates the MeshDrawer2D low level system.
////////////////////////////////////////////////////////////////////
INLINE MeshDrawer2D::
MeshDrawer2D() {
  _root = NodePath("MeshDrawer");  
  _bv = NULL;
  _vertex = NULL;
  _uv = NULL;
  _color = NULL;
  _budget = 5000;
  
  _clip_x = -1000000;
  _clip_y = -1000000;
  _clip_w = 1000000;
  _clip_h = 1000000;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer2D::Destructor
//       Access: Published
//  Description: Destroys the MeshDrawer2D low level system.
////////////////////////////////////////////////////////////////////
INLINE MeshDrawer2D::
~MeshDrawer2D() {
  _root.remove_node();
  if (_vertex != NULL) delete _vertex;
  if (_uv != NULL)     delete _uv;
  if (_color != NULL)  delete _color;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer2D::get_root
//       Access: Published
//  Description: Returns the root NodePath.
////////////////////////////////////////////////////////////////////
INLINE NodePath MeshDrawer2D::
get_root() {
  return _root;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer2D::set_budget
//       Access: Published
//  Description: Sets the total triangle budget of the drawer.
////////////////////////////////////////////////////////////////////
INLINE void MeshDrawer2D::
set_budget(int total_budget) {
  _budget = total_budget;
  generator(_budget);
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer2D::get_budget()
//       Access: Published
//  Description: Gets the total triangle budget of the drawer
////////////////////////////////////////////////////////////////////
INLINE int MeshDrawer2D::
get_budget() {
  return _budget;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer2D::set_budget
//       Access: Published
//  Description: Sets clipping rectangle
////////////////////////////////////////////////////////////////////
INLINE void MeshDrawer2D::
set_clip(float x, float y, float w, float h) {
  _clip_x = x;
  _clip_y = y;
  _clip_w = w;
  _clip_h = h;
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer2D::quad
//       Access: Published
//  Description: Draws a 2d rectangle.
//               Ignores the cliping rectangle
////////////////////////////////////////////////////////////////////
INLINE void MeshDrawer2D::
quad_raw(LVector3f v1, LVector4f c1, LVector2f uv1,
     LVector3f v2, LVector4f c2, LVector2f uv2,
     LVector3f v3, LVector4f c3, LVector2f uv3,
     LVector3f v4, LVector4f c4, LVector2f uv4
) {

  if( _clear_index > _end_clear_index) return;

  _vertex->add_data3f(v1);
  _color->add_data4f(c1);
  _uv->add_data2f(uv1);

  _vertex->add_data3f(v2);
  _color->add_data4f(c2);
  _uv->add_data2f(uv2);

  _vertex->add_data3f(v3);
  _color->add_data4f(c3);
  _uv->add_data2f(uv3);

  _vertex->add_data3f(v4);
  _color->add_data4f(c4);
  _uv->add_data2f(uv4);
  
  _clear_index += 1;
}


INLINE void MeshDrawer2D::
rectangle_raw(float x, float y, float w, float h, 
     float u, float v, float us, float vs, 
     LVector4f color
) {
    
  quad_raw( 
    LVector3f(x,   0, y),   color, LVector2f(u   , v),
    LVector3f(x,   0, y+h), color, LVector2f(u   , v+vs),
    LVector3f(x+w, 0, y),   color, LVector2f(u+us, v),
    LVector3f(x+w, 0, y+h), color, LVector2f(u+us, v+vs)
    );
}

////////////////////////////////////////////////////////////////////
//     Function: MeshDrawer2D::quad
//       Access: Published
//  Description: Draws a 2d rectangle, that can be cliped
////////////////////////////////////////////////////////////////////
INLINE void MeshDrawer2D::
rectangle(float x, float y, float w, float h, 
     float u, float v, float us, float vs, 
     LVector4f color
) {
    
  if( w == 0 &&  h == 0 ) return; // no size return 
  if (x > _clip_x+_clip_w) return;  // we are left of the clip
  if (y > _clip_y+_clip_h) return;  // we are above of the clip
  if (x+w < _clip_x) return; // we are right of the clip
  if (y+h < _clip_y) return; // we are bellow clip 
    
  // the rectange fits but it might need to be cliped
  
  float x_uv_ratio = us/w;
  float y_uv_ratio = vs/h;
  float dt = 0;
      
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

