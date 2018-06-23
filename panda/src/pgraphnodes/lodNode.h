/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file lodNode.h
 * @author drose
 * @date 2002-03-06
 */

#ifndef LODNODE_H
#define LODNODE_H

#include "pandabase.h"
#include "config_pgraphnodes.h"
#include "pandaNode.h"
#include "luse.h"
#include "pvector.h"

/**
 * A Level-of-Detail node.  This selects only one of its children for
 * rendering, according to the distance from the camera and the table
 * indicated in the associated LOD object.
 */
class EXPCL_PANDA_PGRAPHNODES LODNode : public PandaNode {
PUBLISHED:
  INLINE explicit LODNode(const std::string &name);

  static PT(LODNode) make_default_lod(const std::string &name);

protected:
  INLINE LODNode(const LODNode &copy);
public:
  virtual PandaNode *make_copy() const;
  virtual bool safe_to_combine() const;
  virtual bool safe_to_combine_children() const;
  virtual void xform(const LMatrix4 &mat);
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);

  virtual void output(std::ostream &out) const;

  virtual bool is_lod_node() const;

PUBLISHED:
  // The sense of in vs.  out distances is as if the object were coming
  // towards you from far away: it switches "in" at the far distance, and
  // switches "out" at the close distance.  Thus, "in" should be larger than
  // "out".

  INLINE void add_switch(PN_stdfloat in, PN_stdfloat out);
  INLINE bool set_switch(int index, PN_stdfloat in, PN_stdfloat out);
  INLINE void clear_switches();

  INLINE int get_num_switches() const;
  INLINE PN_stdfloat get_in(int index) const;
  MAKE_SEQ(get_ins, get_num_switches, get_in);
  INLINE PN_stdfloat get_out(int index) const;
  MAKE_SEQ(get_outs, get_num_switches, get_out);

  INLINE int get_lowest_switch() const;
  INLINE int get_highest_switch() const;

  INLINE void force_switch(int index);
  INLINE void clear_force_switch();

  // for performance tuning, increasing this value should improve performance
  // at the cost of model quality
  INLINE void set_lod_scale(PN_stdfloat value);
  INLINE PN_stdfloat get_lod_scale() const;


  INLINE void set_center(const LPoint3 &center);
  INLINE const LPoint3 &get_center() const;

  MAKE_SEQ_PROPERTY(ins, get_num_switches, get_in);
  MAKE_SEQ_PROPERTY(outs, get_num_switches, get_out);
  MAKE_PROPERTY(lowest_switch, get_lowest_switch);
  MAKE_PROPERTY(highest_switch, get_highest_switch);
  MAKE_PROPERTY(lod_scale, get_lod_scale, set_lod_scale);
  MAKE_PROPERTY(center, get_center, set_center);

  void show_switch(int index);
  void show_switch(int index, const LColor &color);
  void hide_switch(int index);
  void show_all_switches();
  void hide_all_switches();
  INLINE bool is_any_shown() const;

  bool verify_child_bounds() const;

protected:
  int compute_child(CullTraverser *trav, CullTraverserData &data);

  bool show_switches_cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual void compute_internal_bounds(CPT(BoundingVolume) &internal_bounds,
                                       int &internal_vertices,
                                       int pipeline_stage,
                                       Thread *current_thread) const;

  INLINE void consider_verify_lods(CullTraverser *trav, CullTraverserData &data);

  CPT(TransformState) get_rel_transform(CullTraverser *trav, CullTraverserData &data);

private:
  class CData;
  void do_show_switch(CData *cdata, int index, const LColor &color);
  void do_hide_switch(CData *cdata, int index);
  bool do_verify_child_bounds(const CData *cdata, int index,
                              PN_stdfloat &suggested_radius) const;
  void do_auto_verify_lods(CullTraverser *trav, CullTraverserData &data);

  static const LColor &get_default_show_color(int index);

protected:
  class Switch {
  public:
    INLINE Switch(PN_stdfloat in, PN_stdfloat out);
    INLINE PN_stdfloat get_in() const;
    INLINE PN_stdfloat get_out() const;

    INLINE void set_range(PN_stdfloat in, PN_stdfloat out);
    INLINE bool in_range(PN_stdfloat dist) const;
    INLINE bool in_range_2(PN_stdfloat dist2) const;

    INLINE void rescale(PN_stdfloat factor);

    INLINE bool is_shown() const;
    INLINE void show(const LColor &color);
    INLINE void hide();

    INLINE PandaNode *get_ring_viz() const;
    INLINE PandaNode *get_spindle_viz() const;
    INLINE const RenderState *get_viz_model_state() const;

    INLINE void write_datagram(Datagram &destination) const;
    INLINE void read_datagram(DatagramIterator &source);

  private:
    INLINE void clear_ring_viz();

    void compute_ring_viz();
    void compute_spindle_viz();
    void compute_viz_model_state();

  private:
    PN_stdfloat _in;
    PN_stdfloat _out;
    bool _shown;
    UnalignedLVecBase4 _show_color;
    PT(PandaNode) _ring_viz;
    PT(PandaNode) _spindle_viz;
    CPT(RenderState) _viz_model_state;

  public:
    UpdateSeq _bounds_seq;
    bool _verify_ok;
  };
  typedef pvector<Switch> SwitchVector;

private:
  class EXPCL_PANDA_PGRAPHNODES CData : public CycleData {
  public:
    INLINE CData();
    INLINE CData(const CData &copy);
    virtual CycleData *make_copy() const;

    void check_limits();

    virtual void write_datagram(BamWriter *manager, Datagram &dg) const;
    virtual void fillin(DatagramIterator &scan, BamReader *manager);
    virtual TypeHandle get_parent_type() const {
      return LODNode::get_class_type();
    }

    LPoint3 _center;
    SwitchVector _switch_vector;
    size_t _lowest, _highest;
    UpdateSeq _bounds_seq;

    bool _got_force_switch;
    int _force_switch;
    int _num_shown;
    PN_stdfloat _lod_scale;
  };

  PipelineCycler<CData> _cycler;
  typedef CycleDataReader<CData> CDReader;
  typedef CycleDataWriter<CData> CDWriter;
  typedef CycleDataLockedReader<CData> CDLockedReader;
  typedef CycleDataStageReader<CData> CDStageReader;
  typedef CycleDataStageWriter<CData> CDStageWriter;

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
    register_type(_type_handle, "LODNode",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "lodNode.I"

#endif
