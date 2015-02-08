// Filename: cullableObject.h
// Created by:  drose (04Mar02)
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
#include "graphicsStateGuardianBase.h"
#include "sceneSetup.h"
#include "lightMutex.h"
#include "callbackObject.h"
#include "geomDrawCallbackData.h"

class CullTraverser;

////////////////////////////////////////////////////////////////////
//       Class : CullableObject
// Description : The smallest atom of cull.  This is normally just a
//               Geom and its associated state, but it also contain
//               a draw callback.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGRAPH CullableObject
#ifdef DO_MEMORY_USAGE
  : public ReferenceCount   // We inherit from ReferenceCount just to get the memory type tracking that MemoryUsage provides.
#endif  // DO_MEMORY_USAGE
{
public:
  INLINE CullableObject();
  INLINE CullableObject(const Geom *geom, const RenderState *state,
                        const TransformState *internal_transform);

  INLINE CullableObject(const CullableObject &copy);
  INLINE void operator = (const CullableObject &copy);

  bool munge_geom(GraphicsStateGuardianBase *gsg,
                  GeomMunger *munger, const CullTraverser *traverser,
                  bool force);
  INLINE void draw(GraphicsStateGuardianBase *gsg,
                   bool force, Thread *current_thread);

  INLINE bool request_resident() const;
  INLINE static void flush_level();

  INLINE void set_draw_callback(CallbackObject *draw_callback);

public:
  ALLOC_DELETED_CHAIN(CullableObject);

  void output(ostream &out) const;

public:
  CPT(Geom) _geom;
  PT(GeomMunger) _munger;
  CPT(GeomVertexData) _munged_data;
  CPT(RenderState) _state;
  CPT(TransformState) _internal_transform;
  PT(CallbackObject) _draw_callback;

private:
  bool munge_points_to_quads(const CullTraverser *traverser, bool force);

  static CPT(RenderState) get_flash_cpu_state();
  static CPT(RenderState) get_flash_hardware_state();

  INLINE void draw_inline(GraphicsStateGuardianBase *gsg,
                          bool force, Thread *current_thread);

private:
  // This class is used internally by munge_points_to_quads().
  class PointData {
  public:
    PN_stdfloat _dist;
  };
  class SortPoints {
  public:
    INLINE SortPoints(const PointData *array);
    INLINE bool operator ()(unsigned short a, unsigned short b) const;

    const PointData *_array;
  };

  // This is a cache of converted vertex formats.
  class SourceFormat {
  public:
    SourceFormat(const GeomVertexFormat *format, bool sprite_texcoord);
    INLINE bool operator < (const SourceFormat &other) const;

    CPT(GeomVertexFormat) _format;
    bool _sprite_texcoord;
    bool _retransform_sprites;
  };
  typedef pmap<SourceFormat, CPT(GeomVertexFormat) > FormatMap;
  static FormatMap _format_map;
  static LightMutex _format_lock;

  static PStatCollector _munge_geom_pcollector;
  static PStatCollector _munge_sprites_pcollector;
  static PStatCollector _munge_sprites_verts_pcollector;
  static PStatCollector _munge_sprites_prims_pcollector;
  static PStatCollector _sw_sprites_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
#ifdef DO_MEMORY_USAGE
    ReferenceCount::init_type();
    register_type(_type_handle, "CullableObject",
                  ReferenceCount::get_class_type());
#else
    register_type(_type_handle, "CullableObject");
#endif  // DO_MEMORY_USAGE
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
