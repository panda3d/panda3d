// Filename: qpgeomMunger.h
// Created by:  drose (10Mar05)
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

#ifndef qpGEOMMUNGER_H
#define qpGEOMMUNGER_H

#include "pandabase.h"
#include "typedReferenceCount.h"
#include "qpgeomVertexFormat.h"
#include "qpgeomVertexData.h"
#include "indirectCompareTo.h"
#include "pStatCollector.h"
#include "pointerTo.h"
#include "pmap.h"
#include "pset.h"

class GraphicsStateGuardianBase;
class RenderState;
class qpGeom;

////////////////////////////////////////////////////////////////////
//       Class : qpGeomMunger
// Description : Objects of this class are used to convert vertex data
//               from a Geom into a format suitable for passing to the
//               rendering backend.  Typically, the rendering backend
//               will create a specialization of this class to handle
//               its particular needs (e.g. DXGeomMunger).  This class
//               is necessary because DirectX and OpenGL have somewhat
//               different requirements for vertex format.
//
//               This also performs runtime application of state
//               changes to the vertex data; for instance, by scaling
//               all of the color values in response to a
//               ColorScaleAttrib.
//
//               A GeomMunger must be registered before it can be
//               used, and once registered, the object is constant and
//               cannot be changed.  All registered GeomMungers that
//               perform the same operation will have the same
//               pointer.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomMunger : public TypedReferenceCount {
public:
  qpGeomMunger(const GraphicsStateGuardianBase *gsg, const RenderState *state);
  virtual ~qpGeomMunger();

  INLINE bool is_registered() const;
  INLINE static CPT(qpGeomMunger) register_munger(qpGeomMunger *munger);

  INLINE CPT(qpGeomVertexFormat) munge_format(const qpGeomVertexFormat *format) const;
  void remove_format(const qpGeomVertexFormat *format);

  INLINE CPT(qpGeomVertexData) munge_data(const qpGeomVertexData *data) const;
  void remove_data(const qpGeomVertexData *data);

  // Also see Geom::munge_geom() for the primary interface.

public:
  INLINE int compare_to(const qpGeomMunger &other) const;

protected:
  CPT(qpGeomVertexFormat) do_munge_format(const qpGeomVertexFormat *format);
  CPT(qpGeomVertexData) do_munge_data(const qpGeomVertexData *data);

  virtual CPT(qpGeomVertexFormat) munge_format_impl(const qpGeomVertexFormat *orig);
  virtual void munge_geom_impl(CPT(qpGeom) &geom, CPT(qpGeomVertexData) &data);
  virtual int compare_to_impl(const qpGeomMunger *other) const;

private:
  class Registry;
  INLINE static Registry *get_registry();
  static void make_registry();

  void do_register();
  void do_unregister();

  typedef pmap<const qpGeomVertexFormat *, const qpGeomVertexFormat *> Formats;
  Formats _formats;

  bool _is_registered;
  typedef pset<qpGeomMunger *, IndirectCompareTo<qpGeomMunger> > Mungers;
  class Registry {
  public:
    Registry();
    CPT(qpGeomMunger) register_munger(qpGeomMunger *munger);
    void unregister_munger(qpGeomMunger *munger);

    Mungers _mungers;
  };

  static Registry *_registry;

  static PStatCollector _munge_pcollector;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedReferenceCount::init_type();
    register_type(_type_handle, "qpGeomMunger",
                  TypedReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class qpGeom;
};

#include "qpgeomMunger.I"

#endif

