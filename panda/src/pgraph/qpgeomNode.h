// Filename: qpgeomNode.h
// Created by:  drose (22Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef qpGEOMNODE_H
#define qpGEOMNODE_H

#include "pandabase.h"

#include "pandaNode.h"
#include "pointerToArray.h"
#include "geom.h"
#include "pipelineCycler.h"
#include "cycleData.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : GeomNode
// Description : A node that holds Geom objects, renderable pieces of
//               geometry.  This is the primary kind of leaf node in
//               the scene graph; almost all visible objects will be
//               contained in a GeomNode somewhere.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpGeomNode : public PandaNode {
PUBLISHED:
  qpGeomNode(const string &name);

public:
  qpGeomNode(const qpGeomNode &copy);
  void operator = (const qpGeomNode &copy);
  virtual ~qpGeomNode();

PUBLISHED:
  INLINE int get_num_geoms() const;
  INLINE Geom *get_geom(int n) const;
  INLINE const RenderState *get_geom_state(int n) const;
  INLINE void set_geom_state(int n, const RenderState *state);

  INLINE int add_geom(Geom *geom, const RenderState *state = RenderState::make_empty());
  INLINE void remove_geom(int n);
  INLINE void remove_all_geoms();

  void write_geoms(ostream &out, int indent_level) const;
  void write_verbose(ostream &out, int indent_level) const;

public:
  virtual void output(ostream &out) const;

  virtual bool is_geom_node() const;

protected:
  virtual BoundingVolume *recompute_internal_bound();

private:
  class GeomEntry {
  public:
    INLINE GeomEntry(Geom *geom, const RenderState *state);
    PT(Geom) _geom;
    CPT(RenderState) _state;
  };
  typedef pvector<GeomEntry> Geoms;

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const;

    Geoms _geoms;
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
    PandaNode::init_type();
    register_type(_type_handle, "qpGeomNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PandaNode::Children;
};

#include "qpgeomNode.I"

#endif
