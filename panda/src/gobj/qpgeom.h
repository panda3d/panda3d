// Filename: qpgeom.h
// Created by:  drose (06Mar05)
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

#ifndef qpGEOM_H
#define qpGEOM_H

#include "pandabase.h"
#include "typedWritableReferenceCount.h"
#include "boundedObject.h"
#include "cycleData.h"
#include "cycleDataReader.h"
#include "cycleDataWriter.h"
#include "pipelineCycler.h"
#include "qpgeomVertexData.h"
#include "qpgeomPrimitive.h"
#include "pointerTo.h"
#include "geom.h"

////////////////////////////////////////////////////////////////////
//       Class : qpGeom
// Description : A container for geometry primitives.  This class
//               associates one or more GeomPrimitive objects with a
//               table of vertices defined by a GeomVertexData object.
//               All of the primitives stored in a particular Geom are
//               drawn from the same set of vertices (each primitive
//               uses a subset of all of the vertices in the table),
//               and all of them must be rendered at the same time, in
//               the same graphics state.
//
//               This is part of the experimental Geom rewrite.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeom /* : public TypedWritableReferenceCount, public BoundedObject */
// We temporarily inherit from Geom, merely so we can store this
// pointer where a Geom should go, while we have both implementations
// in the codebase.  We pick up some additional cruft from Geom that
// we're not really using.
  : public Geom
{
PUBLISHED:
  qpGeom();
  qpGeom(const qpGeom &copy);
  void operator = (const qpGeom &copy);
  virtual ~qpGeom();

  // Temporary.
  virtual Geom *make_copy() const;

  INLINE CPT(qpGeomVertexData) get_vertex_data() const;
  PT(qpGeomVertexData) modify_vertex_data();
  void set_vertex_data(PT(qpGeomVertexData) data);

  INLINE int get_num_primitives() const;
  INLINE const qpGeomPrimitive *get_primitive(int i) const;
  INLINE qpGeomPrimitive *modify_primitive(int i);
  INLINE void set_primitive(int i, qpGeomPrimitive *primitive);
  void add_primitive(qpGeomPrimitive *primitive);
  void remove_primitive(int i);
  void clear_primitives();

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

public:
  void draw(GraphicsStateGuardianBase *gsg) const;

protected:
  virtual BoundingVolume *recompute_bound();

private:
  typedef pvector<PT(qpGeomPrimitive) > Primitives;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

    PT(qpGeomVertexData) _data;
    Primitives _primitives;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;

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
    /*TypedWritableReferenceCount::init_type();
      BoundedObject::init_type();*/
    Geom::init_type();
    register_type(_type_handle, "qpGeom",
                  Geom::get_class_type()
                  /*TypedWritableReferenceCount::get_class_type(),
                    BoundedObject::get_class_type()*/);
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const qpGeom &obj);

#include "qpgeom.I"

#endif
