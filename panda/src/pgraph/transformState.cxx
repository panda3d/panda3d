// Filename: transformState.cxx
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

#include "transformState.h"
#include "compose_matrix.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagramIterator.h"
#include "indent.h"
#include "compareTo.h"

TransformState::States *TransformState::_states = NULL;
CPT(TransformState) TransformState::_identity_state;
TypeHandle TransformState::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TransformState::Constructor
//       Access: Protected
//  Description: Actually, this could be a private constructor, since
//               no one inherits from TransformState, but gcc gives us a
//               spurious warning if all constructors are private.
////////////////////////////////////////////////////////////////////
TransformState::
TransformState() {
  if (_states == (States *)NULL) {
    // Make sure the global _states map is allocated.  This only has
    // to be done once.  We could make this map static, but then we
    // run into problems if anyone creates a RenderState object at
    // static init time; it also seems to cause problems when the
    // Panda shared library is unloaded at application exit time.
    _states = new States;
  }
  _saved_entry = _states->end();
  _self_compose = (TransformState *)NULL;
  _flags = F_is_identity | F_singular_known;
  _inv_mat = (LMatrix4f *)NULL;
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
  if (_inv_mat != (LMatrix4f *)NULL) {
    delete _inv_mat;
  }

  // Remove the deleted TransformState object from the global pool.
  if (_saved_entry != _states->end()) {
    nassertv(_states->find(this) == _saved_entry);
    _states->erase(_saved_entry);
    _saved_entry = _states->end();
  }

  // Now make sure we clean up all other floating pointers to the
  // TransformState.  These may be scattered around in the various
  // CompositionCaches from other TransformState objects.

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

  // There are lots of ways to do this loop wrong.  Be very careful if
  // you need to modify it for any reason.
  while (!_composition_cache.empty()) {
    CompositionCache::iterator ci = _composition_cache.begin();

    // It is possible that the "other" TransformState object is
    // currently within its own destructor.  We therefore can't use a
    // PT() to hold its pointer; that could end up calling its
    // destructor twice.  Fortunately, we don't need to hold its
    // reference count to ensure it doesn't destruct while we process
    // this loop; as long as we ensure that no *other* TransformState
    // objects destruct, there will be no reason for that one to.
    TransformState *other = (TransformState *)(*ci).first;

    // We should never have a reflexive entry in this map.  If we
    // do, something got screwed up elsewhere.
    nassertv(other != this);

    // We hold a copy of the composition result to ensure that the
    // result TransformState object (if there is one) doesn't
    // destruct.
    Composition comp = (*ci).second;

    // Now we can remove the element from our cache.  We do this now,
    // rather than later, before any other TransformState objects have
    // had a chance to destruct, so we are confident that our iterator
    // is still valid.
    _composition_cache.erase(ci);

    CompositionCache::iterator oci = other->_composition_cache.find(this);

    // We may or may not still be listed in the other's cache (it
    // might be halfway through pulling entries out, from within its
    // own destructor).
    if (oci != other->_composition_cache.end()) {
      // Hold a copy of the other composition result, too.
      Composition ocomp = (*oci).second;
      
      // Now we're holding a reference count to both computed
      // results, so no objects will be tempted to destruct while we
      // erase the other cache entry.
      other->_composition_cache.erase(oci);
    }

    // It's finally safe to let our held pointers go away.  This may
    // have cascading effects as other TransformState objects are
    // destructed, but there will be no harm done if they destruct
    // now.
  }

  // A similar bit of code for the invert cache.
  while (!_invert_composition_cache.empty()) {
    CompositionCache::iterator ci = _invert_composition_cache.begin();
    TransformState *other = (TransformState *)(*ci).first;
    nassertv(other != this);
    Composition comp = (*ci).second;
    _invert_composition_cache.erase(ci);
    CompositionCache::iterator oci = 
      other->_invert_composition_cache.find(this);
    if (oci != other->_invert_composition_cache.end()) {
      Composition ocomp = (*oci).second;
      other->_invert_composition_cache.erase(oci);
    }
  }

  // Also, if we called compose(this) at some point and the return
  // value was something other than this, we need to decrement the
  // associated reference count.
  if (_self_compose != (TransformState *)NULL && _self_compose != this) {
    unref_delete((TransformState *)_self_compose);
  }

  // If this was true at the beginning of the destructor, but is no
  // longer true now, probably we've been double-deleted.
  nassertv(get_ref_count() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::operator <
//       Access: Public
//  Description: Provides an arbitrary ordering among all unique
//               TransformStates, so we can store the essentially
//               different ones in a big set and throw away the rest.
//
//               This method is not needed outside of the TransformState
//               class because all equivalent TransformState objects are
//               guaranteed to share the same pointer; thus, a pointer
//               comparison is always sufficient.
////////////////////////////////////////////////////////////////////
bool TransformState::
operator < (const TransformState &other) const {
  static const int significant_flags = 
    (F_is_invalid | F_is_identity | F_components_given);

  int flags = (_flags & significant_flags);
  int other_flags = (other._flags & significant_flags);
  if (flags != other_flags) {
    return flags < other_flags;
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
      return c < 0;
    }
    c = _hpr.compare_to(other._hpr);
    if (c != 0) {
      return c < 0;
    }
    c = _scale.compare_to(other._scale);
    return c < 0;
  }

  // Otherwise, compare the matrices.
  return get_mat() < other.get_mat();
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
    _identity_state = return_new(state);
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
  TransformState *state = new TransformState;
  state->_flags = F_is_invalid | F_singular_known | F_is_singular | F_components_known | F_mat_known;
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_pos_hpr_scale
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               components.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
make_pos_hpr_scale(const LVecBase3f &pos, const LVecBase3f &hpr, 
                   const LVecBase3f &scale) {
  // Make a special-case check for the identity transform.
  if (pos == LVecBase3f(0.0f, 0.0f, 0.0f) &&
      hpr == LVecBase3f(0.0f, 0.0f, 0.0f) &&
      scale == LVecBase3f(1.0f, 1.0f, 1.0f)) {
    return make_identity();
  }

  TransformState *state = new TransformState;
  state->_pos = pos;
  state->_hpr = hpr;
  state->_scale = scale;
  state->_flags = F_components_given | F_components_known | F_has_components;
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::make_mat
//       Access: Published, Static
//  Description: Makes a new TransformState with the specified
//               transformation matrix.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
make_mat(const LMatrix4f &mat) {
  // Make a special-case check for the identity matrix.
  if (mat == LMatrix4f::ident_mat()) {
    return make_identity();
  }

  TransformState *state = new TransformState;
  state->_mat = mat;
  state->_flags = F_mat_known;
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
set_pos(const LVecBase3f &pos) const {
  if ((_flags & F_components_given) != 0) {
    // If we started with a componentwise transform, we keep it that
    // way.
    return make_pos_hpr_scale(pos, get_hpr(), get_scale());

  } else {
    // Otherwise, we have a matrix transform, and we keep it that way.
    LMatrix4f mat = get_mat();
    mat.set_row(3, pos);
    return make_mat(mat);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_hpr
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original TransformState with its hpr component
//               replaced with the indicated value, if possible.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_hpr(const LVecBase3f &hpr) const {
  nassertr(has_components(), this);
  return make_pos_hpr_scale(get_pos(), hpr, get_scale());
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::set_scale
//       Access: Published
//  Description: Returns a new TransformState object that represents the
//               original TransformState with its scale component
//               replaced with the indicated value, if possible.
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
set_scale(const LVecBase3f &scale) const {
  nassertr(has_components(), this);
  return make_pos_hpr_scale(get_pos(), get_hpr(), scale);
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
  // This method isn't strictly const, because it updates the cache,
  // but we pretend that it is because it's only a cache which is
  // transparent to the rest of the interface.

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

  if (other == this) {
    // compose(this) has to be handled as a special case, because the
    // caching problem is so different.
    if (_self_compose != (TransformState *)NULL) {
      return _self_compose;
    }
    CPT(TransformState) result = do_compose(this);
    ((TransformState *)this)->_self_compose = result;

    if (result != (const TransformState *)this) {
      // If the result of compose(this) is something other than this,
      // explicitly increment the reference count.  We have to be sure
      // to decrement it again later, in our destructor.
      _self_compose->ref();

      // (If the result was just this again, we still store the
      // result, but we don't increment the reference count, since
      // that would be a self-referential leak.  What a mess this is.)
    }
    return _self_compose;
  }

  // Is this composition already cached?
  CompositionCache::const_iterator ci = _composition_cache.find(other);
  if (ci != _composition_cache.end()) {
    const Composition &comp = (*ci).second;
    if (comp._result == (const TransformState *)NULL) {
      // Well, it wasn't cached already, but we already had an entry
      // (probably created for the reverse direction), so use the same
      // entry to store the new result.
      ((Composition &)comp)._result = do_compose(other);
    }
    // Here's the cache!
    return comp._result;
  }

  // We need to make a new cache entry, both in this object and in the
  // other object.  We make both records so the other TransformState
  // object will know to delete the entry from this object when it
  // destructs, and vice-versa.

  // The cache entry in this object is the only one that indicates the
  // result; the other will be NULL for now.
  CPT(TransformState) result = do_compose(other);
  // We store them in this order, on the off-chance that other is the
  // same as this, a degenerate case which is still worth supporting.
  ((TransformState *)other)->_composition_cache[this]._result = NULL;
  ((TransformState *)this)->_composition_cache[other]._result = result;

  return result;
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

  // Is this composition already cached?
  CompositionCache::const_iterator ci = _invert_composition_cache.find(other);
  if (ci != _invert_composition_cache.end()) {
    const Composition &comp = (*ci).second;
    if (comp._result == (const TransformState *)NULL) {
      // Well, it wasn't cached already, but we already had an entry
      // (probably created for the reverse direction), so use the same
      // entry to store the new result.
      ((Composition &)comp)._result = do_invert_compose(other);
    }
    // Here's the cache!
    return comp._result;
  }

  // We need to make a new cache entry, both in this object and in the
  // other object.  We make both records so the other TransformState
  // object will know to delete the entry from this object when it
  // destructs, and vice-versa.

  // The cache entry in this object is the only one that indicates the
  // result; the other will be NULL for now.
  CPT(TransformState) result = do_invert_compose(other);
  // We store them in this order, on the off-chance that other is the
  // same as this, a degenerate case which is still worth supporting.
  ((TransformState *)other)->_invert_composition_cache[this]._result = NULL;
  ((TransformState *)this)->_invert_composition_cache[other]._result = result;

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::output
//       Access: Published, Virtual
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
    if (!components_given()) {
      out << "m";
    }
    char lead = '(';
    if (!get_pos().almost_equal(LVecBase3f(0.0f, 0.0f, 0.0f))) {
      out << lead << "pos " << get_pos();
      lead = ' ';
    }
    if (!get_hpr().almost_equal(LVecBase3f(0.0f, 0.0f, 0.0f))) {
      out << lead << "hpr " << get_hpr();
      lead = ' ';
    }
    if (!get_scale().almost_equal(LVecBase3f(1.0f, 1.0f, 1.0f))) {
      out << lead << "scale " << get_scale();
      lead = ' ';
    }
    if (lead == '(') {
      out << "(almost identity)";
    } else {
      out << ")";
    }

  } else {
    out << get_mat();
  }
}


////////////////////////////////////////////////////////////////////
//     Function: TransformState::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TransformState::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::return_new
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
return_new(TransformState *state) {
  nassertr(state != (TransformState *)NULL, state);

  // This should be a newly allocated pointer, not one that was used
  // for anything else.
  nassertr(state->_saved_entry == _states->end(), state);

  // Save the state in a local PointerTo so that it will be freed at
  // the end of this function if no one else uses it.
  CPT(TransformState) pt_state = state;

  pair<States::iterator, bool> result = _states->insert(state);
  if (result.second) {
    // The state was inserted; save the iterator and return the
    // input state.
    state->_saved_entry = result.first;
    return pt_state;
  }

  // The state was not inserted; there must be an equivalent one
  // already in the set.  Return that one.
  return *(result.first);
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
  nassertr((_flags & F_is_invalid) == 0, this);
  nassertr((other->_flags & F_is_invalid) == 0, other);

  // We should do this operation componentwise if both transforms were
  // given componentwise.

  LMatrix4f new_mat = other->get_mat() * get_mat();
  return make_mat(new_mat);
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::do_invert_compose
//       Access: Private
//  Description: The private implemention of invert_compose().
////////////////////////////////////////////////////////////////////
CPT(TransformState) TransformState::
do_invert_compose(const TransformState *other) const {
  nassertr((_flags & F_is_invalid) == 0, this);
  nassertr((other->_flags & F_is_invalid) == 0, other);

  if (is_singular()) {
    return make_invalid();
  }

  // We should do this operation componentwise if both transforms were
  // given componentwise.

  // Now that is_singular() has returned true, we can assume that
  // _inv_mat has been allocated and filled in.
  nassertr(_inv_mat != (LMatrix4f *)NULL, make_invalid());

  if (other->is_identity()) {
    return make_mat(*_inv_mat);
  } else {
    return make_mat(other->get_mat() * (*_inv_mat));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::calc_singular
//       Access: Private
//  Description: Determines whether the transform is singular (i.e. it
//               scales to zero, and has no inverse).
////////////////////////////////////////////////////////////////////
void TransformState::
calc_singular() {
  nassertv((_flags & F_is_invalid) == 0);

  // We determine if a matrix is singular by attempting to invert it
  // (and we save the result of this invert operation for a subsequent
  // do_invert_compose() call, which is almost certain to be made if
  // someone is asking whether we're singular).

  // This should be NULL if no one has called calc_singular() yet.
  nassertv(_inv_mat == (LMatrix4f *)NULL);
  _inv_mat = new LMatrix4f;
  bool inverted = _inv_mat->invert_from(get_mat());

  if (!inverted) {
    _flags |= F_is_singular;
    delete _inv_mat;
    _inv_mat = (LMatrix4f *)NULL;
  }
  _flags |= F_singular_known;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::calc_components
//       Access: Private
//  Description: Derives the components from the matrix, if possible.
////////////////////////////////////////////////////////////////////
void TransformState::
calc_components() {
  nassertv((_flags & F_is_invalid) == 0);
  if ((_flags & F_is_identity) != 0) {
    _scale.set(1.0f, 1.0f, 1.0f);
    _hpr.set(0.0f, 0.0f, 0.0f);
    _pos.set(0.0f, 0.0f, 0.0f);
    _flags |= F_has_components;

  } else {
    // If we don't have components and we're not identity, the only
    // other explanation is that we were constructed via a matrix.
    nassertv((_flags & F_mat_known) != 0);

    const LMatrix4f &mat = get_mat();
    bool possible = decompose_matrix(mat, _scale, _hpr, _pos);
    if (possible) {
      // Some matrices can't be decomposed into scale, hpr, pos.
      _flags |= F_has_components;

      // However, we can always get at least the pos.
      mat.get_row3(_pos, 3);
    }
  }

  _flags |= F_components_known;
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::calc_mat
//       Access: Private
//  Description: Computes the matrix from the components.
////////////////////////////////////////////////////////////////////
void TransformState::
calc_mat() {
  nassertv((_flags & F_is_invalid) == 0);
  if ((_flags & F_is_identity) != 0) {
    _mat = LMatrix4f::ident_mat();

  } else {
    // If we don't have a matrix and we're not identity, the only
    // other explanation is that we were constructed via components.
    nassertv((_flags & F_components_known) != 0);
    compose_matrix(_mat, _scale, _hpr, _pos);
  }
  _flags |= F_mat_known;
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
    int flags = F_is_identity | F_singular_known;
    dg.add_uint16(flags);

  } else if ((_flags & F_is_invalid) != 0) {
    // Invalid, nothing much to that either.
    int flags = F_is_invalid | F_singular_known | F_is_singular | F_components_known | F_mat_known;
    dg.add_uint16(flags);

  } else if ((_flags & F_components_given) != 0) {
    // A component-based transform.
    int flags = F_components_given | F_components_known | F_has_components;
    dg.add_uint16(flags);

    _pos.write_datagram(dg);
    _hpr.write_datagram(dg);
    _scale.write_datagram(dg);

  } else {
    // A general matrix.
    nassertv((_flags & F_mat_known) != 0);
    int flags = F_mat_known;
    dg.add_uint16(flags);
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
TypedWritable *TransformState::
change_this(TypedWritable *old_ptr, BamReader *manager) {
  // First, uniquify the pointer.
  TransformState *state = DCAST(TransformState, old_ptr);
  CPT(TransformState) pointer = return_new(state);

  // But now we have a problem, since we have to hold the reference
  // count and there's no way to return a TypedWritable while still
  // holding the reference count!  We work around this by explicitly
  // upping the count, and also setting a finalize() callback to down
  // it later.
  if (pointer == state) {
    pointer->ref();
    manager->register_finalize(state);
  }
  
  // We have to cast the pointer back to non-const, because the bam
  // reader expects that.
  return (TransformState *)pointer.p();
}

////////////////////////////////////////////////////////////////////
//     Function: TransformState::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void TransformState::
finalize() {
  // Unref the pointer that we explicitly reffed in make_from_bam().
  unref();

  // We should never get back to zero after unreffing our own count,
  // because we expect to have been stored in a pointer somewhere.  If
  // we do get to zero, it's a memory leak; the way to avoid this is
  // to call unref_delete() above instead of unref(), but this is
  // dangerous to do from within a virtual function.
  nassertv(get_ref_count() != 0);
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

  _flags = scan.get_uint16();

  if ((_flags & F_components_known) != 0) {
    // Componentwise transform.
    _pos.read_datagram(scan);
    _hpr.read_datagram(scan);
    _scale.read_datagram(scan);
  }

  if ((_flags & F_mat_known) != 0) {
    // General matrix.
    _mat.read_datagram(scan);
  }
}
