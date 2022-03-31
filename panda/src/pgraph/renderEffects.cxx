/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderEffects.cxx
 * @author drose
 * @date 2002-03-14
 */

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
#include "lightReMutexHolder.h"
#include "lightMutexHolder.h"
#include "thread.h"

#include <iterator>

LightReMutex *RenderEffects::_states_lock = nullptr;
RenderEffects::States *RenderEffects::_states = nullptr;
CPT(RenderEffects) RenderEffects::_empty_state;
TypeHandle RenderEffects::_type_handle;

/**
 * Actually, this could be a private constructor, since no one inherits from
 * RenderEffects, but gcc gives us a spurious warning if all constructors are
 * private.
 */
RenderEffects::
RenderEffects() : _lock("RenderEffects") {
  if (_states == nullptr) {
    init_states();
  }
  _saved_entry = _states->end();
  _flags = 0;
}

/**
 * The destructor is responsible for removing the RenderEffects from the
 * global set if it is there.
 */
RenderEffects::
~RenderEffects() {
  // Remove the deleted RenderEffects object from the global pool.
  LightReMutexHolder holder(*_states_lock);

  // unref() should have cleared this.
  nassertv(_saved_entry == _states->end());
}

/**
 * Returns true if all of the effects in this set can safely be transformed,
 * and therefore the complete set can be transformed, by calling xform().
 */
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

/**
 * Preprocesses the accumulated transform that is about to be applied to (or
 * through) this node due to a flatten operation.  The returned value will be
 * used instead.
 */
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

/**
 * Returns true if all of the effects in this set can safely be shared with a
 * sibling node that has the exact same set of effects, or false if this would
 * be bad for any of the effects.
 */
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

/**
 * Returns a new RenderEffects transformed by the indicated matrix.
 */
CPT(RenderEffects) RenderEffects::
xform(const LMatrix4 &mat) const {
  if (is_empty()) {
    return this;
  }

  RenderEffects *new_state = new RenderEffects;
  std::back_insert_iterator<Effects> result =
    std::back_inserter(new_state->_effects);

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

/**
 * Provides an arbitrary ordering among all unique RenderEffects, so we can
 * store the essentially different ones in a big set and throw away the rest.
 *
 * This method is not needed outside of the RenderEffects class because all
 * equivalent RenderEffects objects are guaranteed to share the same pointer;
 * thus, a pointer comparison is always sufficient.
 */
bool RenderEffects::
operator < (const RenderEffects &other) const {
  // We must compare all the properties of the effects, not just the type;
  // thus, we compare them one at a time using compare_to().
  return lexicographical_compare(_effects.begin(), _effects.end(),
                                 other._effects.begin(), other._effects.end(),
                                 CompareTo<Effect>());
}


/**
 * Searches for an effect with the indicated type in the state, and returns
 * its index if it is found, or -1 if it is not.
 */
int RenderEffects::
find_effect(TypeHandle type) const {
  Effects::const_iterator ai = _effects.find(Effect(type));
  if (ai == _effects.end()) {
    return -1;
  }
  return ai - _effects.begin();
}

/**
 * Returns a RenderEffects with no effects set.
 */
CPT(RenderEffects) RenderEffects::
make_empty() {
  // The empty state is asked for so often, we make it a special case and
  // store a pointer forever once we find it the first time.
  if (_empty_state == nullptr) {
    RenderEffects *state = new RenderEffects;
    _empty_state = return_new(state);
  }

  return _empty_state;
}

/**
 * Returns a RenderEffects with one effect set.
 */
CPT(RenderEffects) RenderEffects::
make(const RenderEffect *effect) {
  RenderEffects *state = new RenderEffects;
  state->_effects.reserve(1);
  state->_effects.insert(Effect(effect));
  return return_new(state);
}

/**
 * Returns a RenderEffects with two effects set.
 */
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

/**
 * Returns a RenderEffects with three effects set.
 */
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

/**
 * Returns a RenderEffects with four effects set.
 */
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

/**
 * Returns a new RenderEffects object that represents the same as the source
 * state, with the new RenderEffect added.  If there is already a RenderEffect
 * with the same type, it is replaced.
 */
CPT(RenderEffects) RenderEffects::
add_effect(const RenderEffect *effect) const {
  RenderEffects *new_state = new RenderEffects;
  std::back_insert_iterator<Effects> result =
    std::back_inserter(new_state->_effects);

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
    // At this point we know: !((*ai) < new_effect) && !(new_effect < (*ai))
    // which means (*ai) == new_effect--so we should leave it out, to avoid
    // duplicating effects in the set.
    ++ai;
  }

  while (ai != _effects.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  return return_new(new_state);
}

/**
 * Returns a new RenderEffects object that represents the same as the source
 * state, with the indicated RenderEffect removed.
 */
CPT(RenderEffects) RenderEffects::
remove_effect(TypeHandle type) const {
  RenderEffects *new_state = new RenderEffects;
  std::back_insert_iterator<Effects> result =
    std::back_inserter(new_state->_effects);

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

/**
 * Looks for a RenderEffect of the indicated type in the state, and returns it
 * if it is found, or NULL if it is not.
 */
const RenderEffect *RenderEffects::
get_effect(TypeHandle type) const {
  Effects::const_iterator ai;
  ai = _effects.find(Effect(type));
  if (ai != _effects.end()) {
    return (*ai)._effect;
  }
  return nullptr;
}

/**
 * This method overrides ReferenceCount::unref() to check whether the
 * remaining reference count is entirely in the cache, and if so, it checks
 * for and breaks a cycle in the cache involving this object.  This is
 * designed to prevent leaks from cyclical references within the cache.
 *
 * Note that this is not a virtual method, and cannot be because
 * ReferenceCount itself declares no virtual methods (it avoids the overhead
 * of a virtual function pointer).  But this doesn't matter, because
 * PT(TransformState) is a template class, and will call the appropriate
 * method even though it is non-virtual.
 */
bool RenderEffects::
unref() const {
  LightReMutexHolder holder(*_states_lock);

  if (ReferenceCount::unref()) {
    // The reference count is still nonzero.
    return true;
  }

  // The reference count has just reached zero.  Make sure the object is
  // removed from the global object pool, before anyone else finds it and
  // tries to ref it.
  ((RenderEffects *)this)->release_new();

  return false;
}

/**
 *
 */
void RenderEffects::
output(std::ostream &out) const {
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

/**
 *
 */
void RenderEffects::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << _effects.size() << " effects:\n";
  Effects::const_iterator ai;
  for (ai = _effects.begin(); ai != _effects.end(); ++ai) {
    const Effect &effect = (*ai);
    effect._effect->write(out, indent_level + 2);
  }
}

/**
 * Returns the total number of unique RenderEffects objects allocated in the
 * world.  This will go up and down during normal operations.
 */
int RenderEffects::
get_num_states() {
  if (_states == nullptr) {
    return 0;
  }
  LightReMutexHolder holder(*_states_lock);
  return _states->size();
}

/**
 * Lists all of the RenderEffects in the cache to the output stream, one per
 * line.  This can be quite a lot of output if the cache is large, so be
 * prepared.
 */
void RenderEffects::
list_states(std::ostream &out) {
  out << _states->size() << " states:\n";
  States::const_iterator si;
  for (si = _states->begin(); si != _states->end(); ++si) {
    const RenderEffects *state = (*si);
    state->write(out, 2);
  }
}

/**
 * Ensures that the cache is still stored in sorted order.  Returns true if
 * so, false if there is a problem (which implies someone has modified one of
 * the supposedly-const RenderEffects objects).
 */
bool RenderEffects::
validate_states() {
  if (_states->empty()) {
    return true;
  }
  LightReMutexHolder holder(*_states_lock);

  States::const_iterator si = _states->begin();
  States::const_iterator snext = si;
  ++snext;
  while (snext != _states->end()) {
    if (!(*(*si) < *(*snext))) {
      pgraph_cat.error()
        << "RenderEffects out of order!\n";
      (*si)->write(pgraph_cat.error(false), 2);
      (*snext)->write(pgraph_cat.error(false), 2);
      return false;
    }
    if ((*(*snext) < *(*si))) {
      pgraph_cat.error()
        << "RenderEffects::operator < not defined properly!\n";
      pgraph_cat.error(false)
        << "a < b: " << (*(*si) < *(*snext)) << "\n";
      pgraph_cat.error(false)
        << "b < a: " << (*(*snext) < *(*si)) << "\n";
      (*si)->write(pgraph_cat.error(false), 2);
      (*snext)->write(pgraph_cat.error(false), 2);
      return false;
    }
    si = snext;
    ++snext;
  }

  return true;
}

/**
 * Calls cull_callback() on all effects.  You may check has_cull_callback()
 * first to see if any effects define this method to do anything useful.
 */
void RenderEffects::
cull_callback(CullTraverser *trav, CullTraverserData &data,
              CPT(TransformState) &node_transform,
              CPT(RenderState) &node_state) const {
  Effects::const_iterator ei;
  for (ei = _effects.begin(); ei != _effects.end(); ++ei) {
    (*ei)._effect->cull_callback(trav, data, node_transform, node_state);
  }
}

/**
 * Calls adjust_transform() on all effects.  You may check
 * has_adjust_transform() first to see if any effects define this method to do
 * anything useful.
 *
 * The order in which the individual effects are applied is not defined, so if
 * more than one effect applies a change to the transform on any particular
 * node, you might get indeterminate results.
 */
void RenderEffects::
adjust_transform(CPT(TransformState) &net_transform,
                 CPT(TransformState) &node_transform,
                 const PandaNode *node) const {
  Effects::const_iterator ei;
  for (ei = _effects.begin(); ei != _effects.end(); ++ei) {
    (*ei)._effect->adjust_transform(net_transform, node_transform, node);
  }
}

/**
 * Make sure the global _states map is allocated.  This only has to be done
 * once.  We could make this map static, but then we run into problems if
 * anyone creates a RenderEffects object at static init time; it also seems to
 * cause problems when the Panda shared library is unloaded at application
 * exit time.
 */
void RenderEffects::
init_states() {
  _states = new States;

  // TODO: we should have a global Panda mutex to allow us to safely create
  // _states_lock without a startup race condition.  For the meantime, this is
  // OK because we guarantee that this method is called at static init time,
  // presumably when there is still only one thread in the world.
  _states_lock = new LightReMutex("RenderEffects::_states_lock");
  nassertv(Thread::get_current_thread() == Thread::get_main_thread());
}


/**
 * This function is used to share a common RenderEffects pointer for all
 * equivalent RenderEffects objects.
 *
 * See the similar logic in RenderEffect.  The idea is to create a new
 * RenderEffects object and pass it through this function, which will share
 * the pointer with a previously-created RenderEffects object if it is
 * equivalent.
 */
CPT(RenderEffects) RenderEffects::
return_new(RenderEffects *state) {
  nassertr(state != nullptr, state);

#ifndef NDEBUG
  if (!state_cache) {
    return state;
  }
#endif

#ifndef NDEBUG
  if (paranoid_const) {
    nassertr(validate_states(), state);
  }
#endif

  LightReMutexHolder holder(*_states_lock);

  // This should be a newly allocated pointer, not one that was used for
  // anything else.
  nassertr(state->_saved_entry == _states->end(), state);

  // Save the state in a local PointerTo so that it will be freed at the end
  // of this function if no one else uses it.
  CPT(RenderEffects) pt_state = state;

  std::pair<States::iterator, bool> result = _states->insert(state);
  if (result.second) {
    // The state was inserted; save the iterator and return the input state.
    state->_saved_entry = result.first;
    nassertr(_states->find(state) == state->_saved_entry, pt_state);
    return pt_state;
  }

  // The state was not inserted; there must be an equivalent one already in
  // the set.  Return that one.
  return *(result.first);
}

/**
 * This inverse of return_new, this releases this object from the global
 * RenderEffects table.
 *
 * You must already be holding _states_lock before you call this method.
 */
void RenderEffects::
release_new() {
  nassertv(_states_lock->debug_is_locked());

  if (_saved_entry != _states->end()) {
    nassertv(_states->find(this) == _saved_entry);
    _states->erase(_saved_entry);
    _saved_entry = _states->end();
  }
}

/**
 * This is the private implementation of has_decal().
 */
void RenderEffects::
determine_decal() {
  LightMutexHolder holder(_lock);
  if ((_flags & F_checked_decal) != 0) {
    // Someone else checked it first.
    return;
  }

  const RenderEffect *effect = get_effect(DecalEffect::get_class_type());
  if (effect != nullptr) {
    _flags |= F_has_decal;
  }
  _flags |= F_checked_decal;
}

/**
 * This is the private implementation of has_show_bounds().
 */
void RenderEffects::
determine_show_bounds() {
  LightMutexHolder holder(_lock);
  if ((_flags & F_checked_show_bounds) != 0) {
    // Someone else checked it first.
    return;
  }

  const RenderEffect *effect = get_effect(ShowBoundsEffect::get_class_type());
  if (effect != nullptr) {
    _flags |= F_has_show_bounds;
    const ShowBoundsEffect *sba = DCAST(ShowBoundsEffect, effect);
    if (sba->get_tight()) {
      _flags |= F_has_show_tight_bounds;
    }
  }
  _flags |= F_checked_show_bounds;
}

/**
 * This is the private implementation of has_cull_callback().
 */
void RenderEffects::
determine_cull_callback() {
  LightMutexHolder holder(_lock);
  if ((_flags & F_checked_cull_callback) != 0) {
    // Someone else checked it first.
    return;
  }

  _flags |= F_checked_cull_callback;

  Effects::const_iterator ei;
  for (ei = _effects.begin(); ei != _effects.end(); ++ei) {
    if ((*ei)._effect->has_cull_callback()) {
      _flags |= F_has_cull_callback;
      return;
    }
  }
}

/**
 * This is the private implementation of has_adjust_transform().
 */
void RenderEffects::
determine_adjust_transform() {
  LightMutexHolder holder(_lock);
  if ((_flags & F_checked_adjust_transform) != 0) {
    // Someone else checked it first.
    return;
  }

  _flags |= F_checked_adjust_transform;

  Effects::const_iterator ei;
  for (ei = _effects.begin(); ei != _effects.end(); ++ei) {
    if ((*ei)._effect->has_adjust_transform()) {
      _flags |= F_has_adjust_transform;
      return;
    }
  }
}

/**
 * Tells the BamReader how to create objects of type RenderEffects.
 */
void RenderEffects::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void RenderEffects::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);

  int num_effects = _effects.size();
  nassertv(num_effects == (int)(uint16_t)num_effects);
  dg.add_uint16(num_effects);

  Effects::const_iterator ai;
  for (ai = _effects.begin(); ai != _effects.end(); ++ai) {
    const Effect &effect = (*ai);

    manager->write_pointer(dg, effect._effect);
  }
}

/**
 * Receives an array of pointers, one for each time manager->read_pointer()
 * was called in fillin(). Returns the number of pointers processed.
 */
int RenderEffects::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = TypedWritable::complete_pointers(p_list, manager);

  // Get the effect pointers.
  size_t i = 0;
  while (i < _effects.size()) {
    Effect &effect = _effects[i];

    effect._effect = DCAST(RenderEffect, p_list[pi++]);
    if (effect._effect == nullptr) {
      // Remove this bogus RenderEffect pointer (it must have been from an
      // unwritable class).
      _effects.erase(_effects.begin() + i);

    } else {
      // Keep this good pointer, and increment.
      effect._type = effect._effect->get_type();
      ++i;
    }
  }

  // Now make sure the array is properly sorted.  (It won't necessarily
  // preserve its correct sort after being read from bam, because the sort is
  // based on TypeHandle indices, which can change from session to session.)
  _effects.sort();

  nassertr(_saved_entry == _states->end(), pi);
  return pi;
}

/**
 * Some objects require all of their nested pointers to have been completed
 * before the objects themselves can be completed.  If this is the case,
 * override this method to return true, and be careful with circular
 * references (which would make the object unreadable from a bam file).
 */
bool RenderEffects::
require_fully_complete() const {
  // Since we sort _states based on each RenderEffects' operator < method,
  // which in turn compares based on each nested RenderEffect object's
  // compare_to() method, some of which depend on the RenderEffect's pointers
  // having already been completed (e.g.  CharacterJointEffect), we therefore
  // require each of out our nested RenderEffect objects to have been
  // completed before we can be completed.
  return true;
}

/**
 * Called immediately after complete_pointers(), this gives the object a
 * chance to adjust its own pointer if desired.  Most objects don't change
 * pointers after completion, but some need to.
 *
 * Once this function has been called, the old pointer will no longer be
 * accessed.
 */
TypedWritable *RenderEffects::
change_this(TypedWritable *old_ptr, BamReader *manager) {
  // First, uniquify the pointer.
  RenderEffects *state = DCAST(RenderEffects, old_ptr);
  CPT(RenderEffects) pointer = return_new(state);

  // But now we have a problem, since we have to hold the reference count and
  // there's no way to return a TypedWritable while still holding the
  // reference count!  We work around this by explicitly upping the count, and
  // also setting a finalize() callback to down it later.
  if (pointer == state) {
    pointer->ref();
    manager->register_finalize(state);
  }

  // We have to cast the pointer back to non-const, because the bam reader
  // expects that.
  return (RenderEffects *)pointer.p();
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void RenderEffects::
finalize(BamReader *) {
  // Unref the pointer that we explicitly reffed in change_this().
  unref();

  // We should never get back to zero after unreffing our own count, because
  // we expect to have been stored in a pointer somewhere.  If we do get to
  // zero, it's a memory leak; the way to avoid this is to call unref_delete()
  // above instead of unref(), but this is dangerous to do from within a
  // virtual function.
  nassertv(get_ref_count() != 0);
}

/**
 * This function is called by the BamReader's factory when a new object of
 * type RenderEffects is encountered in the Bam file.  It should create the
 * RenderEffects and extract its information from the file.
 */
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

/**
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new RenderEffects.
 */
void RenderEffects::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);

  int num_effects = scan.get_uint16();

  // Push back a NULL pointer for each effect for now, until we get the actual
  // list of pointers later in complete_pointers().
  _effects.reserve(num_effects);
  for (int i = 0; i < num_effects; i++) {
    manager->read_pointer(scan);
    _effects.push_back(Effect());
  }

  nassertv(_saved_entry == _states->end());
}
