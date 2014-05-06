// Filename: transformState.cxx
// Created by:  drose (25Feb02)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "transformState.h"
#include "compose_matrix.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagramIterator.h"
#include "indent.h"
#include "compareTo.h"
#include "pStatTimer.h"
#include "config_pgraph.h"
#include "lightReMutexHolder.h"
#include "lightMutexHolder.h"
#include "thread.h"
#include "py_panda.h"

LightReMutex *TransformState::_states_lock = NULL;
TransformState::States *TransformState::_states = NULL;
CPT(TransformState) TransformState::_identity_state;
CPT(TransformState) TransformState::_invalid_state;
UpdateSeq TransformState::_last_cycle_detect;
int TransformState::_garbage_index = 0;

PStatCollector TransformState::_cache_update_pcollector("*:State Cache:Update");
PStatCollector TransformState::_garbage_collect_pcollector("*:State Cache:Garbage Collect");
PStatCollector TransformState::_transform_compose_pcollector("*:State Cache:Compose Transform");
PStatCollector TransformState::_transform_invert_pcollector("*:State Cache:Invert Transform");
PStatCollector TransformState::_transform_calc_pcollector("*:State Cache:Calc Components");
PStatCollector TransformState::_transform_break_cycles_pcollector("*:State Cache:Break Cycles");
PStatCollector TransformState::_transform_new_pcollector("*:State Cache:New");
PStatCollector TransformState::_transform_validate_pcollector("*:State Cache:Validate");
PStatCollector TransformState::_transform_hash_pcollector("*:State Cache:Calc Hash");
PStatCollector TransformState::_node_counter("TransformStates:On nodes");
PStatCollector TransformState::_cache_counter("TransformStates:Cached");

CacheStats TransformState::_cache_stats;

TypeHandle TransformState::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TransformState::Constructor
//       Access: Protected
//  Description: Actually, this could be a private constructor, since
//               no one inherits from TransformState, but gcc gives us a
//               spurious warning if all constructors are private.
////////////////////////////////////////////////////////////////////
TransformState::
TransformState() : _lock("TransformState") {
  if (_states == (States *)NULL) {
    init_states();
  }
  _saved_entry = -1;
  _flags = F_is_identity | F_singular_known | F_is_2d;
  _inv_mat = (LMatrix4 *)NULL;
  _cache_stats.add_num_states(1);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::Copy Constructor
//       Access: Private
//  Description: TransformStates are not meant to be copied.
////////////////////////////////////////////////////////////////////
TransformState::
TransformState(const TransformState &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::Copy Assignment Operator
//       Access: Private
//  Description: TransformStates are not meant to be copied.
////////////////////////////////////////////////////////////////////
void TransformState::
operator = (const TransformState &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::Destructor
//       Access: Public, Virtual
//  Description: The destructor is responsible for removing the
//               TransformState from the global set if it is there.
////////////////////////////////////////////////////////////////////
TransformState::
~TransformState() {
  // We'd better not call the destructor twice on a particular object.
  nassertv(!is_destructing());
  set_destructing();
 
  // Free the inverse matrix computation, if it has been stored.
  if (_inv_mat != (LMatrix4 *)NULL) {
    delete _inv_mat;
    _inv_mat = (LMatrix4 *)NULL;
  }

  LightReMutexHolder holder(*_states_lock);

  // unref() should have cleared these.
  nassertv(_saved_entry == -1);
  nassertv(_composition_cache.is_empty() && _invert_composition_cache.is_empty());

  // If this was true at the beginning of the destructor, but is no
  // longer true now, probably we've been double-deleted.
  nassertv(get_ref_count() == 0);
  _cache_stats.add_num_states(-1);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::compare_to
//       Access: Published
//  Description: Provides an arbitrary ordering among all unique
//               TransformStates, so we can store the essentially
//               different ones in a big set and throw away the rest.
//
//               If uniquify_matrix is true, then matrix-defined
//               TransformStates are also uniqified.  If
//               uniquify_matrix is false, then only component-defined
//               TransformStates are uniquified, which is less
//               expensive.
////////////////////////////////////////////////////////////////////
int TransformState::
compare_to(const TransformState &other, bool uniquify_matrix) const {
  static const int significant_flags = 
    (F_is_invalid | F_is_identity | F_components_given | F_hpr_given | F_quat_given | F_is_2d);

  int flags = (_flags & significant_flags);
  int other_flags = (other._flags & significant_flags);
  if (flags != other_flags) {
    return flags < other_flags ? -1 : 1;
  }

  if ((_flags & (F_is_invalid | F_is_identity)) != 0) {
    // All invalid transforms are equivalent to each other, and all
    // identity transforms are equivalent to each other.
    return 0;
  }

  if ((_flags & F_components_given) != 0) {
    // If the transform was specified componentwise, compare them
    // componentwise.
    int c = _pos.compare_to(other._pos);
    if (c != 0) {
      return c;
    }

    if ((_flags & F_hpr_given) != 0) {
      c = _hpr.compare_to(other._hpr);
      if (c != 0) {
        return c;
      }
    } else if ((_flags & F_quat_given) != 0) {
      c = _quat.compare_to(other._quat);
      if (c != 0) {
        return c;
      }
    }

    c = _scale.compare_to(other._scale);
    if (c != 0) {
      return c;
    }

    c = _shear.compare_to(other._shear);
    return c;
  }

  // Otherwise, compare the matrices . . .
  if (uniquify_matrix) {
    // . . . but only if the user thinks that's a worthwhile
    // comparison.
    return get_mat().compare_to(other.get_mat());

  } else {
    // If not, we just compare the pointers.
    if (this != &other) {
      return (this < &other) ? -1 : 1;
    }
    return 0;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_identity
//       Access: Published, Static
//  Description: Constructs an identity transform.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
make_identity() {
  // The identity state is asked for so often, we make it a special case
  // and store a pointer forever once we find it the first time.
  if (_identity_state == (TransformState *)NULL) {
    TransformState *state = new TransformState;
    _identity_state = return_unique(state);
  }

  return _identity_state;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_invalid
//       Access: Published, Static
//  Description: Constructs an invalid transform; for instance, the
//               result of inverting a singular matrix.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
make_invalid() {
  if (_invalid_state == (TransformState *)NULL) {
    TransformState *state = new TransformState;
    state->_flags = F_is_invalid | F_singular_known | F_is_singular | F_components_known | F_mat_known;
    _invalid_state = return_unique(state);
  }

  return _invalid_state;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos_hpr_scale_shear
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
make_pos_hpr_scale_shear(const LVecBase3 &pos, const LVecBase3 &hpr, 
                         const LVecBase3 &scale, const LVecBase3 &shear) {
  nassertr(!(pos.is_nan() || hpr.is_nan() || scale.is_nan() || shear.is_nan()) , make_invalid());
  // Make a special-case check for the identity transform.
  if (pos == LVecBase3(0.0f, 0.0f, 0.0f) &&
      hpr == LVecBase3(0.0f, 0.0f, 0.0f) &&
      scale == LVecBase3(1.0f, 1.0f, 1.0f) &&
      shear == LVecBase3(0.0f, 0.0f, 0.0f)) {
    return make_identity();
  }

  TransformState *state = new TransformState;
  state->_pos = pos;
  state->_hpr = hpr;
  state->_scale = scale;
  state->_shear = shear;
  state->_flags = F_components_given | F_hpr_given | F_components_known | F_hpr_known | F_has_components;
  state->check_uniform_scale();
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos_quat_scale_shear
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
make_pos_quat_scale_shear(const LVecBase3 &pos, const LQuaternion &quat, 
                          const LVecBase3 &scale, const LVecBase3 &shear) {
  nassertr(!(pos.is_nan() || quat.is_nan() || scale.is_nan() || shear.is_nan()) , make_invalid());
  // Make a special-case check for the identity transform.
  if (pos == LVecBase3(0.0f, 0.0f, 0.0f) &&
      quat == LQuaternion::ident_quat() &&
      scale == LVecBase3(1.0f, 1.0f, 1.0f) &&
      shear == LVecBase3(0.0f, 0.0f, 0.0f)) {
    return make_identity();
  }

  TransformState *state = new TransformState;
  state->_pos = pos;
  state->_quat = quat;
  state->_scale = scale;
  state->_shear = shear;
  state->_flags = F_components_given | F_quat_given | F_components_known | F_quat_known | F_has_components;
  state->check_uniform_scale();
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_mat
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               transformation matrix.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
make_mat(const LMatrix4 &mat) {
  nassertr(!mat.is_nan(), make_invalid());
  // Make a special-case check for the identity matrix.
  if (mat == LMatrix4::ident_mat()) {
    return make_identity();
  }

  TransformState *state = new TransformState;
  state->_mat = mat;
  state->_flags = F_mat_known;
  return return_new(state);
}


////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos_rotate_scale_shear2d
//       Access: Published, Static
//  Description: Makes a new two-dimensional TransformState with the
//               specified components.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
make_pos_rotate_scale_shear2d(const LVecBase2 &pos, PN_stdfloat rotate,
                              const LVecBase2 &scale,
                              PN_stdfloat shear) {
  nassertr(!(pos.is_nan() || cnan(rotate) || scale.is_nan() || cnan(shear)) , make_invalid());
  // Make a special-case check for the identity transform.
  if (pos == LVecBase2(0.0f, 0.0f) &&
      rotate == 0.0f &&
      scale == LVecBase2(1.0f, 1.0f) &&
      shear == 0.0f) {
    return make_identity();
  }

  TransformState *state = new TransformState;
  state->_pos.set(pos[0], pos[1], 0.0f);
  switch (get_default_coordinate_system()) {
  default:
  case CS_zup_right:
    state->_hpr.set(rotate, 0.0f, 0.0f);
    break;
  case CS_zup_left:
    state->_hpr.set(-rotate, 0.0f, 0.0f);
    break;
  case CS_yup_right:
    state->_hpr.set(0.0f, 0.0f, -rotate);
    break;
  case CS_yup_left:
    state->_hpr.set(0.0, 0.0f, rotate);
    break;
  }
  state->_scale.set(scale[0], scale[1], 1.0f);
  state->_shear.set(shear, 0.0f, 0.0f);
  state->_flags = F_components_given | F_hpr_given | F_components_known | F_hpr_known | F_has_components | F_is_2d;
  state->check_uniform_scale2d();
  return return_new(state);
}


////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_mat3
//       Access: Published, Static
//  Description: Makes a new two-dimensional TransformState with the
//               specified 3x3 transformation matrix.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
make_mat3(const LMatrix3 &mat) {
  nassertr(!mat.is_nan(), make_invalid());
  // Make a special-case check for the identity matrix.
  if (mat == LMatrix3::ident_mat()) {
    return make_identity();
  }

  TransformState *state = new TransformState;
  state->_mat.set(mat(0, 0), mat(0, 1), 0.0f, mat(0, 2),
                  mat(1, 0), mat(1, 1), 0.0f, mat(1, 2),
                  0.0f, 0.0f, 1.0f, 0.0f,
                  mat(2, 0), mat(2, 1), 0.0f, mat(2, 2));
  state->_flags = F_mat_known | F_is_2d;
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_pos
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original TransformState with its pos component
//               replaced with the indicated value.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_pos(const LVecBase3 &pos) const {
  nassertr(!pos.is_nan(), this);
  nassertr(!is_invalid(), this);
  if (is_identity() || components_given()) {
    // If we started with a componentwise transform, we keep it that
    // way.
    if (quat_given()) {
      return make_pos_quat_scale_shear(pos, get_quat(), get_scale(), get_shear());
    } else {
      return make_pos_hpr_scale_shear(pos, get_hpr(), get_scale(), get_shear());
    }

  } else {
    // Otherwise, we have a matrix transform, and we keep it that way.
    LMatrix4 mat = get_mat();
    mat.set_row(3, pos);
    return make_mat(mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_hpr
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original TransformState with its rotation component
//               replaced with the indicated value, if possible.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_hpr(const LVecBase3 &hpr) const {
  nassertr(!hpr.is_nan(), this);
  nassertr(!is_invalid(), this);
  //  nassertr(has_components(), this);
  return make_pos_hpr_scale_shear(get_pos(), hpr, get_scale(), get_shear());
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_quat
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original TransformState with its rotation component
//               replaced with the indicated value, if possible.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_quat(const LQuaternion &quat) const {
  nassertr(!quat.is_nan(), this);
  nassertr(!is_invalid(), this);
  //  nassertr(has_components(), this);
  return make_pos_quat_scale_shear(get_pos(), quat, get_scale(), get_shear());
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_scale
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original TransformState with its scale component
//               replaced with the indicated value, if possible.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_scale(const LVecBase3 &scale) const {
  nassertr(!scale.is_nan(), this);
  nassertr(!is_invalid(), this);

  if (is_2d() && scale[0] == scale[1] && scale[1] == scale[2]) {
    // Don't inflate from 2-d to 3-d just because we got a uniform
    // scale.
    return make_pos_rotate_scale_shear2d(get_pos2d(), get_rotate2d(),
                                         LVecBase2(scale[0], scale[0]),
                                         get_shear2d());
  }

  //  nassertr(has_components(), this);
  if (quat_given()) {
    return make_pos_quat_scale_shear(get_pos(), get_quat(), scale, get_shear());
  } else {
    return make_pos_hpr_scale_shear(get_pos(), get_hpr(), scale, get_shear());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_shear
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original TransformState with its shear component
//               replaced with the indicated value, if possible.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_shear(const LVecBase3 &shear) const {
  nassertr(!shear.is_nan(), this);
  nassertr(!is_invalid(), this);
  //  nassertr(has_components(), this);
  if (quat_given()) {
    return make_pos_quat_scale_shear(get_pos(), get_quat(), get_scale(), shear);
  } else {
    return make_pos_hpr_scale_shear(get_pos(), get_hpr(), get_scale(), shear);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_pos2d
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original 2-d TransformState with its pos component
//               replaced with the indicated value.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_pos2d(const LVecBase2 &pos) const {
  nassertr(!pos.is_nan(), this);
  nassertr(!is_invalid(), this);
  if (!is_2d()) {
    return set_pos(LVecBase3(pos[0], pos[1], 0.0f));
  }

  if (is_identity() || components_given()) {
    // If we started with a componentwise transform, we keep it that
    // way.
    return make_pos_rotate_scale_shear2d(pos, get_rotate2d(), get_scale2d(),
                                         get_shear2d());

  } else {
    // Otherwise, we have a matrix transform, and we keep it that way.
    LMatrix3 mat = get_mat3();
    mat.set_row(2, pos);
    return make_mat3(mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_rotate2d
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original 2-d TransformState with its rotation component
//               replaced with the indicated value, if possible.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_rotate2d(PN_stdfloat rotate) const {
  nassertr(!cnan(rotate), this);
  nassertr(!is_invalid(), this);

  if (!is_2d()) {
    switch (get_default_coordinate_system()) {
    default:
    case CS_zup_right:
      return set_hpr(LVecBase3(rotate, 0.0f, 0.0f));
    case CS_zup_left:
      return set_hpr(LVecBase3(-rotate, 0.0f, 0.0f));
    case CS_yup_right:
      return set_hpr(LVecBase3(0.0f, 0.0f, -rotate));
    case CS_yup_left:
      return set_hpr(LVecBase3(0.0f, 0.0f, rotate));
    }
  }

  return make_pos_rotate_scale_shear2d(get_pos2d(), rotate, get_scale2d(), 
                                       get_shear2d());
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_scale2d
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original 2-d TransformState with its scale component
//               replaced with the indicated value, if possible.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_scale2d(const LVecBase2 &scale) const {
  nassertr(!scale.is_nan(), this);
  nassertr(!is_invalid(), this);

  if (!is_2d()) {
    return set_scale(LVecBase3(scale[0], scale[1], 1.0f));
  }
  return make_pos_rotate_scale_shear2d(get_pos2d(), get_rotate2d(),
                                       scale, get_shear2d());
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_shear2d
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original 2-d TransformState with its shear component
//               replaced with the indicated value, if possible.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_shear2d(PN_stdfloat shear) const {
  nassertr(!cnan(shear), this);
  nassertr(!is_invalid(), this);
  if (!is_2d()) {
    return set_shear(LVecBase3(shear, 0.0f, 0.0f));
  }
  return make_pos_rotate_scale_shear2d(get_pos2d(), get_rotate2d(),
                                       get_scale2d(), shear);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::compose
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               composition of this state with the other state.
//
//               The result of this operation is cached, and will be
//               retained as long as both this TransformState object and
//               the other TransformState object continue to exist.
//               Should one of them destruct, the cached entry will be
//               removed, and its pointer will be allowed to destruct
//               as well.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
compose(const TransformState *other) const {
  // We handle identity as a trivial special case.
  if (is_identity()) {
    return other;
  }
  if (other->is_identity()) {
    return this;
  }
 
  // If either transform is invalid, the result is invalid.
  if (is_invalid()) {
    return this;
  }
  if (other->is_invalid()) {
    return other;
  }

  if (!transform_cache) {
    return do_compose(other);
  }

  // Is this composition already cached?
  CPT(TransformState) result;
  {
    LightReMutexHolder holder(*_states_lock);
    int index = _composition_cache.find(other);
    if (index != -1) {
      const Composition &comp = _composition_cache.get_data(index);
      result = comp._result;
    }
    if (result != (TransformState *)NULL) {
      _cache_stats.inc_hits();
    }
  }

  if (result != (TransformState *)NULL) {
    // Success!
    return result;
  }

  // Not in the cache.  Compute a new result.  It's important that we
  // don't hold the lock while we do this, or we lose the benefit of
  // parallelization.
  result = do_compose(other);

  // It's OK to cast away the constness of this pointer, because the
  // cache is a transparent property of the class.
  return ((TransformState *)this)->store_compose(other, result);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::invert_compose
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               composition of this state's inverse with the other
//               state.
//
//               This is similar to compose(), but is particularly
//               useful for computing the relative state of a node as
//               viewed from some other node.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
invert_compose(const TransformState *other) const {
  // This method isn't strictly const, because it updates the cache,
  // but we pretend that it is because it's only a cache which is
  // transparent to the rest of the interface.

  // We handle identity as a trivial special case.
  if (is_identity()) {
    return other;
  }
  // Unlike compose(), the case of other->is_identity() is not quite as
  // trivial for invert_compose().

  // If either transform is invalid, the result is invalid.
  if (is_invalid()) {
    return this;
  }
  if (other->is_invalid()) {
    return other;
  }

  if (other == this) {
    // a->invert_compose(a) always produces identity.
    return make_identity();
  }

  if (!transform_cache) {
    return do_invert_compose(other);
  }

  LightReMutexHolder holder(*_states_lock);

  CPT(TransformState) result;
  {
    LightReMutexHolder holder(*_states_lock);
    int index = _invert_composition_cache.find(other);
    if (index != -1) {
      const Composition &comp = _invert_composition_cache.get_data(index);
      result = comp._result;
    }
    if (result != (TransformState *)NULL) {
      _cache_stats.inc_hits();
    }
  }

  if (result != (TransformState *)NULL) {
    // Success!
    return result;
  }

  // Not in the cache.  Compute a new result.  It's important that we
  // don't hold the lock while we do this, or we lose the benefit of
  // parallelization.
  result = do_invert_compose(other);

  // It's OK to cast away the constness of this pointer, because the
  // cache is a transparent property of the class.
  return ((TransformState *)this)->store_invert_compose(other, result);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::unref
//       Access: Published, Virtual
//  Description: This method overrides ReferenceCount::unref() to
//               check whether the remaining reference count is
//               entirely in the cache, and if so, it checks for and
//               breaks a cycle in the cache involving this object.
//               This is designed to prevent leaks from cyclical
//               references within the cache.
////////////////////////////////////////////////////////////////////
bool TransformState::
unref() const {
  if (!transform_cache || garbage_collect_states) {
    // If we're not using the cache at all, or if we're relying on
    // garbage collection, just allow the pointer to unref normally.
    return ReferenceCount::unref();
  }

  // Here is the normal refcounting case, with a normal cache, and
  // without garbage collection in effect.  In this case we will pull
  // the object out of the cache when its reference count goes to 0.

  // We always have to grab the lock, since we will definitely need to
  // be holding it if we happen to drop the reference count to 0.
  // Having to grab the lock at every call to unref() is a big
  // limiting factor on parallelization.
  LightReMutexHolder holder(*_states_lock);

  if (auto_break_cycles && uniquify_transforms) {
    if (get_cache_ref_count() > 0 &&
        get_ref_count() == get_cache_ref_count() + 1) {
      // If we are about to remove the one reference that is not in the
      // cache, leaving only references in the cache, then we need to
      // check for a cycle involving this TransformState and break it if
      // it exists.
      ((TransformState *)this)->detect_and_break_cycles();
    }
  }

  if (ReferenceCount::unref()) {
    // The reference count is still nonzero.
    return true;
  }

  // The reference count has just reached zero.  Make sure the object
  // is removed from the global object pool, before anyone else finds
  // it and tries to ref it.
  ((TransformState *)this)->release_new();
  ((TransformState *)this)->remove_cache_pointers();
  
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::validate_composition_cache
//       Access: Published
//  Description: Returns true if the composition cache and invert
//               composition cache for this particular TransformState
//               are self-consistent and valid, false otherwise.
////////////////////////////////////////////////////////////////////
bool TransformState::
validate_composition_cache() const {
  LightReMutexHolder holder(*_states_lock);

  int size = _composition_cache.get_size();
  for (int i = 0; i < size; ++i) {
    if (!_composition_cache.has_element(i)) {
      continue;
    }
    const TransformState *source = _composition_cache.get_key(i);
    if (source != (TransformState *)NULL) {
      // Check that the source also has a pointer back to this one.  We
      // always add entries to the composition cache in pairs.
      int ri = source->_composition_cache.find(this);
      if (ri == -1) {
        // Failure!  There is no back-pointer.
        pgraph_cat.error()
          << "TransformState::composition cache is inconsistent!\n";
        pgraph_cat.error(false)
          << *this << " compose " << *source << "\n";
        pgraph_cat.error(false)
          << "but no reverse\n";
        return false;
      }
    }
  }

  size = _invert_composition_cache.get_size();
  for (int i = 0; i < size; ++i) {
    if (!_invert_composition_cache.has_element(i)) {
      continue;
    }
    const TransformState *source = _invert_composition_cache.get_key(i);
    if (source != (TransformState *)NULL) {
      // Check that the source also has a pointer back to this one.  We
      // always add entries to the composition cache in pairs.
      int ri = source->_invert_composition_cache.find(this);
      if (ri == -1) {
        // Failure!  There is no back-pointer.
        pgraph_cat.error()
          << "TransformState::invert composition cache is inconsistent!\n";
        pgraph_cat.error(false)
          << *this << " invert compose " << *source << "\n";
        pgraph_cat.error(false)
          << "but no reverse\n";
        return false;
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::output
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformState::
output(ostream &out) const {
  out << "T:";
  if (is_invalid()) {
    out << "(invalid)";

  } else if (is_identity()) {
    out << "(identity)";

  } else if (has_components()) {
    bool output_hpr = !get_hpr().almost_equal(LVecBase3(0.0f, 0.0f, 0.0f));

    if (!components_given()) {
      // A leading "m" indicates the transform was described as a full
      // matrix, and we are decomposing it for the benefit of the
      // user.
      out << "m";

    } else if (output_hpr && quat_given()) {
      // A leading "q" indicates that the pos, scale, and shear are
      // exactly as specified, but the rotation was described as a
      // quaternion, and we are decomposing that to hpr for the
      // benefit of the user.
      out << "q";
    }

    char lead = '(';
    if (is_2d()) {
      if (!get_pos2d().almost_equal(LVecBase2(0.0f, 0.0f))) {
        out << lead << "pos " << get_pos2d();
        lead = ' ';
      }
      if (output_hpr) {
        out << lead << "rotate " << get_rotate2d();
        lead = ' ';
      }
      if (!get_scale2d().almost_equal(LVecBase2(1.0f, 1.0f))) {
        if (has_uniform_scale()) {
          out << lead << "scale " << get_uniform_scale();
          lead = ' ';
        } else {
          out << lead << "scale " << get_scale2d();
          lead = ' ';
        }
      }
      if (has_nonzero_shear()) {
        out << lead << "shear " << get_shear2d();
        lead = ' ';
      }
    } else {
      if (!get_pos().almost_equal(LVecBase3(0.0f, 0.0f, 0.0f))) {
        out << lead << "pos " << get_pos();
        lead = ' ';
      }
      if (output_hpr) {
        out << lead << "hpr " << get_hpr();
        lead = ' ';
      }
      if (!get_scale().almost_equal(LVecBase3(1.0f, 1.0f, 1.0f))) {
        if (has_uniform_scale()) {
          out << lead << "scale " << get_uniform_scale();
          lead = ' ';
        } else {
          out << lead << "scale " << get_scale();
          lead = ' ';
        }
      }
      if (has_nonzero_shear()) {
        out << lead << "shear " << get_shear();
        lead = ' ';
      }
    }
    if (lead == '(') {
      out << "(almost identity)";
    } else {
      out << ")";
    }

  } else {
    if (is_2d()) {
      out << get_mat3();
    } else {
      out << get_mat();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::write
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformState::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::write_composition_cache
//       Access: Published
//  Description: Writes a brief description of the composition cache
//               and invert composition cache to the indicated
//               ostream.  This is not useful except for performance
//               analysis, to examine the cache structure.
////////////////////////////////////////////////////////////////////
void TransformState::
write_composition_cache(ostream &out, int indent_level) const {
  indent(out, indent_level + 2) << _composition_cache << "\n";
  indent(out, indent_level + 2) << _invert_composition_cache << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_num_states
//       Access: Published, Static
//  Description: Returns the total number of unique TransformState
//               objects allocated in the world.  This will go up and
//               down during normal operations.
////////////////////////////////////////////////////////////////////
int TransformState::
get_num_states() {
  if (_states == (States *)NULL) {
    return 0;
  }
  LightReMutexHolder holder(*_states_lock);
  return _states->get_num_entries();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::get_num_unused_states
//       Access: Published, Static
//  Description: Returns the total number of TransformState objects that
//               have been allocated but have no references outside of
//               the internal TransformState cache.
//
//               A nonzero return value is not necessarily indicative
//               of leaked references; it is normal for two
//               TransformState objects, both of which have references
//               held outside the cache, to have the result of their
//               composition stored within the cache.  This result
//               will be retained within the cache until one of the
//               base TransformStates is released.
//
//               Use list_cycles() to get an idea of the number of
//               actual "leaked" TransformState objects.
////////////////////////////////////////////////////////////////////
int TransformState::
get_num_unused_states() {
  if (_states == (States *)NULL) {
    return 0;
  }
  LightReMutexHolder holder(*_states_lock);

  // First, we need to count the number of times each TransformState
  // object is recorded in the cache.  We could just trust
  // get_cache_ref_count(), but we'll be extra cautious for now.
  typedef pmap<const TransformState *, int> StateCount;
  StateCount state_count;

  int size = _states->get_size();
  for (int si = 0; si < size; ++si) {
    if (!_states->has_element(si)) {
      continue;
    }
    const TransformState *state = _states->get_key(si);

    int i;
    int cache_size = state->_composition_cache.get_size();
    for (i = 0; i < cache_size; ++i) {
      if (state->_composition_cache.has_element(i)) {
        const TransformState *result = state->_composition_cache.get_data(i)._result;
        if (result != (const TransformState *)NULL && result != state) {
          // Here's a TransformState that's recorded in the cache.
          // Count it.
          pair<StateCount::iterator, bool> ir =
            state_count.insert(StateCount::value_type(result, 1));
          if (!ir.second) {
            // If the above insert operation fails, then it's already in
            // the cache; increment its value.
            (*(ir.first)).second++;
          }
        }
      }
    }
    cache_size = state->_invert_composition_cache.get_size();
    for (i = 0; i < cache_size; ++i) {
      if (state->_invert_composition_cache.has_element(i)) {
        const TransformState *result = state->_invert_composition_cache.get_data(i)._result;
        if (result != (const TransformState *)NULL && result != state) {
          pair<StateCount::iterator, bool> ir =
            state_count.insert(StateCount::value_type(result, 1));
          if (!ir.second) {
            (*(ir.first)).second++;
          }
        }
      }
    }
  }

  // Now that we have the appearance count of each TransformState
  // object, we can tell which ones are unreferenced outside of the
  // TransformState cache, by comparing these to the reference counts.
  int num_unused = 0;

  StateCount::iterator sci;
  for (sci = state_count.begin(); sci != state_count.end(); ++sci) {
    const TransformState *state = (*sci).first;
    int count = (*sci).second;
    nassertr(count == state->get_cache_ref_count(), num_unused);
    nassertr(count <= state->get_ref_count(), num_unused);
    if (count == state->get_ref_count()) {
      num_unused++;

      if (pgraph_cat.is_debug()) {
        pgraph_cat.debug()
          << "Unused state: " << (void *)state << ":" 
          << state->get_ref_count() << " =\n";
        state->write(pgraph_cat.debug(false), 2);
      }
    }
  }

  return num_unused;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::clear_cache
//       Access: Published, Static
//  Description: Empties the cache of composed TransformStates.  This
//               makes every TransformState forget what results when
//               it is composed with other TransformStates.
//
//               This will eliminate any TransformState objects that
//               have been allocated but have no references outside of
//               the internal TransformState map.  It will not
//               eliminate TransformState objects that are still in
//               use.
//
//               Nowadays, this method should not be necessary, as
//               reference-count cycles in the composition cache
//               should be automatically detected and broken.
//
//               The return value is the number of TransformStates
//               freed by this operation.
////////////////////////////////////////////////////////////////////
int TransformState::
clear_cache() {
  if (_states == (States *)NULL) {
    return 0;
  }
  LightReMutexHolder holder(*_states_lock);

  PStatTimer timer(_cache_update_pcollector);
  int orig_size = _states->get_num_entries();

  // First, we need to copy the entire set of states to a temporary
  // vector, reference-counting each object.  That way we can walk
  // through the copy, without fear of dereferencing (and deleting)
  // the objects in the map as we go.
  {
    typedef pvector< CPT(TransformState) > TempStates;
    TempStates temp_states;
    temp_states.reserve(orig_size);

    int size = _states->get_size();
    for (int si = 0; si < size; ++si) {
      if (!_states->has_element(si)) {
        continue;
      }
      const TransformState *state = _states->get_key(si);
      temp_states.push_back(state);
    }

    // Now it's safe to walk through the list, destroying the cache
    // within each object as we go.  Nothing will be destructed till
    // we're done.
    TempStates::iterator ti;
    for (ti = temp_states.begin(); ti != temp_states.end(); ++ti) {
      TransformState *state = (TransformState *)(*ti).p();

      int i;
      int cache_size = state->_composition_cache.get_size();
      for (i = 0; i < cache_size; ++i) {
        if (state->_composition_cache.has_element(i)) {
          const TransformState *result = state->_composition_cache.get_data(i)._result;
          if (result != (const TransformState *)NULL && result != state) {
            result->cache_unref();
            nassertr(result->get_ref_count() > 0, 0);
          }
        }
      }
      _cache_stats.add_total_size(-state->_composition_cache.get_num_entries());
      state->_composition_cache.clear();

      cache_size = state->_invert_composition_cache.get_size();
      for (i = 0; i < cache_size; ++i) {
        if (state->_invert_composition_cache.has_element(i)) {
          const TransformState *result = state->_invert_composition_cache.get_data(i)._result;
          if (result != (const TransformState *)NULL && result != state) {
            result->cache_unref();
            nassertr(result->get_ref_count() > 0, 0);
          }
        }
      }
      _cache_stats.add_total_size(-state->_invert_composition_cache.get_num_entries());
      state->_invert_composition_cache.clear();
    }

    // Once this block closes and the temp_states object goes away,
    // all the destruction will begin.  Anything whose reference was
    // held only within the various objects' caches will go away.
  }

  int new_size = _states->get_num_entries();
  return orig_size - new_size;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::garbage_collect
//       Access: Published, Static
//  Description: Performs a garbage-collection cycle.  This must be
//               called periodically if garbage-collect-states is true
//               to ensure that TransformStates get cleaned up
//               appropriately.  It does no harm to call it even if
//               this variable is not true, but there is probably no
//               advantage in that case.
////////////////////////////////////////////////////////////////////
int TransformState::
garbage_collect() {
  if (_states == (States *)NULL || !garbage_collect_states) {
    return 0;
  }
  LightReMutexHolder holder(*_states_lock);

  PStatTimer timer(_garbage_collect_pcollector);
  int orig_size = _states->get_num_entries();

  // How many elements to process this pass?
  int size = _states->get_size();
  int num_this_pass = int(size * garbage_collect_states_rate);
  if (num_this_pass <= 0) {
    return 0;
  }
  num_this_pass = min(num_this_pass, size);
  int stop_at_element = (_garbage_index + num_this_pass) % size;
  
  int num_elements = 0;
  int si = _garbage_index;
  do {
    if (_states->has_element(si)) {
      ++num_elements;
      TransformState *state = (TransformState *)_states->get_key(si);
      if (auto_break_cycles && uniquify_transforms) {
        if (state->get_cache_ref_count() > 0 &&
            state->get_ref_count() == state->get_cache_ref_count()) {
          // If we have removed all the references to this state not in
          // the cache, leaving only references in the cache, then we
          // need to check for a cycle involving this TransformState and
          // break it if it exists.
          state->detect_and_break_cycles();
        }
      }

      if (state->get_ref_count() == 1) {
        // This state has recently been unreffed to 1 (the one we
        // added when we stored it in the cache).  Now it's time to
        // delete it.  This is safe, because we're holding the
        // _states_lock, so it's not possible for some other thread to
        // find the state in the cache and ref it while we're doing
        // this.
        state->release_new();
        state->remove_cache_pointers();
        state->cache_unref();
        delete state;
      }
    }      
    
    si = (si + 1) % size;
  } while (si != stop_at_element);
  _garbage_index = si;
  nassertr(_states->validate(), 0);

  int new_size = _states->get_num_entries();
  return orig_size - new_size;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::list_cycles
//       Access: Published, Static
//  Description: Detects all of the reference-count cycles in the
//               cache and reports them to standard output.
//
//               These cycles may be inadvertently created when state
//               compositions cycle back to a starting point.
//               Nowadays, these cycles should be automatically
//               detected and broken, so this method should never list
//               any cycles unless there is a bug in that detection
//               logic.
//
//               The cycles listed here are not leaks in the strictest
//               sense of the word, since they can be reclaimed by a
//               call to clear_cache(); but they will not be reclaimed
//               automatically.
////////////////////////////////////////////////////////////////////
void TransformState::
list_cycles(ostream &out) {
  if (_states == (States *)NULL) {
    return;
  }
  LightReMutexHolder holder(*_states_lock);

  typedef pset<const TransformState *> VisitedStates;
  VisitedStates visited;
  CompositionCycleDesc cycle_desc;

  int size = _states->get_size();
  for (int si = 0; si < size; ++si) {
    if (!_states->has_element(si)) {
      continue;
    }
    const TransformState *state = _states->get_key(si);

    bool inserted = visited.insert(state).second;
    if (inserted) {
      ++_last_cycle_detect;
      if (r_detect_cycles(state, state, 1, _last_cycle_detect, &cycle_desc)) {
        // This state begins a cycle.
        CompositionCycleDesc::reverse_iterator csi;

        out << "\nCycle detected of length " << cycle_desc.size() + 1 << ":\n"
            << "state " << (void *)state << ":" << state->get_ref_count()
            << " =\n";
        state->write(out, 2);
        for (csi = cycle_desc.rbegin(); csi != cycle_desc.rend(); ++csi) {
          const CompositionCycleDescEntry &entry = (*csi);
          if (entry._inverted) {
            out << "invert composed with ";
          } else {
            out << "composed with ";
          }
          out << (const void *)entry._obj << ":" << entry._obj->get_ref_count()
              << " " << *entry._obj << "\n"
              << "produces " << (const void *)entry._result << ":"
              << entry._result->get_ref_count() << " =\n";
          entry._result->write(out, 2);
          visited.insert(entry._result);
        }

        cycle_desc.clear();
      } else {
        ++_last_cycle_detect;
        if (r_detect_reverse_cycles(state, state, 1, _last_cycle_detect, &cycle_desc)) {
          // This state begins a cycle.
          CompositionCycleDesc::iterator csi;
          
          out << "\nReverse cycle detected of length " << cycle_desc.size() + 1 << ":\n"
              << "state ";
          for (csi = cycle_desc.begin(); csi != cycle_desc.end(); ++csi) {
            const CompositionCycleDescEntry &entry = (*csi);
            out << (const void *)entry._result << ":"
                << entry._result->get_ref_count() << " =\n";
            entry._result->write(out, 2);
            out << (const void *)entry._obj << ":"
                << entry._obj->get_ref_count() << " =\n";
            entry._obj->write(out, 2);
            visited.insert(entry._result);
          }
          out << (void *)state << ":"
              << state->get_ref_count() << " =\n";
          state->write(out, 2);
          
          cycle_desc.clear();
        }
      }
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: TransformState::list_states
//       Access: Published, Static
//  Description: Lists all of the TransformStates in the cache to the
//               output stream, one per line.  This can be quite a lot
//               of output if the cache is large, so be prepared.
////////////////////////////////////////////////////////////////////
void TransformState::
list_states(ostream &out) {
  if (_states == (States *)NULL) {
    out << "0 states:\n";
    return;
  }
  LightReMutexHolder holder(*_states_lock);

  out << _states->get_num_entries() << " states:\n";

  int size = _states->get_size();
  for (int si = 0; si < size; ++si) {
    if (!_states->has_element(si)) {
      continue;
    }
    const TransformState *state = _states->get_key(si);
    state->write(out, 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::validate_states
//       Access: Published, Static
//  Description: Ensures that the cache is still stored in sorted
//               order, and that none of the cache elements have been
//               inadvertently deleted.  Returns true if so, false if
//               there is a problem (which implies someone has
//               modified one of the supposedly-const TransformState
//               objects).
////////////////////////////////////////////////////////////////////
bool TransformState::
validate_states() {
  if (_states == (States *)NULL) {
    return true;
  }

  PStatTimer timer(_transform_validate_pcollector);

  LightReMutexHolder holder(*_states_lock);
  if (_states->is_empty()) {
    return true;
  }

  if (!_states->validate()) {
    pgraph_cat.error()
      << "TransformState::_states cache is invalid!\n";
    return false;
  }    

  int size = _states->get_size();
  int si = 0;
  while (si < size && !_states->has_element(si)) {
    ++si;
  }
  nassertr(si < size, false);
  nassertr(_states->get_key(si)->get_ref_count() >= 0, false);
  int snext = si;
  ++snext;
  while (snext < size && !_states->has_element(snext)) {
    ++snext;
  }
  while (snext < size) {
    nassertr(_states->get_key(snext)->get_ref_count() >= 0, false);
    const TransformState *ssi = _states->get_key(si);
    if (!ssi->validate_composition_cache()) {
      return false;
    }
    const TransformState *ssnext = _states->get_key(snext);
    int c = ssi->compare_to(*ssnext);
    int ci = ssnext->compare_to(*ssi);
    if ((ci < 0) != (c > 0) ||
        (ci > 0) != (c < 0) ||
        (ci == 0) != (c == 0)) {
      pgraph_cat.error()
        << "TransformState::compare_to() not defined properly!\n";
      pgraph_cat.error(false)
        << "(a, b): " << c << "\n";
      pgraph_cat.error(false)
        << "(b, a): " << ci << "\n";
      ssi->write(pgraph_cat.error(false), 2);
      ssnext->write(pgraph_cat.error(false), 2);
      return false;
    }
    si = snext;
    ++snext;
    while (snext < size && !_states->has_element(snext)) {
      ++snext;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::init_states
//       Access: Public, Static
//  Description: Make sure the global _states map is allocated.  This
//               only has to be done once.  We could make this map
//               static, but then we run into problems if anyone
//               creates a TransformState object at static init time;
//               it also seems to cause problems when the Panda shared
//               library is unloaded at application exit time.
////////////////////////////////////////////////////////////////////
void TransformState::
init_states() {
  _states = new States;

  // TODO: we should have a global Panda mutex to allow us to safely
  // create _states_lock without a startup race condition.  For the
  // meantime, this is OK because we guarantee that this method is
  // called at static init time, presumably when there is still only
  // one thread in the world.
  _states_lock = new LightReMutex("TransformState::_states_lock");
  _cache_stats.init();
  nassertv(Thread::get_current_thread() == Thread::get_main_thread());
}
  
////////////////////////////////////////////////////////////////////
//     Function: TransformState::return_new
//       Access: Private, Static
//  Description: This function is used to share a common TransformState
//               pointer for all equivalent TransformState objects.
//
//               This is different from return_unique() in that it
//               does not actually guarantee a unique pointer, unless
//               uniquify-transforms is set.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
return_new(TransformState *state) {
  nassertr(state != (TransformState *)NULL, state);
  if (!uniquify_transforms && !state->is_identity()) {
    return state;
  }

  return return_unique(state);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::return_unique
//       Access: Private, Static
//  Description: This function is used to share a common TransformState
//               pointer for all equivalent TransformState objects.
//
//               See the similar logic in RenderState.  The idea is to
//               create a new TransformState object and pass it
//               through this function, which will share the pointer
//               with a previously-created TransformState object if it
//               is equivalent.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
return_unique(TransformState *state) {
  nassertr(state != (TransformState *)NULL, state);

  if (!transform_cache) {
    return state;
  }

#ifndef NDEBUG
  if (paranoid_const) {
    nassertr(validate_states(), state);
  }
#endif

  PStatTimer timer(_transform_new_pcollector);

  LightReMutexHolder holder(*_states_lock);

  if (state->_saved_entry != -1) {
    // This state is already in the cache.
    //nassertr(_states->find(state) == state->_saved_entry, state);
    return state;
  }

  // Save the state in a local PointerTo so that it will be freed at
  // the end of this function if no one else uses it.
  CPT(TransformState) pt_state = state;

  int si = _states->find(state);
  if (si != -1) {
    // There's an equivalent state already in the set.  Return it.
    return _states->get_key(si);
  }

  // Not already in the set; add it.
  if (garbage_collect_states) {
    // If we'll be garbage collecting states explicitly, we'll
    // increment the reference count when we store it in the cache, so
    // that it won't be deleted while it's in it.
    state->cache_ref();
  }
  si = _states->store(state, Empty());

  // Save the index and return the input state.
  state->_saved_entry = si;
  return pt_state;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::do_compose
//       Access: Private
//  Description: The private implemention of compose(); this actually
//               composes two TransformStates, without bothering with the
//               cache.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
do_compose(const TransformState *other) const {
  PStatTimer timer(_transform_compose_pcollector);

  nassertr((_flags & F_is_invalid) == 0, this);
  nassertr((other->_flags & F_is_invalid) == 0, other);

  if (compose_componentwise && 
      has_uniform_scale() && 
      !has_nonzero_shear() && !other->has_nonzero_shear() &&
      ((components_given() && other->has_components()) ||
       (other->components_given() && has_components()))) {
    // We will do this operation componentwise if *either* transform
    // was given componentwise (and there is no non-uniform scale in
    // the way).

    CPT(TransformState) result;
    if (is_2d() && other->is_2d()) {
      // Do a 2-d compose.
      LVecBase2 pos = get_pos2d();
      PN_stdfloat rotate = get_rotate2d();
      LQuaternion quat = get_norm_quat();
      PN_stdfloat scale = get_uniform_scale();

      LPoint3 op = quat.xform(other->get_pos());
      pos += LVecBase2(op[0], op[1]) * scale;

      rotate += other->get_rotate2d();
      LVecBase2 new_scale = other->get_scale2d() * scale;
      
      result = make_pos_rotate_scale2d(pos, rotate, new_scale);

    } else {
      // A normal 3-d compose.
      LVecBase3 pos = get_pos();
      LQuaternion quat = get_norm_quat();
      PN_stdfloat scale = get_uniform_scale();
      
      pos += quat.xform(other->get_pos()) * scale;
      quat = other->get_norm_quat() * quat;
      LVecBase3 new_scale = other->get_scale() * scale;
      
      result = make_pos_quat_scale(pos, quat, new_scale);
    }
      
#ifndef NDEBUG
    if (paranoid_compose) {
      // Now verify against the matrix.
      LMatrix4 new_mat;
      new_mat.multiply(other->get_mat(), get_mat());
      if (!new_mat.almost_equal(result->get_mat(), 0.1)) {
        CPT(TransformState) correct = make_mat(new_mat);
        pgraph_cat.warning()
          << "Componentwise composition of " << *this << " and " << *other
          << " produced:\n"
          << *result << "\n  instead of:\n" << *correct << "\n";
        result = correct;
      }
    }
#endif  // NDEBUG

    return result;
  }

  // Do the operation with matrices.
  if (is_2d() && other->is_2d()) {
    LMatrix3 new_mat = other->get_mat3() * get_mat3();
    return make_mat3(new_mat);
  } else {
    LMatrix4 new_mat;
    new_mat.multiply(other->get_mat(), get_mat());
    return make_mat(new_mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::store_compose
//       Access: Private
//  Description: Stores the result of a composition in the cache.
//               Returns the stored result (it may be a different
//               object than the one passed in, due to another thread
//               having computed the composition first).
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
store_compose(const TransformState *other, const TransformState *result) {
  // Identity should have already been screened.
  nassertr(!is_identity(), other);
  nassertr(!other->is_identity(), this);

  // So should have validity.
  nassertr(!is_invalid(), this);
  nassertr(!other->is_invalid(), other);

  LightReMutexHolder holder(*_states_lock);

  // Is this composition already cached?
  int index = _composition_cache.find(other);
  if (index != -1) {
    Composition &comp = _composition_cache.modify_data(index);
    if (comp._result == (const TransformState *)NULL) {
      // Well, it wasn't cached already, but we already had an entry
      // (probably created for the reverse direction), so use the same
      // entry to store the new result.
      comp._result = result;

      if (result != (const TransformState *)this) {
        // See the comments below about the need to up the reference
        // count only when the result is not the same as this.
        result->cache_ref();
      }
    }
    // Here's the cache!
    _cache_stats.inc_hits();
    return comp._result;
  }
  _cache_stats.inc_misses();

  // We need to make a new cache entry, both in this object and in the
  // other object.  We make both records so the other TransformState
  // object will know to delete the entry from this object when it
  // destructs, and vice-versa.

  // The cache entry in this object is the only one that indicates the
  // result; the other will be NULL for now.
  _cache_stats.add_total_size(1);
  _cache_stats.inc_adds(_composition_cache.get_size() == 0);

  _composition_cache[other]._result = result;

  if (other != this) {
    _cache_stats.add_total_size(1);
    _cache_stats.inc_adds(other->_composition_cache.get_size() == 0);
    ((TransformState *)other)->_composition_cache[this]._result = NULL;
  }

  if (result != (TransformState *)this) {
    // If the result of do_compose() is something other than this,
    // explicitly increment the reference count.  We have to be sure
    // to decrement it again later, when the composition entry is
    // removed from the cache.
    result->cache_ref();
    
    // (If the result was just this again, we still store the
    // result, but we don't increment the reference count, since
    // that would be a self-referential leak.)
  }

  _cache_stats.maybe_report("TransformState");

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::store_invert_compose
//       Access: Private
//  Description: Stores the result of a composition in the cache.
//               Returns the stored result (it may be a different
//               object than the one passed in, due to another thread
//               having computed the composition first).
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
store_invert_compose(const TransformState *other, const TransformState *result) {
  // Identity should have already been screened.
  nassertr(!is_identity(), other);

  // So should have validity.
  nassertr(!is_invalid(), this);
  nassertr(!other->is_invalid(), other);

  nassertr(other != this, make_identity());

  LightReMutexHolder holder(*_states_lock);

  // Is this composition already cached?
  int index = _invert_composition_cache.find(other);
  if (index != -1) {
    Composition &comp = ((TransformState *)this)->_invert_composition_cache.modify_data(index);
    if (comp._result == (const TransformState *)NULL) {
      // Well, it wasn't cached already, but we already had an entry
      // (probably created for the reverse direction), so use the same
      // entry to store the new result.
      comp._result = result;

      if (result != (const TransformState *)this) {
        // See the comments below about the need to up the reference
        // count only when the result is not the same as this.
        result->cache_ref();
      }
    }
    // Here's the cache!
    _cache_stats.inc_hits();
    return comp._result;
  }
  _cache_stats.inc_misses();

  // We need to make a new cache entry, both in this object and in the
  // other object.  We make both records so the other TransformState
  // object will know to delete the entry from this object when it
  // destructs, and vice-versa.

  // The cache entry in this object is the only one that indicates the
  // result; the other will be NULL for now.
  _cache_stats.add_total_size(1);
  _cache_stats.inc_adds(_invert_composition_cache.get_size() == 0);
  _invert_composition_cache[other]._result = result;

  if (other != this) {
    _cache_stats.add_total_size(1);
    _cache_stats.inc_adds(other->_invert_composition_cache.get_size() == 0);
    ((TransformState *)other)->_invert_composition_cache[this]._result = NULL;
  }

  if (result != (TransformState *)this) {
    // If the result of compose() is something other than this,
    // explicitly increment the reference count.  We have to be sure
    // to decrement it again later, when the composition entry is
    // removed from the cache.
    result->cache_ref();
    
    // (If the result was just this again, we still store the
    // result, but we don't increment the reference count, since
    // that would be a self-referential leak.)
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::do_invert_compose
//       Access: Private
//  Description: The private implemention of invert_compose().
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
do_invert_compose(const TransformState *other) const {
  PStatTimer timer(_transform_invert_pcollector);

  nassertr((_flags & F_is_invalid) == 0, this);
  nassertr((other->_flags & F_is_invalid) == 0, other);

  if (compose_componentwise && 
      has_uniform_scale() && 
      !has_nonzero_shear() && !other->has_nonzero_shear() &&
      ((components_given() && other->has_components()) ||
       (other->components_given() && has_components()))) {
    // We will do this operation componentwise if *either* transform
    // was given componentwise (and there is no non-uniform scale in
    // the way).

    CPT(TransformState) result;
    if (is_2d() && other->is_2d()) {
      // Do a 2-d invert compose.
      LVecBase2 pos = get_pos2d();
      PN_stdfloat rotate = get_rotate2d();
      LQuaternion quat = get_norm_quat();
      PN_stdfloat scale = get_uniform_scale();
      
      // First, invert our own transform.
      if (scale == 0.0f) {
        ((TransformState *)this)->_flags |= F_is_singular | F_singular_known;
        return make_invalid();
      }
      scale = 1.0f / scale;
      quat.invert_in_place();
      rotate = -rotate;
      LVecBase3 mp = quat.xform(-LVecBase3(pos[0], pos[1], 0.0f));
      pos = LVecBase2(mp[0], mp[1]) * scale;
      LVecBase2 new_scale(scale, scale);
      
      // Now compose the inverted transform with the other transform.
      if (!other->is_identity()) {
        LPoint3 op = quat.xform(other->get_pos());
        pos += LVecBase2(op[0], op[1]) * scale;

        rotate += other->get_rotate2d();
        new_scale = other->get_scale2d() * scale;
      }

      result = make_pos_rotate_scale2d(pos, rotate, new_scale);

    } else {
      // Do a normal, 3-d invert compose.
      LVecBase3 pos = get_pos();
      LQuaternion quat = get_norm_quat();
      PN_stdfloat scale = get_uniform_scale();
      
      // First, invert our own transform.
      if (scale == 0.0f) {
        ((TransformState *)this)->_flags |= F_is_singular | F_singular_known;
        return make_invalid();
      }
      scale = 1.0f / scale;
      quat.invert_in_place();
      pos = quat.xform(-pos) * scale;
      LVecBase3 new_scale(scale, scale, scale);
      
      // Now compose the inverted transform with the other transform.
      if (!other->is_identity()) {
        pos += quat.xform(other->get_pos()) * scale;
        quat = other->get_norm_quat() * quat;
        new_scale = other->get_scale() * scale;
      }

      result = make_pos_quat_scale(pos, quat, new_scale);
    }

#ifndef NDEBUG
    if (paranoid_compose) {
      // Now verify against the matrix.
      if (is_singular()) {
        pgraph_cat.warning()
          << "Unexpected singular matrix found for " << *this << "\n";
      } else {
        nassertr(_inv_mat != (LMatrix4 *)NULL, make_invalid());
        LMatrix4 new_mat;
        new_mat.multiply(other->get_mat(), *_inv_mat);
        if (!new_mat.almost_equal(result->get_mat(), 0.1)) {
          CPT(TransformState) correct = make_mat(new_mat);
          pgraph_cat.warning()
            << "Componentwise invert-composition of " << *this << " and " << *other
            << " produced:\n"
            << *result << "\n  instead of:\n" << *correct << "\n";
          result = correct;
        }
      }
    }
#endif  // NDEBUG

    return result;
  }

  if (is_singular()) {
    return make_invalid();
  }

  // Now that is_singular() has returned false, we can assume that
  // _inv_mat has been allocated and filled in.
  nassertr(_inv_mat != (LMatrix4 *)NULL, make_invalid());

  if (is_2d() && other->is_2d()) {
    const LMatrix4 &i = *_inv_mat;
    LMatrix3 inv3(i(0, 0), i(0, 1), i(0, 3),
                  i(1, 0), i(1, 1), i(1, 3),
                  i(3, 0), i(3, 1), i(3, 3));
    if (other->is_identity()) {
      return make_mat3(inv3);
    } else {
      return make_mat3(other->get_mat3() * inv3);
    }
  } else {
    if (other->is_identity()) {
      return make_mat(*_inv_mat);
    } else {
      return make_mat(other->get_mat() * (*_inv_mat));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::detect_and_break_cycles
//       Access: Private
//  Description: Detects whether there is a cycle in the cache that
//               begins with this state.  If any are detected, breaks
//               them by removing this state from the cache.
////////////////////////////////////////////////////////////////////
void TransformState::
detect_and_break_cycles() {
  PStatTimer timer(_transform_break_cycles_pcollector);
  
  ++_last_cycle_detect;
  if (r_detect_cycles(this, this, 1, _last_cycle_detect, NULL)) {
    // Ok, we have a cycle.  This will be a leak unless we break the
    // cycle by freeing the cache on this object.
    if (pgraph_cat.is_debug()) {
      pgraph_cat.debug()
        << "Breaking cycle involving " << (*this) << "\n";
    }
    
    remove_cache_pointers();
  } else {
    ++_last_cycle_detect;
    if (r_detect_reverse_cycles(this, this, 1, _last_cycle_detect, NULL)) {
      if (pgraph_cat.is_debug()) {
        pgraph_cat.debug()
          << "Breaking cycle involving " << (*this) << "\n";
      }
      
      remove_cache_pointers();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::r_detect_cycles
//       Access: Private, Static
//  Description: Detects whether there is a cycle in the cache that
//               begins with the indicated state.  Returns true if at
//               least one cycle is found, false if this state is not
//               part of any cycles.  If a cycle is found and
//               cycle_desc is not NULL, then cycle_desc is filled in
//               with the list of the steps of the cycle, in reverse
//               order.
////////////////////////////////////////////////////////////////////
bool TransformState::
r_detect_cycles(const TransformState *start_state,
                const TransformState *current_state,
                int length, UpdateSeq this_seq,
                TransformState::CompositionCycleDesc *cycle_desc) {
  if (current_state->_cycle_detect == this_seq) {
    // We've already seen this state; therefore, we've found a cycle.

    // However, we only care about cycles that return to the starting
    // state and involve more than two steps.  If only one or two
    // nodes are involved, it doesn't represent a memory leak, so no
    // problem there.
    return (current_state == start_state && length > 2);
  }
  ((TransformState *)current_state)->_cycle_detect = this_seq;

  int i;
  int cache_size = current_state->_composition_cache.get_size();
  for (i = 0; i < cache_size; ++i) {
    if (current_state->_composition_cache.has_element(i)) {
      const TransformState *result = current_state->_composition_cache.get_data(i)._result;
      if (result != (const TransformState *)NULL) {
        if (r_detect_cycles(start_state, result, length + 1, 
                            this_seq, cycle_desc)) {
          // Cycle detected.
          if (cycle_desc != (CompositionCycleDesc *)NULL) {
            const TransformState *other = current_state->_composition_cache.get_key(i);
            CompositionCycleDescEntry entry(other, result, false);
            cycle_desc->push_back(entry);
          }
          return true;
        }
      }
    }
  }

  cache_size = current_state->_invert_composition_cache.get_size();
  for (i = 0; i < cache_size; ++i) {
    if (current_state->_invert_composition_cache.has_element(i)) {
      const TransformState *result = current_state->_invert_composition_cache.get_data(i)._result;
      if (result != (const TransformState *)NULL) {
        if (r_detect_cycles(start_state, result, length + 1,
                            this_seq, cycle_desc)) {
          // Cycle detected.
          if (cycle_desc != (CompositionCycleDesc *)NULL) {
            const TransformState *other = current_state->_invert_composition_cache.get_key(i);
            CompositionCycleDescEntry entry(other, result, true);
            cycle_desc->push_back(entry);
          }
          return true;
        }
      }
    }
  }

  // No cycle detected.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::r_detect_reverse_cycles
//       Access: Private, Static
//  Description: Works the same as r_detect_cycles, but checks for
//               cycles in the reverse direction along the cache
//               chain.  (A cycle may appear in either direction, and
//               we must check both.)
////////////////////////////////////////////////////////////////////
bool TransformState::
r_detect_reverse_cycles(const TransformState *start_state,
                        const TransformState *current_state,
                        int length, UpdateSeq this_seq,
                        TransformState::CompositionCycleDesc *cycle_desc) {
  if (current_state->_cycle_detect == this_seq) {
    // We've already seen this state; therefore, we've found a cycle.

    // However, we only care about cycles that return to the starting
    // state and involve more than two steps.  If only one or two
    // nodes are involved, it doesn't represent a memory leak, so no
    // problem there.
    return (current_state == start_state && length > 2);
  }
  ((TransformState *)current_state)->_cycle_detect = this_seq;

  int i;
  int cache_size = current_state->_composition_cache.get_size();
  for (i = 0; i < cache_size; ++i) {
    if (current_state->_composition_cache.has_element(i)) {
      const TransformState *other = current_state->_composition_cache.get_key(i);
      if (other != current_state) {
        int oi = other->_composition_cache.find(current_state);
        nassertr(oi != -1, false);

        const TransformState *result = other->_composition_cache.get_data(oi)._result;
        if (result != (const TransformState *)NULL) {
          if (r_detect_reverse_cycles(start_state, result, length + 1, 
                                      this_seq, cycle_desc)) {
            // Cycle detected.
            if (cycle_desc != (CompositionCycleDesc *)NULL) {
              const TransformState *other = current_state->_composition_cache.get_key(i);
              CompositionCycleDescEntry entry(other, result, false);
              cycle_desc->push_back(entry);
            }
            return true;
          }
        }
      }
    }
  }

  cache_size = current_state->_invert_composition_cache.get_size();
  for (i = 0; i < cache_size; ++i) {
    if (current_state->_invert_composition_cache.has_element(i)) {
      const TransformState *other = current_state->_invert_composition_cache.get_key(i);
      if (other != current_state) {
        int oi = other->_invert_composition_cache.find(current_state);
        nassertr(oi != -1, false);

        const TransformState *result = other->_invert_composition_cache.get_data(oi)._result;
        if (result != (const TransformState *)NULL) {
          if (r_detect_reverse_cycles(start_state, result, length + 1, 
                                      this_seq, cycle_desc)) {
            // Cycle detected.
            if (cycle_desc != (CompositionCycleDesc *)NULL) {
              const TransformState *other = current_state->_invert_composition_cache.get_key(i);
              CompositionCycleDescEntry entry(other, result, false);
              cycle_desc->push_back(entry);
            }
            return true;
          }
        }
      }
    }
  }

  // No cycle detected.
  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: TransformState::release_new
//       Access: Private
//  Description: This inverse of return_new, this releases this object
//               from the global TransformState table.
//
//               You must already be holding _states_lock before you
//               call this method.
////////////////////////////////////////////////////////////////////
void TransformState::
release_new() {
  nassertv(_states_lock->debug_is_locked());
   
  if (_saved_entry != -1) {
    //nassertv(_states->find(this) == _saved_entry);
    _saved_entry = _states->find(this);
    _states->remove_element(_saved_entry);
    _saved_entry = -1;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::remove_cache_pointers
//       Access: Private
//  Description: Remove all pointers within the cache from and to this
//               particular TransformState.  The pointers to this
//               object may be scattered around in the various
//               CompositionCaches from other TransformState objects.
//
//               You must already be holding _states_lock before you
//               call this method.
////////////////////////////////////////////////////////////////////
void TransformState::
remove_cache_pointers() {
  nassertv(_states_lock->debug_is_locked());
   
  // Fortunately, since we added CompositionCache records in pairs, we
  // know exactly the set of TransformState objects that have us in their
  // cache: it's the same set of TransformState objects that we have in
  // our own cache.

  // We do need to put considerable thought into this loop, because as
  // we clear out cache entries we'll cause other TransformState
  // objects to destruct, which could cause things to get pulled out
  // of our own _composition_cache map.  We want to allow this (so
  // that we don't encounter any just-destructed pointers in our
  // cache), but we don't want to get bitten by this cascading effect.
  // Instead of walking through the map from beginning to end,
  // therefore, we just pull out the first one each time, and erase
  // it.

#ifdef DO_PSTATS
  if (_composition_cache.is_empty() && _invert_composition_cache.is_empty()) {
    return;
  }
  PStatTimer timer(_cache_update_pcollector);
#endif  // DO_PSTATS

  // There are lots of ways to do this loop wrong.  Be very careful if
  // you need to modify it for any reason.
  int i = 0;
  while (!_composition_cache.is_empty()) {
    // Scan for the next used slot in the table.
    while (!_composition_cache.has_element(i)) {
      ++i;
    }

    // It is possible that the "other" TransformState object is
    // currently within its own destructor.  We therefore can't use a
    // PT() to hold its pointer; that could end up calling its
    // destructor twice.  Fortunately, we don't need to hold its
    // reference count to ensure it doesn't destruct while we process
    // this loop; as long as we ensure that no *other* TransformState
    // objects destruct, there will be no reason for that one to.
    TransformState *other = (TransformState *)_composition_cache.get_key(i);

    // We hold a copy of the composition result so we can dereference
    // it later.
    Composition comp = _composition_cache.get_data(i);

    // Now we can remove the element from our cache.  We do this now,
    // rather than later, before any other TransformState objects have
    // had a chance to destruct, so we are confident that our iterator
    // is still valid.
    _composition_cache.remove_element(i);
    _cache_stats.add_total_size(-1);
    _cache_stats.inc_dels();

    if (other != this) {
      int oi = other->_composition_cache.find(this);

      // We may or may not still be listed in the other's cache (it
      // might be halfway through pulling entries out, from within its
      // own destructor).
      if (oi != -1) {
        // Hold a copy of the other composition result, too.
        Composition ocomp = other->_composition_cache.get_data(oi);
        
        other->_composition_cache.remove_element(oi);
        _cache_stats.add_total_size(-1);
        _cache_stats.inc_dels();
        
        // It's finally safe to let our held pointers go away.  This may
        // have cascading effects as other TransformState objects are
        // destructed, but there will be no harm done if they destruct
        // now.
        if (ocomp._result != (const TransformState *)NULL && ocomp._result != other) {
          cache_unref_delete(ocomp._result);
        }
      }
    }

    // It's finally safe to let our held pointers go away.  (See
    // comment above.)
    if (comp._result != (const TransformState *)NULL && comp._result != this) {
      cache_unref_delete(comp._result);
    }
  }

  // A similar bit of code for the invert cache.
  i = 0;
  while (!_invert_composition_cache.is_empty()) {
    while (!_invert_composition_cache.has_element(i)) {
      ++i;
    }

    TransformState *other = (TransformState *)_invert_composition_cache.get_key(i);
    nassertv(other != this);
    Composition comp = _invert_composition_cache.get_data(i);
    _invert_composition_cache.remove_element(i);
    _cache_stats.add_total_size(-1);
    _cache_stats.inc_dels();
    if (other != this) {
      int oi = other->_invert_composition_cache.find(this);
      if (oi != -1) {
        Composition ocomp = other->_invert_composition_cache.get_data(oi);
        other->_invert_composition_cache.remove_element(oi);
        _cache_stats.add_total_size(-1);
        _cache_stats.inc_dels();
        if (ocomp._result != (const TransformState *)NULL && ocomp._result != other) {
          cache_unref_delete(ocomp._result);
        }
      }
    }
    if (comp._result != (const TransformState *)NULL && comp._result != this) {
      cache_unref_delete(comp._result);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::do_calc_hash
//       Access: Private
//  Description: Computes a suitable hash value for phash_map.
////////////////////////////////////////////////////////////////////
void TransformState::
do_calc_hash() {
  PStatTimer timer(_transform_hash_pcollector);
  _hash = 0;

  static const int significant_flags = 
    (F_is_invalid | F_is_identity | F_components_given | F_hpr_given | F_is_2d);

  int flags = (_flags & significant_flags);
  _hash = int_hash::add_hash(_hash, flags);

  if ((_flags & (F_is_invalid | F_is_identity)) == 0) {
    // Only bother to put the rest of the stuff in the hash if the
    // transform is not invalid or empty.
    
    if ((_flags & F_components_given) != 0) {
      // If the transform was specified componentwise, hash it
      // componentwise.
      _hash = _pos.add_hash(_hash);
      if ((_flags & F_hpr_given) != 0) {
        _hash = _hpr.add_hash(_hash);

      } else if ((_flags & F_quat_given) != 0) {
        _hash = _quat.add_hash(_hash);
      }

      _hash = _scale.add_hash(_hash);
      _hash = _shear.add_hash(_hash);

    } else {
      // Otherwise, hash the matrix . . .
      if (uniquify_matrix) {
        // . . . but only if the user thinks that's worthwhile.
        if ((_flags & F_mat_known) == 0) {
          // Calculate the matrix without doubly-locking.
          do_calc_mat();
        }
        _hash = _mat.add_hash(_hash);

      } else {
        // Otherwise, hash the pointer only--any two different
        // matrix-based TransformStates are considered to be different,
        // even if their matrices have the same values.

        _hash = pointer_hash::add_hash(_hash, this);
      }
    }
  }

  _flags |= F_hash_known;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::calc_singular
//       Access: Private
//  Description: Determines whether the transform is singular (i.e. it
//               scales to zero, and has no inverse).
////////////////////////////////////////////////////////////////////
void TransformState::
calc_singular() {
  LightMutexHolder holder(_lock);
  if ((_flags & F_singular_known) != 0) {
    // Someone else computed it first.
    return;
  }

  PStatTimer timer(_transform_calc_pcollector);

  nassertv((_flags & F_is_invalid) == 0);

  // We determine if a matrix is singular by attempting to invert it
  // (and we save the result of this invert operation for a subsequent
  // do_invert_compose() call, which is almost certain to be made if
  // someone is asking whether we're singular).

  // This should be NULL if no one has called calc_singular() yet.
  nassertv(_inv_mat == (LMatrix4 *)NULL);
  _inv_mat = new LMatrix4;

  if ((_flags & F_mat_known) == 0) {
    do_calc_mat();
  }
  bool inverted = _inv_mat->invert_from(_mat);

  if (!inverted) {
    _flags |= F_is_singular;
    delete _inv_mat;
    _inv_mat = (LMatrix4 *)NULL;
  }
  _flags |= F_singular_known;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::do_calc_components
//       Access: Private
//  Description: This is the implementation of calc_components(); it
//               assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void TransformState::
do_calc_components() {
  if ((_flags & F_components_known) != 0) {
    // Someone else computed it first.
    return;
  }

  PStatTimer timer(_transform_calc_pcollector);

  nassertv((_flags & F_is_invalid) == 0);
  if ((_flags & F_is_identity) != 0) {
    _scale.set(1.0f, 1.0f, 1.0f);
    _shear.set(0.0f, 0.0f, 0.0f);
    _hpr.set(0.0f, 0.0f, 0.0f);
    _quat = LQuaternion::ident_quat();
    _pos.set(0.0f, 0.0f, 0.0f);
    _flags |= F_has_components | F_components_known | F_hpr_known | F_quat_known | F_uniform_scale;

  } else {
    // If we don't have components and we're not identity, the only
    // other explanation is that we were constructed via a matrix.
    nassertv((_flags & F_mat_known) != 0);

    if ((_flags & F_mat_known) == 0) {
      do_calc_mat();
    }
    bool possible = decompose_matrix(_mat, _scale, _shear, _hpr, _pos);
    if (!possible) {
      // Some matrices can't be decomposed into scale, hpr, pos.  In
      // this case, we now know that we cannot compute the components;
      // but the closest approximations are stored, at least.
      _flags |= F_components_known | F_hpr_known;

    } else {
      // Otherwise, we do have the components, or at least the hpr.
      _flags |= F_has_components | F_components_known | F_hpr_known;
      check_uniform_scale();
    }

    // However, we can always get at least the pos.
    _mat.get_row3(_pos, 3);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::do_calc_hpr
//       Access: Private
//  Description: This is the implementation of calc_hpr(); it
//               assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void TransformState::
do_calc_hpr() {
  if ((_flags & F_hpr_known) != 0) {
    // Someone else computed it first.
    return;
  }

  PStatTimer timer(_transform_calc_pcollector);

  nassertv((_flags & F_is_invalid) == 0);
  if ((_flags & F_components_known) == 0) {
    do_calc_components();
  }
  if ((_flags & F_hpr_known) == 0) {
    // If we don't know the hpr yet, we must have been given a quat.
    // Decompose it.
    nassertv((_flags & F_quat_known) != 0);
    _hpr = _quat.get_hpr();
    _flags |= F_hpr_known;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::calc_quat
//       Access: Private
//  Description: Derives the quat from the hpr.
////////////////////////////////////////////////////////////////////
void TransformState::
calc_quat() {
  LightMutexHolder holder(_lock);
  if ((_flags & F_quat_known) != 0) {
    // Someone else computed it first.
    return;
  }

  PStatTimer timer(_transform_calc_pcollector);

  nassertv((_flags & F_is_invalid) == 0);
  if ((_flags & F_components_known) == 0) {
    do_calc_components();
  }
  if ((_flags & F_quat_known) == 0) {
    // If we don't know the quat yet, we must have been given a hpr.
    // Decompose it.
    nassertv((_flags & F_hpr_known) != 0);
    _quat.set_hpr(_hpr);
    _flags |= F_quat_known;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::calc_norm_quat
//       Access: Private
//  Description: Derives the normalized quat from the quat.
////////////////////////////////////////////////////////////////////
void TransformState::
calc_norm_quat() {
  PStatTimer timer(_transform_calc_pcollector);

  LQuaternion quat = get_quat();
  LightMutexHolder holder(_lock);
  _norm_quat = quat;
  _norm_quat.normalize();
  _flags |= F_norm_quat_known;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::do_calc_mat
//       Access: Private
//  Description: This is the implementation of calc_mat(); it
//               assumes the lock is already held.
////////////////////////////////////////////////////////////////////
void TransformState::
do_calc_mat() {
  if ((_flags & F_mat_known) != 0) {
    // Someone else computed it first.
    return;
  }

  PStatTimer timer(_transform_calc_pcollector);

  nassertv((_flags & F_is_invalid) == 0);
  if ((_flags & F_is_identity) != 0) {
    _mat = LMatrix4::ident_mat();

  } else {
    // If we don't have a matrix and we're not identity, the only
    // other explanation is that we were constructed via components.
    nassertv((_flags & F_components_known) != 0);
    if ((_flags & F_hpr_known) == 0) {
      do_calc_hpr();
    }

    compose_matrix(_mat, _scale, _shear, get_hpr(), _pos);
  }
  _flags |= F_mat_known;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::update_pstats
//       Access: Private
//  Description: Moves the TransformState object from one PStats category
//               to another, so that we can track in PStats how many
//               pointers are held by nodes, and how many are held in
//               the cache only.
////////////////////////////////////////////////////////////////////
void TransformState::
update_pstats(int old_referenced_bits, int new_referenced_bits) {
#ifdef DO_PSTATS
  if ((old_referenced_bits & R_node) != 0) {
    _node_counter.sub_level(1);
  } else if ((old_referenced_bits & R_cache) != 0) {
    _cache_counter.sub_level(1);
  }
  if ((new_referenced_bits & R_node) != 0) {
    _node_counter.add_level(1);
  } else if ((new_referenced_bits & R_cache) != 0) {
    _cache_counter.add_level(1);
  }
#endif  // DO_PSTATS
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TransformState.
////////////////////////////////////////////////////////////////////
void TransformState::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TransformState::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  if ((_flags & F_is_identity) != 0) {
    // Identity, nothing much to that.
    int flags = F_is_identity | F_singular_known | F_is_2d;
    dg.add_uint32(flags);

  } else if ((_flags & F_is_invalid) != 0) {
    // Invalid, nothing much to that either.
    int flags = F_is_invalid | F_singular_known | F_is_singular | F_components_known | F_mat_known;
    dg.add_uint32(flags);

  } else if ((_flags & F_components_given) != 0) {
    // A component-based transform.
    int flags = F_components_given | F_components_known | F_has_components;
    flags |= (_flags & F_is_2d);
    if ((_flags & F_quat_given) != 0) {
      flags |= (F_quat_given | F_quat_known);
    } else if ((_flags & F_hpr_given) != 0) {
      flags |= (F_hpr_given | F_hpr_known);
    }

    dg.add_uint32(flags);

    _pos.write_datagram(dg);
    if ((_flags & F_quat_given) != 0) {
      _quat.write_datagram(dg);
    } else {
      get_hpr().write_datagram(dg);
    }
    _scale.write_datagram(dg);
    _shear.write_datagram(dg);

  } else {
    // A general matrix.
    nassertv((_flags & F_mat_known) != 0);
    int flags = F_mat_known;
    flags |= (_flags & F_is_2d);
    dg.add_uint32(flags);
    _mat.write_datagram(dg);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::change_this
//       Access: Public, Static
//  Description: Called immediately after complete_pointers(), this
//               gives the object a chance to adjust its own pointer
//               if desired.  Most objects don't change pointers after
//               completion, but some need to.
//
//               Once this function has been called, the old pointer
//               will no longer be accessed.
////////////////////////////////////////////////////////////////////
PT(TypedWritableReferenceCount) TransformState::
change_this(TypedWritableReferenceCount *old_ptr, BamReader *manager) {
  // First, uniquify the pointer.
  TransformState *state = DCAST(TransformState, old_ptr);
  CPT(TransformState) pointer = return_unique(state);
  
  // We have to cast the pointer back to non-const, because the bam
  // reader expects that.
  return (TransformState *)pointer.p();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TransformState is encountered
//               in the Bam file.  It should create the TransformState
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TransformState::
make_from_bam(const FactoryParams &params) {
  TransformState *state = new TransformState;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  state->fillin(scan, manager);
  manager->register_change_this(change_this, state);

  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TransformState.
////////////////////////////////////////////////////////////////////
void TransformState::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  _flags = scan.get_uint32();

  if ((_flags & F_components_given) != 0) {
    // Componentwise transform.
    _pos.read_datagram(scan);
    if ((_flags & F_quat_given) != 0) {
      _quat.read_datagram(scan);
    } else {
      _hpr.read_datagram(scan);
    }
    _scale.read_datagram(scan);
    _shear.read_datagram(scan);

    check_uniform_scale();
  }

  if ((_flags & F_mat_known) != 0) {
    // General matrix.
    _mat.read_datagram(scan);
  }
}
