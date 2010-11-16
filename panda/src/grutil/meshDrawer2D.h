// Filename: meshDrawer2D.h
// Created by:  treeform (6Jan10)
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

#ifndef MESHDRAWER2D_H
#define MESHDRAWER2D_H

#include "pandabase.h"
#include "luse.h"
#include "pandaNode.h"
#include "pointerTo.h"
#include "lpoint2.h"
#include "lvecBase2.h"
#include "pnmImage.h"
#include "nodePath.h"
#include "texture.h"
#include "geomVertexFormat.h"
#include "geomVertexArrayFormat.h"
#include "geomVertexData.h"
#include "geomVertexWriter.h"
#include "geomVertexRewriter.h"
#include "boundingVolume.h"

#include "nodePathCollection.h"

#include "geomTristrips.h"
#include "geomTriangles.h"
#include "geom.h"
#include "geomNode.h"
#include "nodePath.h"

////////////////////////////////////////////////////////////////////
//       Class : MeshDrawer2D
// Description : This class allows the drawing of 2d objects -
//               mainly based on quads and rectangles.
//               Allows clipping and serverl high level UI theme
//               functions.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL MeshDrawer2D : public TypedObject {
PUBLISHED:
  INLINE MeshDrawer2D();
  INLINE ~MeshDrawer2D();

  INLINE void set_budget(int budget);
  INLINE int get_budget();
  INLINE NodePath get_root();

  INLINE void quad_raw(
    LVector3f v1, LVector4f c1, LVector2f uv1,
    LVector3f v2, LVector4f c2, LVector2f uv2,
    LVector3f v3, LVector4f c3, LVector2f uv3,
    LVector3f v4, LVector4f c4, LVector2f uv4);

  INLINE void rectangle_raw(
    float x, float y, float w, float h,
    float u, float v, float us, float vs,
    LVector4f color);

  INLINE void set_clip(float x, float y, float w, float h);

  INLINE void rectangle(
    float x, float y, float w, float h,
    float u, float v, float us, float vs,
    LVector4f color);

  void rectangle_border(
      float x, float y, float w, float h,
      float r, float t, float l, float b,
      float tr, float tt, float tl, float tb,
      float u, float v, float us, float vs,
      LVector4f color);

  void rectangle_border_tiled(
      float x, float y, float w, float h,
      float r, float t, float l, float b,
      float tr, float tt, float tl, float tb,
      float u, float v, float us, float vs,
      LVector4f color);

  void rectangle_tiled(
    float x, float y, float w, float h,
    float u, float v, float us, float vs,
    LVector4f color);

  void begin();
  void end();

private:

  // use vars
  NodePath _root;
  int _budget;

  // clip
  float _clip_x;
  float _clip_y;
  float _clip_w;
  float _clip_h;

  // store regeneration geoms & nodes
  PT(Geom) _geom;
  PT(GeomNode) _geomnode;
  PT(GeomVertexData) _vdata;
  PT(GeomTriangles) _prim;
  CPT(GeomPrimitive) _dprim;

  // writers
  GeomVertexRewriter *_vertex;
  GeomVertexRewriter *_uv;
  GeomVertexRewriter *_color;

  // clear indexes
  int _last_clear_index, _start_clear_index, _end_clear_index, _clear_index;

  // bounding volume
  PT(BoundingVolume) _bv;

  // private create all the needed geoms
  void generator(int budget);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedObject::init_type();
    register_type(_type_handle, "MeshDrawer2D",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "meshDrawer2D.I"

#endif /*MESHDRAWER2D_H*/
