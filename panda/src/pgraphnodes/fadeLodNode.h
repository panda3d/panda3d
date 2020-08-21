/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fadeLodNode.h
 * @author sshodhan
 * @date 2004-06-14
 */

#ifndef FADELODNODE_H
#define FADELODNODE_H

#include "pandabase.h"

#include "lodNode.h"

/**
 * A Level-of-Detail node with alpha based switching.
 */
class EXPCL_PANDA_PGRAPHNODES FadeLODNode : public LODNode {
PUBLISHED:
  explicit FadeLODNode(const std::string &name);

protected:
  FadeLODNode(const FadeLODNode &copy);
public:
  virtual PandaNode *make_copy() const;
  virtual bool cull_callback(CullTraverser *trav, CullTraverserData &data);
  virtual void output(std::ostream &out) const;

PUBLISHED:
  INLINE void set_fade_time(PN_stdfloat t);
  INLINE PN_stdfloat get_fade_time() const;
  MAKE_PROPERTY(fade_time, get_fade_time, set_fade_time);

  void set_fade_bin(const std::string &name, int draw_order);
  INLINE const std::string &get_fade_bin_name() const;
  INLINE int get_fade_bin_draw_order() const;
  MAKE_PROPERTY(fade_bin_name, get_fade_bin_name);
  MAKE_PROPERTY(fade_bin_draw_order, get_fade_bin_draw_order);

  void set_fade_state_override(int override);
  INLINE int get_fade_state_override() const;
  MAKE_PROPERTY(fade_state_override, get_fade_state_override,
                                     set_fade_state_override);

private:
  CPT(RenderState) get_fade_1_old_state();
  CPT(RenderState) get_fade_1_new_state(PN_stdfloat in_alpha);
  CPT(RenderState) get_fade_2_old_state(PN_stdfloat out_alpha);
  CPT(RenderState) get_fade_2_new_state();

private:
  PN_stdfloat _fade_time;
  std::string _fade_bin_name;
  int _fade_bin_draw_order;
  int _fade_state_override;

  CPT(RenderState) _fade_1_new_state;
  CPT(RenderState) _fade_1_old_state;
  CPT(RenderState) _fade_2_new_state;
  CPT(RenderState) _fade_2_old_state;

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
    LODNode::init_type();
    register_type(_type_handle, "FadeLODNode",
                  LODNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "fadeLodNode.I"

#endif
