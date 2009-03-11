// Filename: renderAttrib.cxx
// Created by:  drose (21Feb02)
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

#include "renderAttrib.h"
#include "bamReader.h"
#include "indent.h"
#include "config_pgraph.h"
#include "lightReMutexHolder.h"

LightReMutex *RenderAttrib::_attribs_lock = NULL;
RenderAttrib::Attribs *RenderAttrib::_attribs = NULL;
TypeHandle RenderAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
RenderAttrib::
RenderAttrib() {
  if (_attribs == (Attribs *)NULL) {
    init_attribs();
  }
  _saved_entry = _attribs->end();

  _always_reissue = false;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::Copy Constructor
//       Access: Private
//  Description: RenderAttribs are not meant to be copied.
////////////////////////////////////////////////////////////////////
RenderAttrib::
RenderAttrib(const RenderAttrib &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::Copy Assignment Operator
//       Access: Private
//  Description: RenderAttribs are not meant to be copied.
////////////////////////////////////////////////////////////////////
void RenderAttrib::
operator = (const RenderAttrib &) {
  nassertv(false);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::Destructor
//       Access: Public, Virtual
//  Description: The destructor is responsible for removing the
//               RenderAttrib from the global set if it is there.
////////////////////////////////////////////////////////////////////
RenderAttrib::
~RenderAttrib() {
  LightReMutexHolder holder(*_attribs_lock);

  // unref() should have cleared this.
  nassertv(_saved_entry == _attribs->end());
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::lower_attrib_can_override
//       Access: Public, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               This should return false if a RenderAttrib on a
//               higher node will compose into a RenderAttrib on a
//               lower node that has a higher override value, or false
//               if the lower RenderAttrib will completely replace the
//               state.
//
//               The default behavior is false: normally, a
//               RenderAttrib in the graph cannot completely override
//               a RenderAttrib above it, regardless of its override
//               value--instead, the two attribs are composed.  But
//               for some kinds of RenderAttribs, it is useful to
//               allow this kind of override.
//
//               This method only handles the one special case of a
//               lower RenderAttrib with a higher override value.  If
//               the higher RenderAttrib has a higher override value,
//               it always completely overrides.  And if both
//               RenderAttribs have the same override value, they are
//               always composed.
////////////////////////////////////////////////////////////////////
bool RenderAttrib::
lower_attrib_can_override() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::has_cull_callback
//       Access: Public, Virtual
//  Description: Should be overridden by derived classes to return
//               true if cull_callback() has been defined.  Otherwise,
//               returns false to indicate cull_callback() does not
//               need to be called for this node during the cull
//               traversal.
////////////////////////////////////////////////////////////////////
bool RenderAttrib::
has_cull_callback() const {
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::cull_callback
//       Access: Public, Virtual
//  Description: If has_cull_callback() returns true, this function
//               will be called during the cull traversal to perform
//               any additional operations that should be performed at
//               cull time.
//
//               This is called each time the RenderAttrib is
//               discovered applied to a Geom in the traversal.  It
//               should return true if the Geom is visible, false if
//               it should be omitted.
////////////////////////////////////////////////////////////////////
bool RenderAttrib::
cull_callback(CullTraverser *, const CullTraverserData &) const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::unref
//       Access: Published
//  Description: This method overrides ReferenceCount::unref() to
//               clear the pointer from the global object pool when
//               its reference count goes to zero.
////////////////////////////////////////////////////////////////////
bool RenderAttrib::
unref() const {
  // We always have to grab the lock, since we will definitely need to
  // be holding it if we happen to drop the reference count to 0.
  LightReMutexHolder holder(*_attribs_lock);

  if (ReferenceCount::unref()) {
    // The reference count is still nonzero.
    return true;
  }

  // The reference count has just reached zero.  Make sure the object
  // is removed from the global object pool, before anyone else finds
  // it and tries to ref it.
  ((RenderAttrib *)this)->release_new();

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::output
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderAttrib::
output(ostream &out) const {
  out << get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::write
//       Access: Published, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderAttrib::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::get_num_attribs
//       Access: Published, Static
//  Description: Returns the total number of unique RenderAttrib
//               objects allocated in the world.  This will go up and
//               down during normal operations.
////////////////////////////////////////////////////////////////////
int RenderAttrib::
get_num_attribs() {
  LightReMutexHolder holder(*_attribs_lock);

  if (_attribs == (Attribs *)NULL) {
    return 0;
  }
  return _attribs->size();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::list_attribs
//       Access: Published, Static
//  Description: Lists all of the RenderAttribs in the cache to the
//               output stream, one per line.  This can be quite a lot
//               of output if the cache is large, so be prepared.
////////////////////////////////////////////////////////////////////
void RenderAttrib::
list_attribs(ostream &out) {
  LightReMutexHolder holder(*_attribs_lock);

  out << _attribs->size() << " attribs:\n";
  Attribs::const_iterator si;
  for (si = _attribs->begin(); si != _attribs->end(); ++si) {
    const RenderAttrib *attrib = (*si);
    attrib->write(out, 2);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::validate_attribs
//       Access: Published, Static
//  Description: Ensures that the cache is still stored in sorted
//               order.  Returns true if so, false if there is a
//               problem (which implies someone has modified one of
//               the supposedly-const RenderAttrib objects).
////////////////////////////////////////////////////////////////////
bool RenderAttrib::
validate_attribs() {
  LightReMutexHolder holder(*_attribs_lock);

  if (_attribs->empty()) {
    return true;
  }

  Attribs::const_iterator si = _attribs->begin();
  Attribs::const_iterator snext = si;
  ++snext;
  while (snext != _attribs->end()) {
    if ((*si)->compare_to(*(*snext)) >= 0) {
      pgraph_cat.error()
        << "RenderAttribs out of order!\n";
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
//     Function: RenderAttrib::return_new
//       Access: Protected, Static
//  Description: This function is used by derived RenderAttrib types
//               to share a common RenderAttrib pointer for all
//               equivalent RenderAttrib objects.
//
//               This is different from return_unique() in that it
//               does not actually guarantee a unique pointer, unless
//               uniquify-attribs is set.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RenderAttrib::
return_new(RenderAttrib *attrib) {
  nassertr(attrib != (RenderAttrib *)NULL, attrib);
  if (!uniquify_attribs) {
    return attrib;
  }

  return return_unique(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::return_unique
//       Access: Protected, Static
//  Description: This function is used by derived RenderAttrib types
//               to share a common RenderAttrib pointer for all
//               equivalent RenderAttrib objects.
//
//               The make() function of the derived type should create
//               a new RenderAttrib and pass it through return_new(),
//               which will either save the pointer and return it
//               unchanged (if this is the first similar such object)
//               or delete it and return an equivalent pointer (if
//               there was already a similar object saved).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RenderAttrib::
return_unique(RenderAttrib *attrib) {
  nassertr(attrib != (RenderAttrib *)NULL, attrib);

#ifndef NDEBUG
  if (!state_cache) {
    return attrib;
  }
#endif

#ifndef NDEBUG
  if (paranoid_const) {
    nassertr(validate_attribs(), attrib);
  }
#endif

  LightReMutexHolder holder(*_attribs_lock);

  if (attrib->_saved_entry != _attribs->end()) {
    // This attrib is already in the cache.
    nassertr(_attribs->find(attrib) == attrib->_saved_entry, attrib);
    return attrib;
  }

  // Save the attrib in a local PointerTo so that it will be freed at
  // the end of this function if no one else uses it.
  CPT(RenderAttrib) pt_attrib = attrib;

  pair<Attribs::iterator, bool> result = _attribs->insert(attrib);
  if (result.second) {
    // The attribute was inserted; save the iterator and return the
    // input attribute.
    attrib->_saved_entry = result.first;

    return pt_attrib;
  }

  // The attribute was not inserted; there must be an equivalent one
  // already in the set.  Return that one.
  return *(result.first);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to return a unique number indicating whether
//               this RenderAttrib is equivalent to the other one.
//
//               This should return 0 if the two RenderAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two RenderAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int RenderAttrib::
compare_to_impl(const RenderAttrib *other) const {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               This should return the result of applying the other
//               RenderAttrib to a node in the scene graph below this
//               RenderAttrib, which was already applied.  In most
//               cases, the result is the same as the other
//               RenderAttrib (that is, a subsequent RenderAttrib
//               completely replaces the preceding one).  On the other
//               hand, some kinds of RenderAttrib (for instance,
//               ColorTransformAttrib) might combine in meaningful
//               ways.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RenderAttrib::
compose_impl(const RenderAttrib *other) const {
  return other;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RenderAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  return other;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::output_comparefunc
//       Access: Protected
//  Description: Outputs a string representation of the given
//               PandaCompareFunc object.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::release_new
//       Access: Private
//  Description: This inverse of return_new, this releases this object
//               from the global RenderAttrib table.
//
//               You must already be holding _attribs_lock before you
//               call this method.
////////////////////////////////////////////////////////////////////
void RenderAttrib::
release_new() {
  nassertv(_attribs_lock->debug_is_locked());

  if (_saved_entry != _attribs->end()) {

#ifndef NDEBUG
    nassertd(_attribs->find(this) == _saved_entry) {
      cerr << "Tried to release " << *this << " (" << (void *)this << "), not found!\n";
      validate_attribs();
      Attribs::const_iterator si = _attribs->begin();
      if (si == _attribs->end()) {
        cerr << "  Attribs list is empty.\n";
      } else {
        cerr << "  Attribs list contains " << _attribs->size() << " entries.\n";
        const RenderAttrib *attrib = (*si);
        cerr << "    " << *attrib << " (" << (void *)attrib << ")\n";

        Attribs::const_iterator sprev = si;
        ++si;
        while (si != _attribs->end()) {
          const RenderAttrib *attrib = (*si);
          cerr << "    " << *attrib << " (" << (void *)attrib << ")\n";
          if (((*sprev)->compare_to(*attrib)) >= 0) {
            cerr << "*** above out of order!\n";
          }
          sprev = si;
          ++si;
        }
        cerr << "  Done.\n";
      }
    }
#endif  // NDEBUG

    _attribs->erase(_saved_entry);
    _saved_entry = _attribs->end();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::init_attribs
//       Access: Public, Static
//  Description: Make sure the global _attribs map is allocated.  This
//               only has to be done once.  We could make this map
//               static, but then we run into problems if anyone
//               creates a RenderAttrib object at static init time;
//               it also seems to cause problems when the Panda shared
//               library is unloaded at application exit time.
////////////////////////////////////////////////////////////////////
void RenderAttrib::
init_attribs() {
  _attribs = new Attribs;

  // TODO: we should have a global Panda mutex to allow us to safely
  // create _attribs_lock without a startup race condition.  For the
  // meantime, this is OK because we guarantee that this method is
  // called at static init time, presumably when there is still only
  // one thread in the world.
  _attribs_lock = new LightReMutex("RenderAttrib::_attribs_lock");
  nassertv(Thread::get_current_thread() == Thread::get_main_thread());
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void RenderAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  TypedWritable::write_datagram(manager, dg);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::change_this
//       Access: Public, Static
//  Description: Called immediately after complete_pointers(), this
//               gives the object a chance to adjust its own pointer
//               if desired.  Most objects don't change pointers after
//               completion, but some need to.
//
//               Once this function has been called, the old pointer
//               will no longer be accessed.
////////////////////////////////////////////////////////////////////
TypedWritable *RenderAttrib::
change_this(TypedWritable *old_ptr, BamReader *manager) {
  // First, uniquify the pointer.
  RenderAttrib *attrib = DCAST(RenderAttrib, old_ptr);
  CPT(RenderAttrib) pointer = return_unique(attrib);

  // But now we have a problem, since we have to hold the reference
  // count and there's no way to return a TypedWritable while still
  // holding the reference count!  We work around this by explicitly
  // upping the count, and also setting a finalize() callback to down
  // it later.
  if (pointer == attrib) {
    pointer->ref();
    manager->register_finalize(attrib);
  }
  
  // We have to cast the pointer back to non-const, because the bam
  // reader expects that.
  return (RenderAttrib *)pointer.p();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::finalize
//       Access: Public, Virtual
//  Description: Called by the BamReader to perform any final actions
//               needed for setting up the object after all objects
//               have been read and all pointers have been completed.
////////////////////////////////////////////////////////////////////
void RenderAttrib::
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
//     Function: RenderAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RenderAttrib.
////////////////////////////////////////////////////////////////////
void RenderAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  TypedWritable::fillin(scan, manager);
  manager->register_change_this(change_this, this);
}
