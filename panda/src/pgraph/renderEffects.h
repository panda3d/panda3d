// Filename: renderEffects.h
// Created by:  drose (14Mar02)
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

#ifndef RENDEREFFECTS_H
#define RENDEREFFECTS_H

#include "pandabase.h"

#include "renderEffect.h"
#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "indirectLess.h"
#include "ordered_vector.h"

class BillboardEffect;
class CompassEffect;
class FactoryParams;

////////////////////////////////////////////////////////////////////
//       Class : RenderEffects
// Description : This represents a unique collection of RenderEffect
//               objects that correspond to a particular renderable
//               state.
//
//               You should not attempt to create or modify a
//               RenderEffects object directly.  Instead, call one of
//               the make() functions to create one for you.  And
//               instead of modifying a RenderEffects object, create a
//               new one.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA RenderEffects : public TypedWritableReferenceCount {
protected:
  RenderEffects();

private:
  RenderEffects(const RenderEffects &copy);
  void operator = (const RenderEffects &copy);

public:
  virtual ~RenderEffects();

  bool operator < (const RenderEffects &other) const;

  bool safe_to_transform() const;
  bool safe_to_combine() const;
  CPT(RenderEffects) xform(const LMatrix4f &mat) const;

PUBLISHED:
  INLINE bool is_empty() const;
  INLINE int get_num_effects() const;
  INLINE const RenderEffect *get_effect(int n) const;

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

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

public:
  INLINE const BillboardEffect *get_billboard() const;
  INLINE bool has_decal() const;
  INLINE const CompassEffect *get_compass() const;
  INLINE bool has_show_bounds() const;

private:
  static CPT(RenderEffects) return_new(RenderEffects *state);
  void determine_billboard();
  void determine_decal();
  void determine_compass();
  void determine_show_bounds();

private:
  typedef pset<const RenderEffects *, IndirectLess<RenderEffects> > States;
  static States _states;
  static CPT(RenderEffects) _empty_state;

  // This iterator records the entry corresponding to this RenderEffects
  // object in the above global set.  We keep the iterator around so
  // we can remove it when the RenderEffects destructs.
  States::iterator _saved_entry;

private:
  // This is the actual data within the RenderEffects: a set of
  // RenderEffects.
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

  // We cache the pointer to some critical effects stored in the
  // state, if they exist.
  const BillboardEffect *_billboard;
  const CompassEffect *_compass;

  enum Flags {
    F_checked_billboard    = 0x0001,
    F_checked_decal        = 0x0002,
    F_has_decal            = 0x0004,
    F_checked_show_bounds  = 0x0008,
    F_has_show_bounds      = 0x0010,
    F_checked_compass      = 0x0020,
  };
  short _flags;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual int complete_pointers(TypedWritable **plist, BamReader *manager);
  static TypedWritable *change_this(TypedWritable *old_ptr, BamReader *manager);
  virtual void finalize();

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

INLINE ostream &operator << (ostream &out, const RenderEffects &state) {
  state.output(out);
  return out;
}

#include "renderEffects.I"

#endif

