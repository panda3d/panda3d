// Filename: clipPlaneAttrib.cxx
// Created by:  drose (11Jul02)
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

#include "clipPlaneAttrib.h"
#include "pandaNode.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle ClipPlaneAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make_all_off
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that turns off
//               all planes (and hence disables planeing).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make_all_off() {
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib;
  attrib->_operation = O_set;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that turns on (or
//               off, according to op) the indicate plane(s).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make(ClipPlaneAttrib::Operation op, PlaneNode *plane) {
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib;
  attrib->_operation = op;
  attrib->_planes.push_back(plane);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that turns on (or
//               off, according to op) the indicate plane(s).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make(ClipPlaneAttrib::Operation op, PlaneNode *plane1, PlaneNode *plane2) {
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib;
  attrib->_operation = op;
  attrib->_planes.push_back(plane1);
  attrib->_planes.push_back(plane2);

  attrib->_planes.sort();
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that turns on (or
//               off, according to op) the indicate plane(s).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make(ClipPlaneAttrib::Operation op, PlaneNode *plane1, PlaneNode *plane2,
     PlaneNode *plane3) {
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib;
  attrib->_operation = op;
  attrib->_planes.push_back(plane1);
  attrib->_planes.push_back(plane2);
  attrib->_planes.push_back(plane3);

  attrib->_planes.sort();
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ClipPlaneAttrib object that turns on (or
//               off, according to op) the indicate plane(s).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
make(ClipPlaneAttrib::Operation op, PlaneNode *plane1, PlaneNode *plane2,
     PlaneNode *plane3, PlaneNode *plane4) {
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib;
  attrib->_operation = op;
  attrib->_planes.push_back(plane1);
  attrib->_planes.push_back(plane2);
  attrib->_planes.push_back(plane3);
  attrib->_planes.push_back(plane4);

  attrib->_planes.sort();
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::has_plane
//       Access: Published
//  Description: Returns true if the indicated plane is listed in the
//               attrib, false otherwise.
////////////////////////////////////////////////////////////////////
bool ClipPlaneAttrib::
has_plane(PlaneNode *plane) const {
  return _planes.find(plane) != _planes.end();
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void ClipPlaneAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_clip_plane(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ClipPlaneAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (_operation == O_set && _planes.empty()) {
    out << "all off";
  } else {
    switch (_operation) {
    case O_set:
      out << "set";
      break;
    case O_add:
      out << "add";
      break;
    case O_remove:
      out << "remove";
      break;
    }

    Planes::const_iterator li;
    for (li = _planes.begin(); li != _planes.end(); ++li) {
      PlaneNode *plane = (*li);
      out << " " << *plane;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ClipPlaneAttrib
//               types to return a unique number indicating whether
//               this ClipPlaneAttrib is equivalent to the other one.
//
//               This should return 0 if the two ClipPlaneAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two ClipPlaneAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int ClipPlaneAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ClipPlaneAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_operation != ta->_operation) {
    return (int)_operation - (int)ta->_operation;
  }

  Planes::const_iterator li = _planes.begin();
  Planes::const_iterator oli = ta->_planes.begin();

  while (li != _planes.end() && oli != ta->_planes.end()) {
    PlaneNode *plane = (*li);
    PlaneNode *other_plane = (*oli);

    if (plane != other_plane) {
      return plane < other_plane ? -1 : 1;
    }

    ++li;
    ++oli;
  }

  if (li != _planes.end()) {
    return 1;
  }
  if (oli != ta->_planes.end()) {
    return -1;
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::compose_impl
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
CPT(RenderAttrib) ClipPlaneAttrib::
compose_impl(const RenderAttrib *other) const {
  const ClipPlaneAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (ta->_operation == O_set) {
    // If the other type is O_set, it doesn't matter what we are.
    return ta;
  }

  if (_operation == ta->_operation) {
    // If the operation types match, the composition is simply the
    // union.
    return do_add(ta, _operation);

  } else if (ta->_operation == O_remove) {
    // If the other operation type is remove, and our type is add or
    // set, then remove.
    return do_remove(ta, _operation);

  } else if (_operation == O_remove) {
    // If our type is remove, then the other one wins.
    return ta;

  } else {
    // Otherwise, the result is the union.
    return do_add(ta, _operation);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  // I think in this case the other attrib always wins.  Maybe this
  // needs a bit more thought.  It's hard to imagine that it's even
  // important to compute this properly.
  return other;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ClipPlaneAttrib
//               types to specify what the default property for a
//               ClipPlaneAttrib of this type should be.
//
//               This should return a newly-allocated ClipPlaneAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of ClipPlaneAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *ClipPlaneAttrib::
make_default_impl() const {
  return new ClipPlaneAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::do_add
//       Access: Private
//  Description: Returns a new ClipPlaneAttrib that represents all the
//               planes of this attrib, with those of the other one
//               added in.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
do_add(const ClipPlaneAttrib *other, ClipPlaneAttrib::Operation op) const {
  Planes::const_iterator ai = _planes.begin();
  Planes::const_iterator bi = other->_planes.begin();

  // Create a new ClipPlaneAttrib that will hold the result.
  ClipPlaneAttrib *new_attrib = new ClipPlaneAttrib;
  new_attrib->_operation = op;
  back_insert_iterator<Planes> result = 
    back_inserter(new_attrib->_planes);

  while (ai != _planes.end() && bi != other->_planes.end()) {
    if ((*ai) < (*bi)) {
      // Here is a plane that we have in the original, which is not
      // present in the secondary.
      *result = *ai;
      ++ai;
      ++result;
    } else if ((*bi) < (*ai)) {
      // Here is a new plane we have in the secondary, that was not
      // present in the original.
      *result = *bi;
      ++bi;
      ++result;
    } else {
      // Here is a plane we have in both.
      *result = *ai;
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _planes.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  while (bi != other->_planes.end()) {
    *result = *bi;
    ++bi;
    ++result;
  }

  return return_new(new_attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::do_remove
//       Access: Private
//  Description: Returns a new ClipPlaneAttrib that represents all the
//               planes of this attrib, with those of the other one
//               removed.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ClipPlaneAttrib::
do_remove(const ClipPlaneAttrib *other, ClipPlaneAttrib::Operation op) const {
  Planes::const_iterator ai = _planes.begin();
  Planes::const_iterator bi = other->_planes.begin();

  // Create a new ClipPlaneAttrib that will hold the result.
  ClipPlaneAttrib *new_attrib = new ClipPlaneAttrib;
  new_attrib->_operation = op;
  back_insert_iterator<Planes> result = 
    back_inserter(new_attrib->_planes);

  while (ai != _planes.end() && bi != other->_planes.end()) {
    if ((*ai) < (*bi)) {
      // Here is a plane that we have in the original, which is
      // not present in the secondary.  Keep it.
      *result = *ai;
      ++ai;
      ++result;
    } else if ((*bi) < (*ai)) {
      // Here is a new plane we have in the secondary, that was
      // not present in the original.  Ignore it.
      ++bi;
    } else {
      // Here is a plane we have in both.  Drop it.
      ++ai;
      ++bi;
    }
  }

  while (ai != _planes.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  return return_new(new_attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ClipPlaneAttrib.
////////////////////////////////////////////////////////////////////
void ClipPlaneAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ClipPlaneAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8((int)_operation);
  PN_uint16 num_planes = _planes.size();
  nassertv(num_planes == _planes.size());
  dg.add_uint16(num_planes);

  Planes::const_iterator li;
  for (li = _planes.begin(); li != _planes.end(); ++li) {
    PlaneNode *plane = (*li);
    manager->write_pointer(dg, plane);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int ClipPlaneAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  Planes::iterator li;
  for (li = _planes.begin(); li != _planes.end(); ++li) {
    PlaneNode *node;
    DCAST_INTO_R(node, p_list[pi++], pi);
    (*li) = node;
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ClipPlaneAttrib is encountered
//               in the Bam file.  It should create the ClipPlaneAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ClipPlaneAttrib::
make_from_bam(const FactoryParams &params) {
  ClipPlaneAttrib *attrib = new ClipPlaneAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ClipPlaneAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ClipPlaneAttrib.
////////////////////////////////////////////////////////////////////
void ClipPlaneAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _operation = (Operation)scan.get_int8();
  int num_planes = scan.get_uint16();

  for (int i = 0; i < num_planes; i++) {
    manager->read_pointer(scan);
    _planes.push_back(NULL);
  }
}
