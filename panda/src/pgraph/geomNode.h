// Filename: geomNode.h
// Created by:  drose (22Feb02)
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

#ifndef GEOMNODE_H
#define GEOMNODE_H

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
class EXPCL_PANDA GeomNode : public PandaNode {
PUBLISHED:
  GeomNode(const string &name);

protected:
  GeomNode(const GeomNode &copy);
public:
  virtual ~GeomNode();
  virtual PandaNode *make_copy() const;
  virtual void apply_attribs_to_vertices(const AccumulatedAttribs &attribs,
                                         int attrib_types,
                                         GeomTransformer &transformer);
  virtual void xform(const LMatrix4f &mat);
  virtual PandaNode *combine_with(PandaNode *other); 
  virtual CPT(TransformState)
    calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point,
                      bool &found_any,
                      const TransformState *transform) const;
  virtual CollideMask get_legal_collide_mask() const;

PUBLISHED:
  INLINE int get_num_geoms() const;
  INLINE const Geom *get_geom(int n) const;
  INLINE Geom *get_unique_geom(int n);
  INLINE const RenderState *get_geom_state(int n) const;
  INLINE void set_geom_state(int n, const RenderState *state);

  int add_geom(Geom *geom, const RenderState *state = RenderState::make_empty());
  void add_geoms_from(const GeomNode *other);
  INLINE void remove_geom(int n);
  INLINE void remove_all_geoms();

  void write_geoms(ostream &out, int indent_level) const;
  void write_verbose(ostream &out, int indent_level) const;

  INLINE static CollideMask get_default_collide_mask();

public:
  virtual void output(ostream &out) const;

  virtual bool is_geom_node() const;

protected:
  virtual BoundingVolume *recompute_internal_bound();

public:
  // This must be declared public so that VC6 will allow the nested
  // CData class to access it.
  class GeomEntry {
  public:
    INLINE GeomEntry(Geom *geom, const RenderState *state);
    PT(Geom) _geom;
    CPT(RenderState) _state;
  };

private:
  typedef pvector<GeomEntry> Geoms;
  typedef pmap<const InternalName *, int> NameCount;

  INLINE void count_name(NameCount &name_count, const InternalName *name);
  INLINE int get_name_count(const NameCount &name_count, const InternalName *name);

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA CData : public CycleData {
  public:
    INLINE CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);

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
    register_type(_type_handle, "GeomNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PandaNode::Children;
  friend class GeomTransformer;
};

#include "geomNode.I"

#endif
