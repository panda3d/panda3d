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
    const LVector3 &v1, const LVector4 &c1, const LVector2 &uv1,
    const LVector3 &v2, const LVector4 &c2, const LVector2 &uv2,
    const LVector3 &v3, const LVector4 &c3, const LVector2 &uv3,
    const LVector3 &v4, const LVector4 &c4, const LVector2 &uv4);

  INLINE void rectangle_raw(
    PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h,
    PN_stdfloat u, PN_stdfloat v, PN_stdfloat us, PN_stdfloat vs,
    const LVector4 &color);

  INLINE void set_clip(PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h);

  INLINE void rectangle(
    PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h,
    PN_stdfloat u, PN_stdfloat v, PN_stdfloat us, PN_stdfloat vs,
    const LVector4 &color);

  void rectangle_border(
      PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h,
      PN_stdfloat r, PN_stdfloat t, PN_stdfloat l, PN_stdfloat b,
      PN_stdfloat tr, PN_stdfloat tt, PN_stdfloat tl, PN_stdfloat tb,
      PN_stdfloat u, PN_stdfloat v, PN_stdfloat us, PN_stdfloat vs,
      const LVector4 &color);

  void rectangle_border_tiled(
      PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h,
      PN_stdfloat r, PN_stdfloat t, PN_stdfloat l, PN_stdfloat b,
      PN_stdfloat tr, PN_stdfloat tt, PN_stdfloat tl, PN_stdfloat tb,
      PN_stdfloat u, PN_stdfloat v, PN_stdfloat us, PN_stdfloat vs,
      const LVector4 &color);

  void rectangle_tiled(
    PN_stdfloat x, PN_stdfloat y, PN_stdfloat w, PN_stdfloat h,
    PN_stdfloat u, PN_stdfloat v, PN_stdfloat us, PN_stdfloat vs,
    const LVector4 &color);

  void begin();
  void end();

private:

  // use vars
  NodePath _root;
  int _budget;

  // clip
  PN_stdfloat _clip_x;
  PN_stdfloat _clip_y;
  PN_stdfloat _clip_w;
  PN_stdfloat _clip_h;

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
