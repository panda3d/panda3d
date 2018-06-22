/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderEffect.cxx
 * @author drose
 * @date 2002-03-14
 */

#include "renderEffect.h"
#include "bamReader.h"
#include "indent.h"
#include "config_pgraph.h"

RenderEffect::Effects *RenderEffect::_effects = nullptr;
TypeHandle RenderEffect::_type_handle;

/**
 *
 */
RenderEffect::
RenderEffect() {
  if (_effects == nullptr) {
    // Make sure the global _effects map is allocated.  This only has to be
    // done once.  We could make this map static, but then we run into
    // problems if anyone creates a RenderState object at static init time; it
    // also seems to cause problems when the Panda shared library is unloaded
    // at application exit time.
    _effects = new Effects;
  }
  _saved_entry = _effects->end();
}

/**
 * The destructor is responsible for removing the RenderEffect from the global
 * set if it is there.
 */
RenderEffect::
~RenderEffect() {
  if (_saved_entry != _effects->end()) {
    // We cannot make this assertion, because the RenderEffect has already
    // partially destructed--this means we cannot look up the object in the
    // map.  In fact, the map is temporarily invalid until we finish
    // destructing, since we screwed up the ordering when we changed the
    // return value of get_type(). nassertv(_effects->find(this) ==
    // _saved_entry);

    // Note: this isn't thread-safe, because once the derived class destructor
    // exits and before this destructor completes, the map is invalid, and
    // other threads may inadvertently attempt to read the invalid map.  To
    // make it thread-safe, we need to move this functionality to a separate
    // method, that is to be called from *each* derived class's destructor
    // (and then we can put the above assert back in).
    _effects->erase(_saved_entry);
    _saved_entry = _effects->end();
  }
}

/**
 * Returns true if it is generally safe to transform this particular kind of
 * RenderEffect by calling the xform() method, false otherwise.
 */
bool RenderEffect::
safe_to_transform() const {
  return true;
}

/**
 * Preprocesses the accumulated transform that is about to be applied to (or
 * through) this node due to a flatten operation.  The returned value will be
 * used instead.
 */
CPT(TransformState) RenderEffect::
prepare_flatten_transform(const TransformState *net_transform) const {
  return net_transform;
}

/**
 * Returns true if this kind of effect can safely be combined with sibling
 * nodes that share the exact same effect, or false if this is not a good
 * idea.
 */
bool RenderEffect::
safe_to_combine() const {
  return true;
}

/**
 * Returns a new RenderEffect transformed by the indicated matrix.
 */
CPT(RenderEffect) RenderEffect::
xform(const LMatrix4 &) const {
  return this;
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this effect during the cull traversal.
 */
bool RenderEffect::
has_cull_callback() const {
  return false;
}

/**
 * If has_cull_callback() returns true, this function will be called during
 * the cull traversal to perform any additional operations that should be
 * performed at cull time.  This may include additional manipulation of render
 * state or additional visible/invisible decisions, or any other arbitrary
 * operation.
 *
 * At the time this function is called, the current node's transform and state
 * have not yet been applied to the net_transform and net_state.  This
 * callback may modify the node_transform and node_state to apply an effective
 * change to the render state at this level.
 */
void RenderEffect::
cull_callback(CullTraverser *, CullTraverserData &,
              CPT(TransformState) &, CPT(RenderState) &) const {
}

/**
 * Should be overridden by derived classes to return true if
 * adjust_transform() has been defined, and therefore the RenderEffect has
 * some effect on the node's apparent local and net transforms.
 */
bool RenderEffect::
has_adjust_transform() const {
  return false;
}

/**
 * Performs some operation on the node's apparent net and/or local transforms.
 * This will only be called if has_adjust_transform() is redefined to return
 * true.
 *
 * Both parameters are in/out.  The original transforms will be passed in, and
 * they may (or may not) be modified in-place by the RenderEffect.
 */
void RenderEffect::
adjust_transform(CPT(TransformState) &, CPT(TransformState) &,
                 const PandaNode *) const {
}

/**
 *
 */
void RenderEffect::
output(std::ostream &out) const {
  out << get_type();
}

/**
 *
 */
void RenderEffect::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

/**
 * Returns the total number of unique RenderEffect objects allocated in the
 * world.  This will go up and down during normal operations.
 */
int RenderEffect::
get_num_effects() {
  if (_effects == nullptr) {
    return 0;
  }
  return _effects->size();
}

/**
 * Lists all of the RenderEffects in the cache to the output stream, one per
 * line.  This can be quite a lot of output if the cache is large, so be
 * prepared.
 */
void RenderEffect::
list_effects(std::ostream &out) {
  out << _effects->size() << " effects:\n";
  Effects::const_iterator si;
  for (si = _effects->begin(); si != _effects->end(); ++si) {
    const RenderEffect *effect = (*si);
    effect->write(out, 2);
  }
}

/**
 * Ensures that the cache is still stored in sorted order.  Returns true if
 * so, false if there is a problem (which implies someone has modified one of
 * the supposedly-const RenderEffect objects).
 */
bool RenderEffect::
validate_effects() {
  if (_effects->empty()) {
    return true;
  }

  Effects::const_iterator si = _effects->begin();
  Effects::const_iterator snext = si;
  ++snext;
  while (snext != _effects->end()) {
    if ((*si)->compare_to(*(*snext)) >= 0) {
      pgraph_cat.error()
        << "RenderEffects out of order!\n";
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
 * This function is used by derived RenderEffect types to share a common
 * RenderEffect pointer for all equivalent RenderEffect objects.
 *
 * The make() function of the derived type should create a new RenderEffect
 * and pass it through return_new(), which will either save the pointer and
 * return it unchanged (if this is the first similar such object) or delete it
 * and return an equivalent pointer (if there was already a similar object
 * saved).
 */
CPT(RenderEffect) RenderEffect::
return_new(RenderEffect *effect) {
  nassertr(effect != nullptr, effect);

  // This should be a newly allocated pointer, not one that was used for
  // anything else.
  nassertr(effect->_saved_entry == _effects->end(), effect);

#ifndef NDEBUG
  if (paranoid_const) {
    nassertr(validate_effects(), effect);
  }
#endif

  // Save the effect in a local PointerTo so that it will be freed at the end
  // of this function if no one else uses it.
  CPT(RenderEffect) pt_effect = effect;

  std::pair<Effects::iterator, bool> result = _effects->insert(effect);
  if (result.second) {
    // The effect was inserted; save the iterator and return the input effect.
    effect->_saved_entry = result.first;
    return pt_effect;
  }

  // The effect was not inserted; there must be an equivalent one already in
  // the set.  Return that one.
  return *(result.first);
}

/**
 * Intended to be overridden by derived RenderEffect types to return a unique
 * number indicating whether this RenderEffect is equivalent to the other one.
 *
 * This should return 0 if the two RenderEffect objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two RenderEffect objects whose get_type()
 * functions return the same.
 */
int RenderEffect::
compare_to_impl(const RenderEffect *other) const {
  return 0;
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void RenderEffect::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
}

/**
 * Called immediately after complete_pointers(), this gives the object a
 * chance to adjust its own pointer if desired.  Most objects don't change
 * pointers after completion, but some need to.
 *
 * Once this function has been called, the old pointer will no longer be
 * accessed.
 */
TypedWritable *RenderEffect::
change_this(TypedWritable *old_ptr, BamReader *manager) {
  // First, uniquify the pointer.
  RenderEffect *effect = DCAST(RenderEffect, old_ptr);
  CPT(RenderEffect) pointer = return_new(effect);

  // But now we have a problem, since we have to hold the reference count and
  // there's no way to return a TypedWritable while still holding the
  // reference count!  We work around this by explicitly upping the count, and
  // also setting a finalize() callback to down it later.
  if (pointer == effect) {
    pointer->ref();
    manager->register_finalize(effect);
  }

  // We have to cast the pointer back to non-const, because the bam reader
  // expects that.
  return (RenderEffect *)pointer.p();
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void RenderEffect::
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
 * This internal function is called by make_from_bam to read in all of the
 * relevant data from the BamFile for the new RenderEffect.
 */
void RenderEffect::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
}
