// Filename: transformState.h
// Created by:  drose (25Feb02)
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

#ifndef TRANSFORMSTATE_H
#define TRANSFORMSTATE_H

#include "pandabase.h"

#include "typedWritableReferenceCount.h"
#include "pointerTo.h"
#include "indirectLess.h"
#include "luse.h"
#include "pset.h"
#include "event.h"

class GraphicsStateGuardianBase;
class FactoryParams;

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

PUBLISHED:
  bool operator < (const TransformState &other) const;

  static CPT(TransformState) make_identity();
  static CPT(TransformState) make_invalid();
  INLINE static CPT(TransformState) make_pos(const LVecBase3f &pos);
  INLINE static CPT(TransformState) make_hpr(const LVecBase3f &hpr);
  INLINE static CPT(TransformState) make_quat(const LQuaternionf &quat);
  INLINE static CPT(TransformState) make_pos_hpr(const LVecBase3f &pos,
                                                 const LVecBase3f &hpr);
  INLINE static CPT(TransformState) make_scale(float scale);
  INLINE static CPT(TransformState) make_scale(const LVecBase3f &scale);
  INLINE static CPT(TransformState) make_shear(const LVecBase3f &scale);
  INLINE static CPT(TransformState) make_pos_hpr_scale(const LVecBase3f &pos,
                                                       const LVecBase3f &hpr, 
                                                       const LVecBase3f &scale);
  INLINE static CPT(TransformState) make_pos_quat_scale(const LVecBase3f &pos,
                                                        const LQuaternionf &quat, 
                                                        const LVecBase3f &scale);
  static CPT(TransformState) make_pos_hpr_scale_shear(const LVecBase3f &pos,
                                                      const LVecBase3f &hpr, 
                                                      const LVecBase3f &scale,
                                                      const LVecBase3f &shear);
  static CPT(TransformState) make_pos_quat_scale_shear(const LVecBase3f &pos,
                                                       const LQuaternionf &quat, 
                                                       const LVecBase3f &scale,
                                                       const LVecBase3f &shear);
  static CPT(TransformState) make_mat(const LMatrix4f &mat);

  INLINE bool is_identity() const;
  INLINE bool is_invalid() const;
  INLINE bool is_singular() const;
  INLINE bool has_components() const;
  INLINE bool components_given() const;
  INLINE bool hpr_given() const;
  INLINE bool quat_given() const;
  INLINE bool has_pos() const;
  INLINE bool has_hpr() const;
  INLINE bool has_quat() const;
  INLINE bool has_scale() const;
  INLINE bool has_uniform_scale() const;
  INLINE bool has_shear() const;
  INLINE bool has_nonzero_shear() const;
  INLINE bool has_mat() const;
  INLINE const LVecBase3f &get_pos() const;
  INLINE const LVecBase3f &get_hpr() const;
  INLINE const LQuaternionf &get_quat() const;
  INLINE const LVecBase3f &get_scale() const;
  INLINE float get_uniform_scale() const;
  INLINE const LVecBase3f &get_shear() const;
  INLINE const LMatrix4f &get_mat() const;

  CPT(TransformState) set_pos(const LVecBase3f &pos) const;
  CPT(TransformState) set_hpr(const LVecBase3f &hpr) const;
  CPT(TransformState) set_quat(const LQuaternionf &quat) const;
  CPT(TransformState) set_scale(const LVecBase3f &scale) const;
  CPT(TransformState) set_shear(const LVecBase3f &shear) const;

  CPT(TransformState) compose(const TransformState *other) const;
  CPT(TransformState) invert_compose(const TransformState *other) const;

  void output(ostream &out) const;
  void write(ostream &out, int indent_level) const;

  static int get_num_states();
  static int get_num_unused_states();
  static int clear_cache();
  static void list_cycles(ostream &out);
  static void list_states(ostream &out);
  static bool validate_states();

private:
  class CompositionCycleDescEntry {
  public:
    INLINE CompositionCycleDescEntry(const TransformState *obj,
                                     const TransformState *result,
                                     bool inverted);

    const TransformState *_obj;
    const TransformState *_result;
    bool _inverted;
  };
  typedef pvector<CompositionCycleDescEntry> CompositionCycleDesc;
  typedef pset<const TransformState *> VisitedStates;

  static CPT(TransformState) return_new(TransformState *state);
  CPT(TransformState) do_compose(const TransformState *other) const;
  CPT(TransformState) do_invert_compose(const TransformState *other) const;
  static bool r_detect_cycles(const TransformState *start_state,
                              const TransformState *current_state,
                              int length,
                              VisitedStates &visited_this_cycle,
                              CompositionCycleDesc &cycle_desc);

private:
  typedef pset<const TransformState *, IndirectLess<TransformState> > States;
  static States *_states;
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
    INLINE Composition();
    INLINE Composition(const Composition &copy);

    // _result is reference counted if and only if it is not the same
    // pointer as this.
    const TransformState *_result;
  };
    
  // The first element of the map is the object we compose with.  This
  // is not reference counted within this map; instead we store a
  // companion pointer in the other object, and remove the references
  // explicitly when either object destructs.
  typedef pmap<const TransformState *, Composition> CompositionCache;
  CompositionCache _composition_cache;
  CompositionCache _invert_composition_cache;

private:
  // This is the actual data within the TransformState.
  INLINE void check_singular() const;
  INLINE void check_components() const;
  INLINE void check_hpr() const;
  INLINE void check_quat() const;
  INLINE void check_mat() const;
  void calc_singular();
  void calc_components();
  void calc_hpr();
  void calc_quat();
  void calc_mat();

  INLINE void check_uniform_scale();

  INLINE void set_destructing();
  INLINE bool is_destructing() const;

  enum Flags {
    F_is_identity        = 0x0001,
    F_is_singular        = 0x0002,
    F_singular_known     = 0x0004,  // set if we know F_is_singular
    F_components_given   = 0x0008,
    F_components_known   = 0x0010,  // set if we know F_has_components
    F_has_components     = 0x0020,
    F_mat_known          = 0x0040,  // set if _mat is defined
    F_is_invalid         = 0x0080,
    F_quat_given         = 0x0100,
    F_quat_known         = 0x0200,  // set if _quat is defined
    F_hpr_given          = 0x0400,
    F_hpr_known          = 0x0800,  // set if _hpr is defined
    F_uniform_scale      = 0x1000,
    F_has_nonzero_shear  = 0x2000,
    F_is_destructing     = 0x8000,
  };
  LVecBase3f _pos, _hpr, _scale, _shear;
  LQuaternionf _quat;
  LMatrix4f _mat;
  LMatrix4f *_inv_mat;
  
  unsigned short _flags;

public:
  static void register_with_read_factory();
  virtual void write_datagram(BamWriter *manager, Datagram &dg);
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

