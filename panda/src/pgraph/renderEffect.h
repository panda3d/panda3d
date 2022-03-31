/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderEffect.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef RENDEREFFECT_H
#define RENDEREFFECT_H

#include "pandabase.h"

#include "transformState.h"
#include "renderState.h"

#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "pset.h"
#include "luse.h"

class CullTraverser;
class CullTraverserData;
class PandaNode;

/**
 * This is the base class for a number of special render effects that may be
 * set on scene graph nodes to change the way they render.  This includes
 * BillboardEffect, DecalEffect, etc.
 *
 * RenderEffect represents render properties that must be applied as soon as
 * they are encountered in the scene graph, rather than propagating down to
 * the leaves.  This is different from RenderAttrib, which represents
 * properties like color and texture that don't do anything until they
 * propagate down to a GeomNode.
 *
 * You should not attempt to create or modify a RenderEffect directly;
 * instead, use the make() method of the appropriate kind of effect you want.
 * This will allocate and return a new RenderEffect of the appropriate type,
 * and it may share pointers if possible.  Do not modify the new RenderEffect
 * if you wish to change its properties; instead, create a new one.
 */
class EXPCL_PANDA_PGRAPH RenderEffect : public TypedWritableReferenceCount {
protected:
  RenderEffect();

public:
  RenderEffect(const RenderEffect &copy) = delete;
  virtual ~RenderEffect();

  RenderEffect &operator = (const RenderEffect &copy) = delete;

  virtual bool safe_to_transform() const;
  virtual CPT(TransformState) prepare_flatten_transform(const TransformState *net_transform) const;
  virtual bool safe_to_combine() const;
  virtual CPT(RenderEffect) xform(const LMatrix4 &mat) const;

  virtual bool has_cull_callback() const;
  virtual void cull_callback(CullTraverser *trav, CullTraverserData &data,
                             CPT(TransformState) &node_transform,
                             CPT(RenderState) &node_state) const;

  virtual bool has_adjust_transform() const;
  virtual void adjust_transform(CPT(TransformState) &net_transform,
                                CPT(TransformState) &node_transform,
                                const PandaNode *node) const;

PUBLISHED:
  INLINE int compare_to(const RenderEffect &other) const;

  virtual void output(std::ostream &out) const;
  virtual void write(std::ostream &out, int indent_level) const;

  static int get_num_effects();
  static void list_effects(std::ostream &out);
  static bool validate_effects();

protected:
  static CPT(RenderEffect) return_new(RenderEffect *effect);

  virtual int compare_to_impl(const RenderEffect *other) const;

private:
  typedef pset<const RenderEffect *, indirect_compare_to<const RenderEffect *> > Effects;
  static Effects *_effects;

  Effects::iterator _saved_entry;

public:
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  static TypedWritable *change_this(TypedWritable *old_ptr, BamReader *manager);
  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *new_from_bam(RenderEffect *effect, BamReader *manager);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "RenderEffect",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const RenderEffect &effect) {
  effect.output(out);
  return out;
}

#include "renderEffect.I"

#endif
