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
#include "geomVertexData.h"
#include "geomMunger.h"
#include "renderState.h"
#include "transformState.h"
#include "pointerTo.h"
#include "referenceCount.h"
#include "geomNode.h"
#include "cullTraverserData.h"
#include "pStatCollector.h"
#include "deletedChain.h"

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
  INLINE CullableObject(const CullTraverser *trav,
                        const CullTraverserData &data,
                        const GeomNode::Geoms &geoms, int i,
                        CullableObject *next = NULL);
  INLINE CullableObject(const Geom *geom, const RenderState *state,
                        const TransformState *net_transform,
                        const TransformState *modelview_transform,
                        const GraphicsStateGuardianBase *gsg,
                        CullableObject *next = NULL);
    
  INLINE CullableObject(const CullableObject &copy);
  INLINE void operator = (const CullableObject &copy);

  INLINE bool has_decals() const;

  void munge_geom(GraphicsStateGuardianBase *gsg,
                  GeomMunger *munger, const CullTraverser *traverser);
  INLINE void draw(GraphicsStateGuardianBase *gsg,
                   Thread *current_thread);

public:
  ~CullableObject();
  ALLOC_DELETED_CHAIN(CullableObject);

  void output(ostream &out) const;

public:
  CPT(Geom) _geom;
  PT(GeomMunger) _munger;
  CPT(GeomVertexData) _munged_data;
  CPT(RenderState) _state;
  CPT(TransformState) _net_transform;
  CPT(TransformState) _modelview_transform;
  CPT(TransformState) _internal_transform;
  CullableObject *_next;

  // This flag is only used by certain CullBin types.  In particular,
  // it is used by CullBinOcclusionTest.
  bool _already_drawn;

private:
  void munge_points_to_quads(const CullTraverser *traverser);
  void munge_texcoord_light_vector(const CullTraverser *traverser);

  static CPT(RenderState) get_flash_cpu_state();
  static CPT(RenderState) get_flash_hardware_state();

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

  static PStatCollector _munge_points_pcollector;
  static PStatCollector _munge_light_vector_pcollector;

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
