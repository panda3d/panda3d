// Filename: meshDrawer.h
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

#ifndef MESHDRAWER_H
#define MESHDRAWER_H

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
//       Class : MeshDrawer
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_GRUTIL MeshDrawer : public TypedObject {
PUBLISHED:
  INLINE MeshDrawer();
  INLINE ~MeshDrawer();

  INLINE void set_plate_size(int plate_size);
  INLINE int get_plate_size();
  INLINE void set_budget(int budget);
  INLINE int get_budget();

  INLINE NodePath get_root();

  void begin(NodePath camera, NodePath render);
  INLINE void tri(LVector3f v1, LVector4f c1, LVector2f uv1,
                  LVector3f v2, LVector4f c2, LVector2f uv2,
                  LVector3f v3, LVector4f c3, LVector2f uv3);
  void particle(LVector3f pos, int frame, float size, LVector4f color, float rotation);
  void blended_particle(LVector3f pos, int frame1, int frame2,
           float blend, float size, LVector4f color, float rotation);
  void billboard(LVector3f pos, int frame, float size, LVector4f color);
  void segment(LVector3f start, LVector3f stop, int frame, float thickness, LVector4f color);
  void cross_segment(LVector3f start, LVector3f stop, int frame, float thickness, LVector4f color);
  void uneven_segment(LVector3f start, LVector3f stop,
  		int frame, int multi_frame,
  		float thickness_start, LVector4f color_start,
  		float thickness_stop, LVector4f color_stop);

  void link_segment(LVector3f pos, int frame, float thickness, LVector4f color);
  void link_segment_end(int frame, LVector4f color);

  void explosion(LVector3f pos, int frame, float size, LVector4f color,
           int seed, int number, float distance);
  void stream(LVector3f start, LVector3f stop, int frame, float size, LVector4f color,
        int number, float offset);
  void geometry(NodePath node);
  void end();

private:

  // use vars
  NodePath _root;
  NodePath _camera, _render;
  int _plate_size;
  float _frame_size;
  int _budget;

  // store regeneration geoms & nodes
  PT(Geom) _geom;
  PT(GeomNode) _geomnode;
  PT(GeomVertexData) _vdata;
  PT(GeomTriangles) _prim;
  CPT(GeomPrimitive) _dprim;

  // writers
  GeomVertexRewriter *_vertex;
  GeomVertexRewriter *_normal;
  GeomVertexRewriter *_uv;
  GeomVertexRewriter *_color;

  // billboard vectors
  LVector4f _colorv;
  LVector3f _normalv;
  LVector3f _eyePos;
  LVector3f _b1, _b2, _b3, _b4;
  LVector3f _up, _right;

  // clear indexes
  int _last_clear_index, _start_clear_index, _end_clear_index, _clear_index;

  // used for curves
  int _at_start;
  LVector3f _last_v1,_last_v2,_last_v3,_last_v4,_last_pos;
  float _last_thickness;
  LVector4f _last_color;

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
    register_type(_type_handle, "MeshDrawer",
                  TypedObject::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

};

#include "meshDrawer.I"

#endif /*MESHDRAWER_H*/
