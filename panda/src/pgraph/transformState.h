// Filename: transformState.h
// Created by:  drose (25Feb02)
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

#ifndef TRANSFORMSTATE_H
#define TRANSFORMSTATE_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "indirectLess.h"
#include "luse.h"
#include "pset.h"

class GraphicsStateGuardianBase;

////////////////////////////////////////////////////////////////////
//       Class : TransformState
// Description : Indicates a coordinate-system transform on vertices.
//               TransformStates are the primary means for storing
//               transformations on the scene graph.
//
//               Transforms may be specified in one of two ways:
//               componentwise, with a pos-hpr-scale, or with an
//               arbitrary transform matrix.  If you specify a
//               transform componentwise, it will remember its
//               original components.
//
//               TransformState objects are managed very much like
//               RenderState objects.  They are immutable and
//               reference-counted automatically.
//
//               You should not attempt to create or modify a
//               TransformState object directly.  Instead, call one of
//               the make() functions to create one for you.  And
//               instead of modifying a TransformState object, create a
//               new one.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TransformState : public TypedWritableReferenceCount {
protected:
  TransformState();

private:
  TransformState(const TransformState &copy);
  void operator = (const TransformState &copy);

public:
  virtual ~TransformState();

  bool operator < (const TransformState &other) const;

PUBLISHED:
  static CPT(TransformState) make_identity();
  INLINE static CPT(TransformState) make_pos(const LVecBase3f &pos);
  INLINE static CPT(TransformState) make_hpr(const LVecBase3f &hpr);
  INLINE static CPT(TransformState) make_pos_hpr(const LVecBase3f &pos,
                                                 const LVecBase3f &hpr);
  INLINE static CPT(TransformState) make_scale(float scale);
  INLINE static CPT(TransformState) make_scale(const LVecBase3f &scale);
  static CPT(TransformState) make_pos_hpr_scale(const LVecBase3f &pos,
                                                const LVecBase3f &hpr, 
                                                const LVecBase3f &scale);
  static CPT(TransformState) make_mat(const LMatrix4f &mat);

  INLINE bool is_identity() const;
  INLINE bool is_singular() const;
  INLINE bool has_components() const;
  INLINE const LVecBase3f &get_pos() const;
  INLINE const LVecBase3f &get_hpr() const;
  INLINE const LVecBase3f &get_scale() const;
  INLINE const LMatrix4f &get_mat() const;

  CPT(TransformState) compose(const TransformState *other) const;
  CPT(TransformState) invert_compose(const TransformState *other) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

private:
  static CPT(TransformState) return_new(TransformState *state);
  CPT(TransformState) do_compose(const TransformState *other) const;
  CPT(TransformState) do_invert_compose(const TransformState *other) const;

private:
  typedef pset<const TransformState *, IndirectLess<TransformState> > States;
  static States _states;
  static CPT(TransformState) _identity_state;

  // This iterator records the entry corresponding to this TransformState
  // object in the above global set.  We keep the iterator around so
  // we can remove it when the TransformState destructs.
  States::iterator _saved_entry;

  // This data structure manages the job of caching the composition of
  // two TransformStates.  It's complicated because we have to be sure to
  // remove the entry if *either* of the input TransformStates destructs.
  // To implement this, we always record Composition entries in pairs,
  // one in each of the two involved TransformState objects.
  class Composition {
  public:
    CPT(TransformState) _result;
  };
    
  typedef pmap<const TransformState *, Composition> CompositionCache;
  CompositionCache _composition_cache;
  CompositionCache _invert_composition_cache;

  // Thise pointer is used to cache the result of compose(this).  This
  // has to be a special case, because we have to handle the reference
  // counts carefully so that we don't leak.
  const TransformState *_self_compose;

private:
  // This is the actual data within the TransformState.
  INLINE void check_singular() const;
  INLINE void check_components() const;
  INLINE void check_mat() const;
  void calc_singular();
  void calc_components();
  void calc_mat();

  enum Flags {
    F_is_identity      =  0x0001,
    F_is_singular      =  0x0002,
    F_singular_known   =  0x0004,
    F_components_given =  0x0008,
    F_components_known =  0x0010,
    F_has_components   =  0x0020,
    F_mat_known        =  0x0040,
  };
  LVecBase3f _pos, _hpr, _scale;
  LMatrix4f _mat;
  
  short _flags;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
  virtual void finalize();

protected:
  static TypedWritable *make_from_bam(const FactoryParams &params);
  static TypedWritable *new_from_bam(TransformState *state, BamReader *manager);
  void fillin(DatagramIterator &scan, BamReader *manager);
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    TypedWritableReferenceCount::init_type();
    register_type(_type_handle, "TransformState",
                  TypedWritableReferenceCount::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

INLINE ostream &operator << (ostream &out, const TransformState &state) {
  state.output(out);
  return out;
}

#include "transformState.I"

#endif

