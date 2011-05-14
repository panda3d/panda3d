// Filename: bulletDebugNode.h
// Created by:  enn0x (23Jan10)
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

#ifndef __BULLET_DEBUG_NODE_H__
#define __BULLET_DEBUG_NODE_H__

#include "pandabase.h"

#include "bullet_includes.h"

#include "geomNode.h"
#include "geom.h"
#include "geomVertexData.h"
#include "geomLines.h"
#include "geomTriangles.h"

////////////////////////////////////////////////////////////////////
//       Class : BulletDebugNode
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletDebugNode : public GeomNode {

PUBLISHED:
  BulletDebugNode(const char *name="debug");
  INLINE ~BulletDebugNode();

  virtual void draw_mask_changed();

  INLINE void set_verbose(bool verbose);
  INLINE bool get_verbose() const;

public:
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_modify_transform() const;
  virtual bool safe_to_combine() const;
  virtual bool safe_to_combine_children() const;
  virtual bool safe_to_flatten_below() const;

private:
  void post_step(btDynamicsWorld *world);

  struct Line {
    LVecBase3f _p0;
    LVecBase3f _p1;
    Colorf _color;
  };

  struct Triangle {
    LVecBase3f _p0;
    LVecBase3f _p1;
    LVecBase3f _p2;
    Colorf _color;
  };

  class DebugDraw : public btIDebugDraw {

  public:
    DebugDraw() {};

    virtual void setDebugMode(int mode);
    virtual int getDebugMode() const;
    virtual void reportErrorWarning(const char *warning);

    virtual void drawLine(const btVector3 &from, const btVector3 &to,
      const btVector3 &color);
    virtual void drawContactPoint(const btVector3 &point, 
      const btVector3 &normal, btScalar distance, int lifetime, 
      const btVector3 &color);
    virtual void draw3dText(const btVector3 &location, const char *text);
    virtual void drawTriangle(const btVector3 &v0, const btVector3 &v1, 
      const btVector3 &v2, const btVector3 &color, btScalar);
    virtual void drawTriangle(const btVector3 &v0, const btVector3 &v1, 
      const btVector3 &v2, const btVector3 &n0, const btVector3 &n1, 
      const btVector3 &n2, const btVector3 &color, btScalar alpha);
    virtual void drawSphere(btScalar radius, const btTransform &transform, 
      const btVector3 &color);

  public:
    pvector<Line> _lines;
    pvector<Triangle> _triangles;

  private:
    int _mode;
  };

  DebugDraw _drawer;

  bool _verbose;

  PT(GeomVertexData) _vdata;
  PT(Geom) _geom_lines;
  PT(GeomLines) _prim_lines;
  PT(Geom) _geom_triangles;
  PT(GeomTriangles) _prim_triangles;

  friend class BulletWorld;

////////////////////////////////////////////////////////////////////
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "BulletDebugNode", 
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {
    init_type();
    return get_class_type();
  }

private:
  static TypeHandle _type_handle;
};

#include "bulletDebugNode.I"

#endif // __BULLET_DEBUG_NODE_H__

