// Filename: renderAttrib.cxx
// Created by:  drose (21Feb02)
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

#include "renderAttrib.h"
#include "bamReader.h"
#include "indent.h"

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
    // Make sure the global _attribs map is allocated.  This only has
    // to be done once.  We could make this map static, but then we
    // run into problems if anyone creates a RenderState object at
    // static init time; it also seems to cause problems when the
    // Panda shared library is unloaded at application exit time.
    _attribs = new Attribs;
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
  if (_saved_entry != _attribs->end()) {
    // We cannot make this assertion, because the RenderAttrib has
    // already partially destructed--this means we cannot look up the
    // object in the map.  In fact, the map is temporarily invalid
    // until we finish destructing, since we screwed up the ordering
    // when we changed the return value of get_type().
    //    nassertv(_attribs->find(this) == _saved_entry);

    // Note: this isn't thread-safe, because once the derived class
    // destructor exits and before this destructor completes, the map
    // is invalid, and other threads may inadvertently attempt to read
    // the invalid map.  To make it thread-safe, we need to move this
    // functionality to a separate method, that is to be called from
    // *each* derived class's destructor (and then we can put the
    // above assert back in).
    _attribs->erase(_saved_entry);
    _saved_entry = _attribs->end();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void RenderAttrib::
issue(GraphicsStateGuardianBase *) const {
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
//               The make() function of the derived type should create
//               a new RenderAttrib and pass it through return_new(),
//               which will either save the pointer and return it
//               unchanged (if this is the first similar such object)
//               or delete it and return an equivalent pointer (if
//               there was already a similar object saved).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RenderAttrib::
return_new(RenderAttrib *attrib) {
  nassertr(attrib != (RenderAttrib *)NULL, attrib);

  // This should be a newly allocated pointer, not one that was used
  // for anything else.
  nassertr(attrib->_saved_entry == _attribs->end(), attrib);

#ifndef NDEBUG
  if (paranoid_const) {
    nassertr(validate_attribs(), attrib);
  }
#endif

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
//     Function: RenderAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify what the default property for a
//               RenderAttrib of this type should be.
//
//               This should return a newly-allocated RenderAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of RenderAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *RenderAttrib::
make_default_impl() const {
  return (RenderAttrib *)NULL;
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
  CPT(RenderAttrib) pointer = return_new(attrib);

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
//  Description: Method to ensure that any necessary clean up tasks
//               that have to be performed by this object are performed
////////////////////////////////////////////////////////////////////
void RenderAttrib::
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
