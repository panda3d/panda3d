/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file occluderEffect.h
 * @author drose
 * @date 2011-03-17
 */

#ifndef OCCLUDEREFFECT_H
#define OCCLUDEREFFECT_H

#include "pandabase.h"

#include "occluderNode.h"
#include "renderEffect.h"
#include "nodePath.h"
#include "ordered_vector.h"
#include "pmap.h"

/**
 * This functions similarly to a LightAttrib or ClipPlaneAttrib.  It indicates
 * the set of occluders that modify the geometry at this level and below.
 * Unlike a ClipPlaneAttrib, an OccluderEffect takes effect immediately when
 * it is encountered during traversal, and thus can only add occluders; it may
 * not remove them.
 */
class EXPCL_PANDA_PGRAPH OccluderEffect : public RenderEffect {
private:
  INLINE OccluderEffect();
  INLINE OccluderEffect(const OccluderEffect &copy);

PUBLISHED:
  static CPT(RenderEffect) make();

  INLINE int get_num_on_occluders() const;
  INLINE NodePath get_on_occluder(int n) const;
  MAKE_SEQ(get_on_occluders, get_num_on_occluders, get_on_occluder);
  INLINE bool has_on_occluder(const NodePath &occluder) const;

  INLINE bool is_identity() const;

  CPT(RenderEffect) add_on_occluder(const NodePath &occluder) const;
  CPT(RenderEffect) remove_on_occluder(const NodePath &occluder) const;

public:
  virtual void output(std::ostream &out) const;

protected:
  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  void sort_on_occluders();

private:
  typedef ov_set<NodePath> Occluders;
  Occluders _on_occluders;

  UpdateSeq _sort_seq;

  static CPT(RenderEffect) _empty_effect;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  virtual bool require_fully_complete() const;

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderEffect::init_type();
    register_type(_type_handle, "OccluderEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "occluderEffect.I"

#endif
