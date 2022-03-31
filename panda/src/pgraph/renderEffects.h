/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderEffects.h
 * @author drose
 * @date 2002-03-14
 */

#ifndef RENDEREFFECTS_H
#define RENDEREFFECTS_H

#include "pandabase.h"

#include "transformState.h"
#include "renderState.h"

#include "renderEffect.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "ordered_vector.h"
#include "lightReMutex.h"
#include "lightMutex.h"

class CullTraverser;
class CullTraverserData;
class FactoryParams;

/**
 * This represents a unique collection of RenderEffect objects that correspond
 * to a particular renderable state.
 *
 * You should not attempt to create or modify a RenderEffects object directly.
 * Instead, call one of the make() functions to create one for you.  And
 * instead of modifying a RenderEffects object, create a new one.
 */
class EXPCL_PANDA_PGRAPH RenderEffects : public TypedWritableReferenceCount {
protected:
  RenderEffects();

public:
  RenderEffects(const RenderEffects &copy) = delete;
  virtual ~RenderEffects();

  RenderEffects &operator = (const RenderEffects &copy) = delete;

  bool safe_to_transform() const;
  virtual CPT(TransformState) prepare_flatten_transform(const TransformState *net_transform) const;
  bool safe_to_combine() const;
  CPT(RenderEffects) xform(const LMatrix4 &mat) const;

PUBLISHED:
  bool operator < (const RenderEffects &other) const;

  INLINE bool is_empty() const;
  INLINE size_t get_num_effects() const;
  INLINE const RenderEffect *get_effect(size_t n) const;

  INLINE size_t size() const;
  INLINE const RenderEffect *operator [] (size_t n) const;
  INLINE const RenderEffect *operator [] (TypeHandle type) const;

  int find_effect(TypeHandle type) const;

  static CPT(RenderEffects) make_empty();
  static CPT(RenderEffects) make(const RenderEffect *effect);
  static CPT(RenderEffects) make(const RenderEffect *effect1,
                                 const RenderEffect *effect2);
  static CPT(RenderEffects) make(const RenderEffect *effect1,
                                 const RenderEffect *effect2,
                                 const RenderEffect *effect3);
  static CPT(RenderEffects) make(const RenderEffect *effect1,
                                 const RenderEffect *effect2,
                                 const RenderEffect *effect3,
                                 const RenderEffect *effect4);

  CPT(RenderEffects) add_effect(const RenderEffect *effect) const;
  CPT(RenderEffects) remove_effect(TypeHandle type) const;

  const RenderEffect *get_effect(TypeHandle type) const;

  virtual bool unref() const;

  void output(std::ostream &out) const;
  void write(std::ostream &out, int indent_level) const;

  static int get_num_states();
  static void list_states(std::ostream &out);
  static bool validate_states();

public:
  INLINE bool has_decal() const;
  INLINE bool has_show_bounds() const;
  INLINE bool has_show_tight_bounds() const;

  INLINE bool has_cull_callback() const;
  void cull_callback(CullTraverser *trav, CullTraverserData &data,
                     CPT(TransformState) &node_transform,
                     CPT(RenderState) &node_state) const;

  INLINE bool has_adjust_transform() const;
  void adjust_transform(CPT(TransformState) &net_transform,
                        CPT(TransformState) &node_transform,
                        const PandaNode *node) const;

  static void init_states();

private:
  static CPT(RenderEffects) return_new(RenderEffects *state);
  void release_new();

  void determine_decal();
  void determine_show_bounds();
  void determine_cull_callback();
  void determine_adjust_transform();

private:
  // This mutex protects _states.  It also protects any modification to the
  // cache, which is encoded in _composition_cache and
  // _invert_composition_cache.
  static LightReMutex *_states_lock;
  typedef pset<const RenderEffects *, indirect_less<const RenderEffects *> > States;
  static States *_states;
  static CPT(RenderEffects) _empty_state;

  // This iterator records the entry corresponding to this RenderEffects
  // object in the above global set.  We keep the iterator around so we can
  // remove it when the RenderEffects destructs.
  States::iterator _saved_entry;

private:
  // This is the actual data within the RenderEffects: a set of RenderEffects.
  class Effect {
  public:
    INLINE Effect(const RenderEffect *effect);
    INLINE Effect();
    INLINE Effect(TypeHandle type);
    INLINE Effect(const Effect &copy);
    INLINE void operator = (const Effect &copy);
    INLINE bool operator < (const Effect &other) const;
    INLINE int compare_to(const Effect &other) const;

    TypeHandle _type;
    CPT(RenderEffect) _effect;
  };
  typedef ov_set<Effect> Effects;
  Effects _effects;

  enum Flags {
    F_checked_decal            = 0x0001,
    F_has_decal                = 0x0002,
    F_checked_show_bounds      = 0x0004,
    F_has_show_bounds          = 0x0008,
    F_has_show_tight_bounds    = 0x0010,
    F_checked_cull_callback    = 0x0020,
    F_has_cull_callback        = 0x0040,
    F_checked_adjust_transform = 0x0080,
    F_has_adjust_transform     = 0x0100,
  };
  int _flags;

  // This mutex protects _flags, and all of the above computed values.
  LightMutex _lock;


public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  virtual bool require_fully_complete() const;
  static TypedWritable *change_this(TypedWritable *old_ptr, BamReader *manager);
  virtual void finalize(BamReader *manager);

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  void fillin(DatagramIterator &scan, BamReader *manager);

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "RenderEffects",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE std::ostream &operator << (std::ostream &out, const RenderEffects &state) {
  state.output(out);
  return out;
}

#include "renderEffects.I"

#endif
