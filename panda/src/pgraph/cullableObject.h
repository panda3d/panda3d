// Filename: cullableObject.h
// Created by:  drose (04Mar02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef CULLABLEOBJECT_H
#define CULLABLEOBJECT_H

#include "pandabase.h"

#include "geom.h"
#include "qpgeom.h"
#include "qpgeomVertexData.h"
#include "qpgeomMunger.h"
#include "renderState.h"
#include "transformState.h"
#include "pointerTo.h"
#include "referenceCount.h"
#include "geomNode.h"
#include "cullTraverserData.h"
#include "pStatCollector.h"

class CullTraverser;

////////////////////////////////////////////////////////////////////
//       Class : CullableObject
// Description : The smallest atom of cull.  This is normally just a
//               Geom and its associated state, but it also represent
//               a number of Geoms to be drawn together, with a number
//               of Geoms decalled onto them.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA CullableObject {
public:
  INLINE CullableObject(CullableObject *next = NULL);
  INLINE CullableObject(const CullTraverserData &data,
                        GeomNode *geom_node, int i,
                        CullableObject *next = NULL);
  INLINE CullableObject(const Geom *geom, const RenderState *state,
                        const TransformState *transform,
                        CullableObject *next = NULL);
    
  INLINE CullableObject(const CullableObject &copy);
  INLINE void operator = (const CullableObject &copy);

  INLINE bool has_decals() const;

  void munge_geom(GraphicsStateGuardianBase *gsg,
                  qpGeomMunger *munger, const CullTraverser *traverser);
  INLINE void draw(GraphicsStateGuardianBase *gsg);

public:
  ~CullableObject();

  // We will allocate and destroy hundreds or thousands of these a
  // frame during the normal course of rendering.  As an optimization,
  // then, we implement operator new and delete here to minimize this
  // overhead.
  INLINE void *operator new(size_t size);
  INLINE void operator delete(void *ptr);
  void output(ostream &out) const;

PUBLISHED:
  INLINE static int get_num_ever_allocated();

public:
  CPT(Geom) _geom;
  PT(qpGeomMunger) _munger;
  CPT(qpGeomVertexData) _munged_data;
  CPT(RenderState) _state;
  CPT(TransformState) _transform;
  CullableObject *_next;

private:
  void munge_points_to_quads(const CullTraverser *traverser);

private:
  // This class is used internally by munge_points_to_quads().
  class PointData {
  public:
    LPoint3f _eye;
    float _dist;
  };
  class SortPoints {
  public:
    INLINE SortPoints(const PointData *array);
    INLINE bool operator ()(unsigned short a, unsigned short b) const;

    const PointData *_array;
  };

  static CullableObject *_deleted_chain;
  static int _num_ever_allocated;

  static PStatCollector _munge_points_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "CullableObject");
  }

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const CullableObject &object) {
  object.output(out);
  return out;
}

#include "cullableObject.I"

#endif
