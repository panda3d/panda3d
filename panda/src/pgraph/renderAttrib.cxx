// Filename: renderAttrib.cxx
// Created by:  drose (21Feb02)
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

#include "renderAttrib.h"
#include "bamReader.h"
#include "indent.h"

RenderAttrib::Attribs RenderAttrib::_attribs;
TypeHandle RenderAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::Constructor
//       Access: Protected
//  Description:
////////////////////////////////////////////////////////////////////
RenderAttrib::
RenderAttrib() {
  _saved_entry = _attribs.end();
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
  if (_saved_entry != _attribs.end()) {
    _attribs.erase(_saved_entry);
    _saved_entry = _attribs.end();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderAttrib::
output(ostream &out) const {
  out << get_type();
}

////////////////////////////////////////////////////////////////////
//     Function: RenderAttrib::write
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderAttrib::
write(ostream &out, int indent_level) const {
  indent(out, indent_level) << *this << "\n";
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
  nassertr(attrib->_saved_entry == _attribs.end(), attrib);

  // Save the attrib in a local PointerTo so that it will be freed at
  // the end of this function if no one else uses it.
  CPT(RenderAttrib) pt_attrib = attrib;

  pair<Attribs::iterator, bool> result = _attribs.insert(attrib);
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
//               TransformAttrib) might combine in meaningful ways.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RenderAttrib::
compose_impl(const RenderAttrib *other) const {
  return other;
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
write_datagram(BamWriter *, Datagram &) {
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
//     Function: RenderAttrib::new_from_bam
//       Access: Protected, Static
//  Description: Uniquifies the pointer for a RenderAttrib object just
//               created from a bam file, and preserves its reference
//               count correctly.
////////////////////////////////////////////////////////////////////
TypedWritable *RenderAttrib::
new_from_bam(RenderAttrib *attrib, BamReader *manager) {
  // First, uniquify the pointer.
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
//     Function: RenderAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RenderAttrib.
////////////////////////////////////////////////////////////////////
void RenderAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
}
