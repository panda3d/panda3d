/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file geomNode.h
 * @author drose
 * @date 2002-02-22
 */

#ifndef GEOMNODE_H
#define GEOMNODE_H

#include "pandabase.h"
#include "config_pgraph.h"
#include "pandaNode.h"
#include "pointerToArray.h"
#include "geom.h"
#include "pipelineCycler.h"
#include "cycleData.h"
#include "pvector.h"
#include "copyOnWritePointer.h"

class GraphicsStateGuardianBase;

/**
 * A node that holds Geom objects, renderable pieces of geometry.  This is the
 * primary kind of leaf node in the scene graph; almost all visible objects
 * will be contained in a GeomNode somewhere.
 */
class EXPCL_PANDA_PGRAPH GeomNode : public PandaNode {
PUBLISHED:
  explicit GeomNode(const std::string &name);

protected:
  GeomNode(const GeomNode &copy);
public:
  virtual ~GeomNode();
  virtual PandaNode *make_copy() const;
  virtual void apply_attribs_to_vertices(const AccumulatedAttribs &attribs,
                                         int attrib_types,
                                         GeomTransformer &transformer);
  virtual void xform(const LMatrix4 &mat);
  virtual PandaNode *combine_with(PandaNode *other);
  virtual CPT(TransformState)
    calc_tight_bounds(LPoint3 &min_point, LPoint3 &max_point,
                      bool &found_any,
                      const TransformState *transform,
                      Thread *current_thread) const;
  virtual bool is_renderable() const;
  virtual void add_for_draw(CullTraverser *trav, CullTraverserData &data);
  virtual CollideMask get_legal_collide_mask() const;

  virtual bool safe_to_flatten() const;
  virtual bool safe_to_combine() const;

  virtual void r_prepare_scene(GraphicsStateGuardianBase *gsg,
                               const RenderState *node_state,
                               GeomTransformer &transformer,
                               Thread *current_thread);

PUBLISHED:
  INLINE void set_preserved(bool value);
  INLINE bool get_preserved() const;

  INLINE int get_num_geoms() const;
  INLINE CPT(Geom) get_geom(int n) const;
  MAKE_SEQ(get_geoms, get_num_geoms, get_geom);
  INLINE PT(Geom) modify_geom(int n);
  MAKE_SEQ(modify_geoms, get_num_geoms, modify_geom);
  INLINE const RenderState *get_geom_state(int n) const;
  MAKE_SEQ(get_geom_states, get_num_geoms, get_geom_state);
  INLINE void set_geom_state(int n, const RenderState *state);

  void add_geom(Geom *geom, const RenderState *state = RenderState::make_empty());
  void add_geoms_from(const GeomNode *other);
  void set_geom(int n, Geom *geom);
  INLINE void remove_geom(int n);
  INLINE void remove_all_geoms();
  bool check_valid() const;

  void decompose();
  void unify(int max_indices, bool preserve_order);

  void write_geoms(std::ostream &out, int indent_level) const;
  void write_verbose(std::ostream &out, int indent_level) const;

  INLINE static CollideMask get_default_collide_mask();
  MAKE_PROPERTY(default_collide_mask, get_default_collide_mask);

public:
  virtual void output(std::ostream &out) const;

  virtual bool is_geom_node() const;

  void do_premunge(GraphicsStateGuardianBase *gsg,
                   const RenderState *node_state,
                   GeomTransformer &transformer);

protected:
  virtual void r_mark_geom_bounds_stale(Thread *current_thread);
  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

public:
  // This must be declared public so that VC6 will allow the nested CData
  // class to access it.
  class GeomEntry {
  public:
    INLINE GeomEntry(Geom *geom, const RenderState *state);
    COWPT(Geom) _geom;
    CPT(RenderState) _state;
  };

  typedef CopyOnWriteObj< pvector<GeomEntry> > GeomList;

private:

  bool _preserved;
  typedef pmap<const InternalName *, int> NameCount;

  INLINE void count_name(NameCount &name_count, const InternalName *name);
  INLINE int get_name_count(const NameCount &name_count, const InternalName *name);

  // This is the data that must be cycled between pipeline stages.
  class EXPCL_PANDA_PGRAPH CData : public CycleData {
  public:
    INLINE CData();
    CData(const CData &copy);
    virtual CycleData *make_copy() const;
    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return GeomNode::get_class_type();
    }

    INLINE CPT(GeomList) get_geoms() const;
    INLINE PT(GeomList) modify_geoms();
    INLINE void set_geoms(GeomList *geoms);

  private:
    COWPT(GeomList) _geoms;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataStageReader<CData> CDStageReader;
  typedef CycleDataLockedStageReader<CData> CDLockedStageReader;
  typedef CycleDataStageWriter<CData> CDStageWriter;

public:
  // This class is returned from get_geoms().  It is similar to
  // PandaNode::get_children(); use this to walk through the list of geoms
  // faster than walking through the geoms directly from the node.
  class EXPCL_PANDA_PGRAPH Geoms {
  public:
    INLINE Geoms();
    INLINE Geoms(const CData *cdata);
    INLINE Geoms(const Geoms &copy);
    INLINE Geoms(Geoms &&from) noexcept;

    INLINE void operator = (const Geoms &copy);
    INLINE void operator = (Geoms &&from) noexcept;

    INLINE int get_num_geoms() const;
    INLINE CPT(Geom) get_geom(int n) const;
    INLINE const RenderState *get_geom_state(int n) const;

  private:
    CPT(GeomList) _geoms;
  };

  INLINE Geoms get_geoms(Thread *current_thread = Thread::get_current_thread()) const;

  // This data is only needed when reading from a bam file.
  class BamAuxData : public BamReader::AuxData {
  public:
    // We just hold a pointer to the RenderState that may otherwise lose its
    // pointers before it can finalize.
    CPT(RenderState) _hold_state;
  };

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);

  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    GeomList::init_type();
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

  friend class GeomTransformer;
};

#include "geomNode.I"

#endif
