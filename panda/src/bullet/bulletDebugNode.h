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

////////////////////////////////////////////////////////////////////
//       Class : BulletDebugNode
// Description : 
////////////////////////////////////////////////////////////////////
class EXPCL_PANDABULLET BulletDebugNode : public GeomNode {

PUBLISHED:
  BulletDebugNode(const char *name="debug");
  INLINE ~BulletDebugNode();

  virtual void draw_mask_changed();

  INLINE void show_wireframe(bool show);
  INLINE void show_constraints(bool show);
  INLINE void show_bounding_boxes(bool show);
  INLINE void show_normals(bool show);

public:
  virtual bool safe_to_flatten() const;
  virtual bool safe_to_transform() const;
  virtual bool safe_to_modify_transform() const;
  virtual bool safe_to_combine() const;
  virtual bool safe_to_combine_children() const;
  virtual bool safe_to_flatten_below() const;

private:
  void sync_b2p(btDynamicsWorld *world);

  struct Line {
    LVecBase3 _p0;
    LVecBase3 _p1;
    UnalignedLVecBase4 _color;
  };

  struct Triangle {
    LVecBase3 _p0;
    LVecBase3 _p1;
    LVecBase3 _p2;
    UnalignedLVecBase4 _color;
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

    bool _normals;
    int _mode;
  };

  DebugDraw _drawer;

  bool _wireframe;
  bool _constraints;
  bool _bounds;

  friend class BulletWorld;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomNode::init_type();
    register_type(_type_handle, "BulletDebugNode", 
                  GeomNode::get_class_type());
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

