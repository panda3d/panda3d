// Filename: renderEffects.cxx
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

#include "renderEffects.h"
#include "billboardEffect.h"
#include "decalEffect.h"
#include "compassEffect.h"
#include "polylightEffect.h"
#include "showBoundsEffect.h"
#include "config_pgraph.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagramIterator.h"
#include "indent.h"
#include "compareTo.h"

RenderEffects::States *RenderEffects::_states = NULL;
CPT(RenderEffects) RenderEffects::_empty_state;
TypeHandle RenderEffects::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::Constructor
//       Access: Protected
//  Description: Actually, this could be a private constructor, since
//               no one inherits from RenderEffects, but gcc gives us a
//               spurious warning if all constructors are private.
////////////////////////////////////////////////////////////////////
RenderEffects::
RenderEffects() {
  if (_states == (States *)NULL) {
    // Make sure the global _states map is allocated.  This only has
    // to be done once.  We could make this map static, but then we
    // run into problems if anyone creates a RenderState object at
    // static init time; it also seems to cause problems when the
    // Panda shared library is unloaded at application exit time.
    _states = new States;
  }
  _saved_entry = _states->end();
  _flags = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::Copy Constructor
//       Access: Private
//  Description: RenderEffectss are not meant to be copied.
////////////////////////////////////////////////////////////////////
RenderEffects::
RenderEffects(const RenderEffects &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::Copy Assignment Operator
//       Access: Private
//  Description: RenderEffectss are not meant to be copied.
////////////////////////////////////////////////////////////////////
void RenderEffects::
operator = (const RenderEffects &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::Destructor
//       Access: Public, Virtual
//  Description: The destructor is responsible for removing the
//               RenderEffects from the global set if it is there.
////////////////////////////////////////////////////////////////////
RenderEffects::
~RenderEffects() {
  // Remove the deleted RenderEffects object from the global pool.
  if (_saved_entry != _states->end()) {
    _states->erase(_saved_entry);
    _saved_entry = _states->end();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::safe_to_transform
//       Access: Public
//  Description: Returns true if all of the effects in this set can
//               safely be transformed, and therefore the complete set
//               can be transformed, by calling xform().
////////////////////////////////////////////////////////////////////
bool RenderEffects::
safe_to_transform() const {
  Effects::const_iterator ai;
  for (ai = _effects.begin(); ai != _effects.end(); ++ai) {
    const Effect &effect = (*ai);
    if (!effect._effect->safe_to_transform()) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::prepare_flatten_transform
//       Access: Public, Virtual
//  Description: Preprocesses the accumulated transform that is about
//               to be applied to (or through) this node due to a
//               flatten operation.  The returned value will be used
//               instead.
////////////////////////////////////////////////////////////////////
CPT(TransformState) RenderEffects::
prepare_flatten_transform(const TransformState *net_transform) const {
  CPT(TransformState) result = net_transform;
  Effects::const_iterator ai;
  for (ai = _effects.begin(); ai != _effects.end(); ++ai) {
    const Effect &effect = (*ai);
    result = effect._effect->prepare_flatten_transform(result);
  }

  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::safe_to_combine
//       Access: Public
//  Description: Returns true if all of the effects in this set can
//               safely be shared with a sibling node that has the
//               exact same set of effects, or false if this would be
//               bad for any of the effects.
////////////////////////////////////////////////////////////////////
bool RenderEffects::
safe_to_combine() const {
  Effects::const_iterator ai;
  for (ai = _effects.begin(); ai != _effects.end(); ++ai) {
    const Effect &effect = (*ai);
    if (!effect._effect->safe_to_combine()) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::xform
//       Access: Public, Virtual
//  Description: Returns a new RenderEffects transformed by the
//               indicated matrix.
////////////////////////////////////////////////////////////////////
CPT(RenderEffects) RenderEffects::
xform(const LMatrix4f &mat) const {
  if (is_empty()) {
    return this;
  }

  RenderEffects *new_state = new RenderEffects;
  back_insert_iterator<Effects> result = 
    back_inserter(new_state->_effects);

  Effects::const_iterator ai;
  for (ai = _effects.begin(); ai != _effects.end(); ++ai) {
    const Effect &effect = (*ai);
    Effect new_effect(effect);
    new_effect._effect = effect._effect->xform(mat);
    *result = new_effect;
    ++result;
  }

  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::operator <
//       Access: Published
//  Description: Provides an arbitrary ordering among all unique
//               RenderEffectss, so we can store the essentially
//               different ones in a big set and throw away the rest.
//
//               This method is not needed outside of the RenderEffects
//               class because all equivalent RenderEffects objects are
//               guaranteed to share the same pointer; thus, a pointer
//               comparison is always sufficient.
////////////////////////////////////////////////////////////////////
bool RenderEffects::
operator < (const RenderEffects &other) const {
  // We must compare all the properties of the effects, not just
  // the type; thus, we compare them one at a time using compare_to().
  return lexicographical_compare(_effects.begin(), _effects.end(),
                                 other._effects.begin(), other._effects.end(),
                                 CompareTo<Effect>());
}


////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::find_effect
//       Access: Published
//  Description: Searches for an effect with the indicated type in
//               the state, and returns its index if it is found, or
//               -1 if it is not.
////////////////////////////////////////////////////////////////////
int RenderEffects::
find_effect(TypeHandle type) const {
  Effects::const_iterator ai = _effects.find(Effect(type));
  if (ai == _effects.end()) {
    return -1;
  }
  return ai - _effects.begin();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::make_empty
//       Access: Published, Static
//  Description: Returns a RenderEffects with no effects set.
////////////////////////////////////////////////////////////////////
CPT(RenderEffects) RenderEffects::
make_empty() {
  // The empty state is asked for so often, we make it a special case
  // and store a pointer forever once we find it the first time.
  if (_empty_state == (RenderEffects *)NULL) {
    RenderEffects *state = new RenderEffects;
    _empty_state = return_new(state);
  }

  return _empty_state;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::make
//       Access: Published, Static
//  Description: Returns a RenderEffects with one effect set.
////////////////////////////////////////////////////////////////////
CPT(RenderEffects) RenderEffects::
make(const RenderEffect *effect) {
  RenderEffects *state = new RenderEffects;
  state->_effects.reserve(1);
  state->_effects.insert(Effect(effect));
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::make
//       Access: Published, Static
//  Description: Returns a RenderEffects with two effects set.
////////////////////////////////////////////////////////////////////
CPT(RenderEffects) RenderEffects::
make(const RenderEffect *effect1,
     const RenderEffect *effect2) {
  RenderEffects *state = new RenderEffects;
  state->_effects.reserve(2);
  state->_effects.push_back(Effect(effect1));
  state->_effects.push_back(Effect(effect2));
  state->_effects.sort();
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::make
//       Access: Published, Static
//  Description: Returns a RenderEffects with three effects set.
////////////////////////////////////////////////////////////////////
CPT(RenderEffects) RenderEffects::
make(const RenderEffect *effect1,
     const RenderEffect *effect2,
     const RenderEffect *effect3) {
  RenderEffects *state = new RenderEffects;
  state->_effects.reserve(2);
  state->_effects.push_back(Effect(effect1));
  state->_effects.push_back(Effect(effect2));
  state->_effects.push_back(Effect(effect3));
  state->_effects.sort();
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::make
//       Access: Published, Static
//  Description: Returns a RenderEffects with four effects set.
////////////////////////////////////////////////////////////////////
CPT(RenderEffects) RenderEffects::
make(const RenderEffect *effect1,
     const RenderEffect *effect2,
     const RenderEffect *effect3,
     const RenderEffect *effect4) {
  RenderEffects *state = new RenderEffects;
  state->_effects.reserve(2);
  state->_effects.push_back(Effect(effect1));
  state->_effects.push_back(Effect(effect2));
  state->_effects.push_back(Effect(effect3));
  state->_effects.push_back(Effect(effect4));
  state->_effects.sort();
  return return_new(state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::add_effect
//       Access: Published
//  Description: Returns a new RenderEffects object that represents the
//               same as the source state, with the new RenderEffect
//               added.  If there is already a RenderEffect with the
//               same type, it is replaced.
////////////////////////////////////////////////////////////////////
CPT(RenderEffects) RenderEffects::
add_effect(const RenderEffect *effect) const {
  RenderEffects *new_state = new RenderEffects;
  back_insert_iterator<Effects> result = 
    back_inserter(new_state->_effects);

  Effect new_effect(effect);
  Effects::const_iterator ai = _effects.begin();

  while (ai != _effects.end() && (*ai) < new_effect) {
    *result = *ai;
    ++ai;
    ++result;
  }
  *result = new_effect;
  ++result;

  if (ai != _effects.end() && !(new_effect < (*ai))) {
    // At this point we know:
    // !((*ai) < new_effect) && !(new_effect < (*ai))
    // which means (*ai) == new_effect--so we should leave it out,
    // to avoid duplicating effects in the set.
    ++ai;
  }

  while (ai != _effects.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::remove_effect
//       Access: Published
//  Description: Returns a new RenderEffects object that represents the
//               same as the source state, with the indicated
//               RenderEffect removed.
////////////////////////////////////////////////////////////////////
CPT(RenderEffects) RenderEffects::
remove_effect(TypeHandle type) const {
  RenderEffects *new_state = new RenderEffects;
  back_insert_iterator<Effects> result = 
    back_inserter(new_state->_effects);

  Effects::const_iterator ai = _effects.begin();

  while (ai != _effects.end()) {
    if ((*ai)._type != type) {
      *result = *ai;
      ++result;
    }
    ++ai;
  }

  return return_new(new_state);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::get_effect
//       Access: Published, Virtual
//  Description: Looks for a RenderEffect of the indicated type in the
//               state, and returns it if it is found, or NULL if it
//               is not.
////////////////////////////////////////////////////////////////////
const RenderEffect *RenderEffects::
get_effect(TypeHandle type) const {
  Effects::const_iterator ai;
  ai = _effects.find(Effect(type));
  if (ai != _effects.end()) {
    return (*ai)._effect;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderEffects::
output(ostream &out) const {
  out << "E:";
  if (_effects.empty()) {
    out << "(empty)";

  } else {
    Effects::const_iterator ai = _effects.begin();
    out << "(" << (*ai)._type;
    ++ai;
    while (ai != _effects.end()) {
      out << " " << (*ai)._type;
      ++ai;
    }
    out << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderEffects::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << _effects.size() << " effects:\n";
  Effects::const_iterator ai;
  for (ai = _effects.begin(); ai != _effects.end(); ++ai) {
    const Effect &effect = (*ai);
    effect._effect->write(out, indent_level + 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::cull_callback
//       Access: Public
//  Description: Calls cull_callback() on all effects.  You may check
//               has_cull_callback() first to see if any effects
//               define this method to do anything useful.
////////////////////////////////////////////////////////////////////
void RenderEffects::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &node_state) const {
  Effects::const_iterator ei;
  for (ei = _effects.begin(); ei != _effects.end(); ++ei) {
    (*ei)._effect->cull_callback(trav, data, node_transform, node_state);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::adjust_transform
//       Access: Public
//  Description: Calls adjust_transform() on all effects.  You may check
//               has_adjust_transform() first to see if any effects
//               define this method to do anything useful.
//
//               The order in which the individual effects are applied
//               is not defined, so if more than one effect applies a
//               change to the transform on any particular node, you
//               might get indeterminate results.
////////////////////////////////////////////////////////////////////
void RenderEffects::
adjust_transform(CPT(TransformState) &net_transform,
                 CPT(TransformState) &node_transform) const {
  Effects::const_iterator ei;
  for (ei = _effects.begin(); ei != _effects.end(); ++ei) {
    (*ei)._effect->adjust_transform(net_transform, node_transform);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::get_num_states
//       Access: Published, Static
//  Description: Returns the total number of unique RenderEffects
//               objects allocated in the world.  This will go up and
//               down during normal operations.
////////////////////////////////////////////////////////////////////
int RenderEffects::
get_num_states() {
  if (_states == (States *)NULL) {
    return 0;
  }
  return _states->size();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::list_states
//       Access: Published, Static
//  Description: Lists all of the RenderEffectss in the cache to the
//               output stream, one per line.  This can be quite a lot
//               of output if the cache is large, so be prepared.
////////////////////////////////////////////////////////////////////
void RenderEffects::
list_states(ostream &out) {
  out << _states->size() << " states:\n";
  States::const_iterator si;
  for (si = _states->begin(); si != _states->end(); ++si) {
    const RenderEffects *state = (*si);
    state->write(out, 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::validate_states
//       Access: Published, Static
//  Description: Ensures that the cache is still stored in sorted
//               order.  Returns true if so, false if there is a
//               problem (which implies someone has modified one of
//               the supposedly-const RenderEffects objects).
////////////////////////////////////////////////////////////////////
bool RenderEffects::
validate_states() {
  if (_states->empty()) {
    return true;
  }

  States::const_iterator si = _states->begin();
  States::const_iterator snext = si;
  ++snext;
  while (snext != _states->end()) {
    if (!(*(*si) < *(*snext))) {
      pgraph_cat.error()
        << "RenderEffectss out of order!\n";
      (*si)->write(pgraph_cat.error(false), 2);
      (*snext)->write(pgraph_cat.error(false), 2);
      return false;
    }
    si = snext;
    ++snext;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::return_new
//       Access: Private, Static
//  Description: This function is used to share a common RenderEffects
//               pointer for all equivalent RenderEffects objects.
//
//               See the similar logic in RenderEffect.  The idea is
//               to create a new RenderEffects object and pass it
//               through this function, which will share the pointer
//               with a previously-created RenderEffects object if it is
//               equivalent.
////////////////////////////////////////////////////////////////////
CPT(RenderEffects) RenderEffects::
return_new(RenderEffects *state) {
  nassertr(state != (RenderEffects *)NULL, state);

  // This should be a newly allocated pointer, not one that was used
  // for anything else.
  nassertr(state->_saved_entry == _states->end(), state);

#ifndef NDEBUG
  if (paranoid_const) {
    nassertr(validate_states(), state);
  }
#endif

  // Save the state in a local PointerTo so that it will be freed at
  // the end of this function if no one else uses it.
  CPT(RenderEffects) pt_state = state;

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
//     Function: RenderEffects::determine_decal
//       Access: Private
//  Description: This is the private implementation of has_decal().
////////////////////////////////////////////////////////////////////
void RenderEffects::
determine_decal() {
  const RenderEffect *effect = get_effect(DecalEffect::get_class_type());
  if (effect != (const RenderEffect *)NULL) {
    _flags |= F_has_decal;
  }
  _flags |= F_checked_decal;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::determine_show_bounds
//       Access: Private
//  Description: This is the private implementation of has_show_bounds().
////////////////////////////////////////////////////////////////////
void RenderEffects::
determine_show_bounds() {
  const RenderEffect *effect = get_effect(ShowBoundsEffect::get_class_type());
  if (effect != (const RenderEffect *)NULL) {
    _flags |= F_has_show_bounds;
    const ShowBoundsEffect *sba = DCAST(ShowBoundsEffect, effect);
    if (sba->get_tight()) {
      _flags |= F_has_show_tight_bounds;
    }
  }
  _flags |= F_checked_show_bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::determine_cull_callback
//       Access: Private
//  Description: This is the private implementation of has_cull_callback().
////////////////////////////////////////////////////////////////////
void RenderEffects::
determine_cull_callback() {
  _flags |= F_checked_cull_callback;

  Effects::const_iterator ei;
  for (ei = _effects.begin(); ei != _effects.end(); ++ei) {
    if ((*ei)._effect->has_cull_callback()) {
      _flags |= F_has_cull_callback;
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::determine_adjust_transform
//       Access: Private
//  Description: This is the private implementation of has_adjust_transform().
////////////////////////////////////////////////////////////////////
void RenderEffects::
determine_adjust_transform() {
  _flags |= F_checked_adjust_transform;

  Effects::const_iterator ei;
  for (ei = _effects.begin(); ei != _effects.end(); ++ei) {
    if ((*ei)._effect->has_adjust_transform()) {
      _flags |= F_has_adjust_transform;
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               RenderEffects.
////////////////////////////////////////////////////////////////////
void RenderEffects::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void RenderEffects::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  int num_effects = _effects.size();
  nassertv(num_effects == (int)(PN_uint16)num_effects);
  dg.add_uint16(num_effects);

  Effects::const_iterator ai;
  for (ai = _effects.begin(); ai != _effects.end(); ++ai) {
    const Effect &effect = (*ai);

    manager->write_pointer(dg, effect._effect);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int RenderEffects::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  // Get the effect pointers.
  Effects::iterator ai;
  for (ai = _effects.begin(); ai != _effects.end(); ++ai) {
    Effect &effect = (*ai);

    effect._effect = DCAST(RenderEffect, p_list[pi++]);
    nassertr(effect._effect != (RenderEffect *)NULL, pi);
    effect._type = effect._effect->get_type();
  }

  // Now make sure the array is properly sorted.  (It won't
  // necessarily preserve its correct sort after being read from bam,
  // because the sort is based on TypeHandle indices, which can change
  // from session to session.)
  _effects.sort();

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::change_this
//       Access: Public, Static
//  Description: Called immediately after complete_pointers(), this
//               gives the object a chance to adjust its own pointer
//               if desired.  Most objects don't change pointers after
//               completion, but some need to.
//
//               Once this function has been called, the old pointer
//               will no longer be accessed.
////////////////////////////////////////////////////////////////////
TypedWritable *RenderEffects::
change_this(TypedWritable *old_ptr, BamReader *manager) {
  // First, uniquify the pointer.
  RenderEffects *state = DCAST(RenderEffects, old_ptr);
  CPT(RenderEffects) pointer = return_new(state);

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
  return (RenderEffects *)pointer.p();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void RenderEffects::
finalize(BamReader *) {
  // Unref the pointer that we explicitly reffed in change_this().
  unref();

  // We should never get back to zero after unreffing our own count,
  // because we expect to have been stored in a pointer somewhere.  If
  // we do get to zero, it's a memory leak; the way to avoid this is
  // to call unref_delete() above instead of unref(), but this is
  // dangerous to do from within a virtual function.
  nassertv(get_ref_count() != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type RenderEffects is encountered
//               in the Bam file.  It should create the RenderEffects
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *RenderEffects::
make_from_bam(const FactoryParams &params) {
  RenderEffects *state = new RenderEffects;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  state->fillin(scan, manager);
  manager->register_change_this(change_this, state);

  return state;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderEffects::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RenderEffects.
////////////////////////////////////////////////////////////////////
void RenderEffects::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  int num_effects = scan.get_uint16();

  // Push back a NULL pointer for each effect for now, until we get
  // the actual list of pointers later in complete_pointers().
  _effects.reserve(num_effects);
  for (int i = 0; i < num_effects; i++) {
    manager->read_pointer(scan);
    _effects.push_back(Effect());
  }
}
