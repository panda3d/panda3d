/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file characterJointEffect.h
 * @author drose
 * @date 2006-07-26
 */

#ifndef CHARACTERJOINTEFFECT_H
#define CHARACTERJOINTEFFECT_H

#include "pandabase.h"

#include "renderEffect.h"
#include "luse.h"
#include "nodePath.h"
#include "character.h"
#include "weakPointerTo.h"

/**
 * This effect will be added automatically to a node by
 * CharacterJoint::add_net_transform() and
 * CharacterJoint::add_local_transform().
 *
 * The effect binds the node back to the character, so that querying the
 * relative transform of the affected node will automatically force the
 * indicated character to be updated first.
 */
class EXPCL_PANDA_CHAR CharacterJointEffect : public RenderEffect {
private:
  INLINE CharacterJointEffect();

PUBLISHED:
  static CPT(RenderEffect) make(Character *character);

  INLINE PT(Character) get_character() const;

public:
  INLINE bool matches_character(Character *character) const;

  virtual bool safe_to_transform() const;
  virtual bool safe_to_combine() const;
  virtual void output(std::ostream &out) const;

  virtual bool has_cull_callback() const;
  virtual void cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const;

  virtual bool has_adjust_transform() const;
  virtual void adjust_transform(CPT(TransformState) &net_transform,
                                CPT(TransformState) &node_transform,
                                const PandaNode *node) const;

protected:
  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  WPT(Character) _character;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist,
                                BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    RenderEffect::init_type();
    register_type(_type_handle, "CharacterJointEffect",
                  RenderEffect::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "characterJointEffect.I"

#endif
