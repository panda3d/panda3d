/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file renderAttrib.cxx
 * @author drose
 * @date 2002-02-21
 */

#include "renderAttrib.h"
#include "bamReader.h"
#include "indent.h"
#include "config_pgraph.h"
#include "lightReMutexHolder.h"
#include "pStatTimer.h"

using std::ostream;

LightReMutex *RenderAttrib::_attribs_lock = nullptr;
RenderAttrib::Attribs RenderAttrib::_attribs;
TypeHandle RenderAttrib::_type_handle;

size_t RenderAttrib::_garbage_index = 0;

PStatCollector RenderAttrib::_garbage_collect_pcollector("*:State Cache:Garbage Collect");

/**
 *
 */
RenderAttrib::
RenderAttrib() {
  if (_attribs_lock == nullptr) {
    init_attribs();
  }
  _saved_entry = -1;
}

/**
 * The destructor is responsible for removing the RenderAttrib from the global
 * set if it is there.
 */
RenderAttrib::
~RenderAttrib() {
  // unref() should have cleared this.
  nassertv(_saved_entry == -1);
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * This should return false if a RenderAttrib on a higher node will compose
 * into a RenderAttrib on a lower node that has a higher override value, or
 * true if the lower RenderAttrib will completely replace the state.
 *
 * The default behavior is false: normally, a RenderAttrib in the graph cannot
 * completely override a RenderAttrib above it, regardless of its override
 * value--instead, the two attribs are composed.  But for some kinds of
 * RenderAttribs, it is useful to allow this kind of override.
 *
 * This method only handles the one special case of a lower RenderAttrib with
 * a higher override value.  If the higher RenderAttrib has a higher override
 * value, it always completely overrides.  And if both RenderAttribs have the
 * same override value, they are always composed.
 */
bool RenderAttrib::
lower_attrib_can_override() const {
  return false;
}

/**
 * Should be overridden by derived classes to return true if cull_callback()
 * has been defined.  Otherwise, returns false to indicate cull_callback()
 * does not need to be called for this node during the cull traversal.
 */
bool RenderAttrib::
has_cull_callback() const {
  return false;
}

/**
 * If has_cull_callback() returns true, this function will be called during
 * the cull traversal to perform any additional operations that should be
 * performed at cull time.
 *
 * This is called each time the RenderAttrib is discovered applied to a Geom
 * in the traversal.  It should return true if the Geom is visible, false if
 * it should be omitted.
 */
bool RenderAttrib::
cull_callback(CullTraverser *, const CullTraverserData &) const {
  return true;
}

/**
 * This method overrides ReferenceCount::unref() to clear the pointer from the
 * global object pool when its reference count goes to zero.
 */
bool RenderAttrib::
unref() const {
  if (!state_cache || garbage_collect_states) {
    // If we're not using the cache at all, or if we're relying on garbage
    // collection, just allow the pointer to unref normally.
    return ReferenceCount::unref();
  }

  // Here is the normal refcounting case, with a normal cache, and without
  // garbage collection in effect.  In this case we will pull the object out
  // of the cache when its reference count goes to 0.

  // We always have to grab the lock, since we will definitely need to be
  // holding it if we happen to drop the reference count to 0. Having to grab
  // the lock at every call to unref() is a big limiting factor on
  // parallelization.
  LightReMutexHolder holder(*_attribs_lock);

  if (ReferenceCount::unref()) {
    // The reference count is still nonzero.
    return true;
  }

  // The reference count has just reached zero.  Make sure the object is
  // removed from the global object pool, before anyone else finds it and
  // tries to ref it.
  ((RenderAttrib *)this)->release_new();

  return false;
}

/**
 *
 */
void RenderAttrib::
output(ostream &out) const {
  out << get_type();
}

/**
 *
 */
void RenderAttrib::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

/**
 * Returns the total number of unique RenderAttrib objects allocated in the
 * world.  This will go up and down during normal operations.
 */
int RenderAttrib::
get_num_attribs() {
  LightReMutexHolder holder(*_attribs_lock);
  return _attribs.get_num_entries();
}

/**
 * Lists all of the RenderAttribs in the cache to the output stream, one per
 * line.  This can be quite a lot of output if the cache is large, so be
 * prepared.
 */
void RenderAttrib::
list_attribs(ostream &out) {
  LightReMutexHolder holder(*_attribs_lock);

  size_t size = _attribs.get_num_entries();
  out << size << " attribs:\n";
  for (size_t si = 0; si < size; ++si) {
    const RenderAttrib *attrib = _attribs.get_key(si);
    attrib->write(out, 2);
  }
}

/**
 * Performs a garbage-collection cycle.  This is called automatically from
 * RenderState::garbage_collect(); see that method for more information.
 */
int RenderAttrib::
garbage_collect() {
  if (!garbage_collect_states) {
    return 0;
  }
  LightReMutexHolder holder(*_attribs_lock);

  PStatTimer timer(_garbage_collect_pcollector);
  size_t orig_size = _attribs.get_num_entries();

#ifdef _DEBUG
  nassertr(_attribs.validate(), 0);
#endif

  // How many elements to process this pass?
  size_t size = orig_size;
  size_t num_this_pass = std::max(0, int(size * garbage_collect_states_rate));
  if (num_this_pass <= 0) {
    return 0;
  }

  size_t si = _garbage_index;
  if (si >= size) {
    si = 0;
  }

  num_this_pass = std::min(num_this_pass, size);
  size_t stop_at_element = (si + num_this_pass) % size;

  do {
    RenderAttrib *attrib = (RenderAttrib *)_attribs.get_key(si);
    if (attrib->get_ref_count() == 1) {
      // This attrib has recently been unreffed to 1 (the one we added when
      // we stored it in the cache).  Now it's time to delete it.  This is
      // safe, because we're holding the _attribs_lock, so it's not possible
      // for some other thread to find the attrib in the cache and ref it
      // while we're doing this.
      attrib->release_new();
      unref_delete(attrib);

      // When we removed it from the hash map, it swapped the last element
      // with the one we just removed.  So the current index contains one we
      // still need to visit.
      --size;
      --si;
      if (stop_at_element > 0) {
        --stop_at_element;
      }
    }

    si = (si + 1) % size;
  } while (si != stop_at_element);
  _garbage_index = si;

  nassertr(_attribs.get_num_entries() == size, 0);

#ifdef _DEBUG
  nassertr(_attribs.validate(), 0);
#endif

  // If we just cleaned up a lot of attribs, see if we can reduce the table in
  // size.  This will help reduce iteration overhead in the future.
  _attribs.consider_shrink_table();

  return (int)orig_size - (int)size;
}

/**
 * Ensures that the cache is still stored in sorted order.  Returns true if
 * so, false if there is a problem (which implies someone has modified one of
 * the supposedly-const RenderAttrib objects).
 */
bool RenderAttrib::
validate_attribs() {
  LightReMutexHolder holder(*_attribs_lock);
  if (_attribs.is_empty()) {
    return true;
  }

  if (!_attribs.validate()) {
    pgraph_cat.error()
      << "RenderAttrib::_attribs cache is invalid!\n";

    size_t size = _attribs.get_num_entries();
    for (size_t si = 0; si < size; ++si) {
      const RenderAttrib *attrib = _attribs.get_key(si);
      //cerr << si << ": " << attrib << "\n";
      attrib->write(std::cerr, 2);
    }

    return false;
  }

  size_t size = _attribs.get_num_entries();
  size_t si = 0;
  nassertr(si < size, false);
  nassertr(_attribs.get_key(si)->get_ref_count() >= 0, false);
  size_t snext = si;
  ++snext;
  while (snext < size) {
    nassertr(_attribs.get_key(snext)->get_ref_count() >= 0, false);
    const RenderAttrib *ssi = _attribs.get_key(si);
    const RenderAttrib *ssnext = _attribs.get_key(snext);
    int c = ssi->compare_to(*ssnext);
    int ci = ssnext->compare_to(*ssi);
    if ((ci < 0) != (c > 0) ||
        (ci > 0) != (c < 0) ||
        (ci == 0) != (c == 0)) {
      pgraph_cat.error()
        << "RenderAttrib::compare_to() not defined properly!\n";
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
  }

  return true;
}

/**
 * This function is used by derived RenderAttrib types to share a common
 * RenderAttrib pointer for all equivalent RenderAttrib objects.
 *
 * This is different from return_unique() in that it does not actually
 * guarantee a unique pointer, unless uniquify-attribs is set.
 */
CPT(RenderAttrib) RenderAttrib::
return_new(RenderAttrib *attrib) {
  nassertr(attrib != nullptr, attrib);
  if (!uniquify_attribs) {
    attrib->calc_hash();
    return attrib;
  }

  return return_unique(attrib);
}

/**
 * This function is used by derived RenderAttrib types to share a common
 * RenderAttrib pointer for all equivalent RenderAttrib objects.
 *
 * The make() function of the derived type should create a new RenderAttrib
 * and pass it through return_new(), which will either save the pointer and
 * return it unchanged (if this is the first similar such object) or delete it
 * and return an equivalent pointer (if there was already a similar object
 * saved).
 */
CPT(RenderAttrib) RenderAttrib::
return_unique(RenderAttrib *attrib) {
  nassertr(attrib != nullptr, attrib);

  attrib->calc_hash();

  if (!state_cache) {
    return attrib;
  }

#ifndef NDEBUG
  if (paranoid_const) {
    nassertr(validate_attribs(), attrib);
  }
#endif

  LightReMutexHolder holder(*_attribs_lock);

  if (attrib->_saved_entry != -1) {
    // This attrib is already in the cache.  nassertr(_attribs.find(attrib)
    // == attrib->_saved_entry, attrib);
    return attrib;
  }

  int si = _attribs.find(attrib);
  if (si != -1) {
    // There's an equivalent attrib already in the set.  Return it.  If this
    // is a newly created RenderAttrib, though, be sure to delete it.
    if (attrib->get_ref_count() == 0) {
      delete attrib;
    }
    return _attribs.get_key(si);
  }

  // Not already in the set; add it.
  if (garbage_collect_states) {
    // If we'll be garbage collecting attribs explicitly, we'll increment the
    // reference count when we store it in the cache, so that it won't be
    // deleted while it's in it.
    attrib->ref();
  }
  si = _attribs.store(attrib, nullptr);

  // Save the index and return the input attrib.
  attrib->_saved_entry = si;
  return attrib;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * number indicating whether this RenderAttrib is equivalent to the other one.
 *
 * This should return 0 if the two RenderAttrib objects are equivalent, a
 * number less than zero if this one should be sorted before the other one,
 * and a number greater than zero otherwise.
 *
 * This will only be called with two RenderAttrib objects whose get_type()
 * functions return the same.
 */
int RenderAttrib::
compare_to_impl(const RenderAttrib *other) const {
  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to return a unique
 * hash for these particular properties.  RenderAttribs that compare the same
 * with compare_to_impl(), above, should return the same hash; RenderAttribs
 * that compare differently should return a different hash.
 */
size_t RenderAttrib::
get_hash_impl() const {
  return 0;
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * This should return the result of applying the other RenderAttrib to a node
 * in the scene graph below this RenderAttrib, which was already applied.  In
 * most cases, the result is the same as the other RenderAttrib (that is, a
 * subsequent RenderAttrib completely replaces the preceding one).  On the
 * other hand, some kinds of RenderAttrib (for instance, ColorTransformAttrib)
 * might combine in meaningful ways.
 */
CPT(RenderAttrib) RenderAttrib::
compose_impl(const RenderAttrib *other) const {
  return other;
}

/**
 * Intended to be overridden by derived RenderAttrib types to specify how two
 * consecutive RenderAttrib objects of the same type interact.
 *
 * See invert_compose() and compose_impl().
 */
CPT(RenderAttrib) RenderAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  return other;
}

/**
 * Outputs a string representation of the given PandaCompareFunc object.
 */
void RenderAttrib::
output_comparefunc(ostream &out, PandaCompareFunc fn) const {
  switch (fn) {
  case M_none:
    out << "none";
    break;

  case M_never:
    out << "never";
    break;

  case M_less:
    out << "less";
    break;

  case M_equal:
    out << "equal";
    break;

  case M_less_equal:
    out << "less_equal";
    break;

  case M_greater:
    out << "greater";
    break;

  case M_not_equal:
    out << "not_equal";
    break;

  case M_greater_equal:
    out << "greater_equal";
    break;

  case M_always:
    out << "always";
    break;
  }
}

/**
 * This inverse of return_new, this releases this object from the global
 * RenderAttrib table.
 *
 * You must already be holding _attribs_lock before you call this method.
 */
void RenderAttrib::
release_new() {
  nassertv(_attribs_lock->debug_is_locked());

  if (_saved_entry != -1) {
    _saved_entry = -1;
    nassertv_always(_attribs.remove(this));
  }
}

/**
 * Make sure the global _attribs map is allocated.  This only has to be done
 * once.  We could make this map static, but then we run into problems if
 * anyone creates a RenderAttrib object at static init time; it also seems to
 * cause problems when the Panda shared library is unloaded at application
 * exit time.
 */
void RenderAttrib::
init_attribs() {
  // TODO: we should have a global Panda mutex to allow us to safely create
  // _attribs_lock without a startup race condition.  For the meantime, this
  // is OK because we guarantee that this method is called at static init
  // time, presumably when there is still only one thread in the world.
  _attribs_lock = new LightReMutex("RenderAttrib::_attribs_lock");
  nassertv(Thread::get_current_thread() == Thread::get_main_thread());
}

/**
 * Writes the contents of this object to the datagram for shipping out to a
 * Bam file.
 */
void RenderAttrib::
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
TypedWritable *RenderAttrib::
change_this(TypedWritable *old_ptr, BamReader *manager) {
  // First, uniquify the pointer.
  RenderAttrib *attrib = DCAST(RenderAttrib, old_ptr);
  CPT(RenderAttrib) pointer = return_unique(attrib);

  // But now we have a problem, since we have to hold the reference count and
  // there's no way to return a TypedWritable while still holding the
  // reference count!  We work around this by explicitly upping the count, and
  // also setting a finalize() callback to down it later.
  if (pointer == attrib) {
    pointer->ref();
    manager->register_finalize(attrib);
  }

  // We have to cast the pointer back to non-const, because the bam reader
  // expects that.
  return (RenderAttrib *)pointer.p();
}

/**
 * Called by the BamReader to perform any final actions needed for setting up
 * the object after all objects have been read and all pointers have been
 * completed.
 */
void RenderAttrib::
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
 * relevant data from the BamFile for the new RenderAttrib.
 */
void RenderAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  manager->register_change_this(change_this, this);
}
