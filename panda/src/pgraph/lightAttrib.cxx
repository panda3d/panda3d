// Filename: lightAttrib.cxx
// Created by:  drose (26Mar02)
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

#include "lightAttrib.h"
#include "pandaNode.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle LightAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make_all_off
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that turns off
//               all lights (and hence disables lighting).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make_all_off() {
  LightAttrib *attrib = new LightAttrib;
  attrib->_operation = O_set;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that turns on (or
//               off, according to op) the indicate light(s).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make(LightAttrib::Operation op, Light *light) {
  LightAttrib *attrib = new LightAttrib;
  attrib->_operation = op;
  attrib->_lights.push_back(light);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that turns on (or
//               off, according to op) the indicate light(s).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make(LightAttrib::Operation op, Light *light1, Light *light2) {
  LightAttrib *attrib = new LightAttrib;
  attrib->_operation = op;
  attrib->_lights.push_back(light1);
  attrib->_lights.push_back(light2);

  attrib->_lights.sort();
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that turns on (or
//               off, according to op) the indicate light(s).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make(LightAttrib::Operation op, Light *light1, Light *light2,
     Light *light3) {
  LightAttrib *attrib = new LightAttrib;
  attrib->_operation = op;
  attrib->_lights.push_back(light1);
  attrib->_lights.push_back(light2);
  attrib->_lights.push_back(light3);

  attrib->_lights.sort();
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make
//       Access: Published, Static
//  Description: Constructs a new LightAttrib object that turns on (or
//               off, according to op) the indicate light(s).
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
make(LightAttrib::Operation op, Light *light1, Light *light2,
     Light *light3, Light *light4) {
  LightAttrib *attrib = new LightAttrib;
  attrib->_operation = op;
  attrib->_lights.push_back(light1);
  attrib->_lights.push_back(light2);
  attrib->_lights.push_back(light3);
  attrib->_lights.push_back(light4);

  attrib->_lights.sort();
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::has_light
//       Access: Published
//  Description: Returns true if the indicated light is listed in the
//               attrib, false otherwise.
////////////////////////////////////////////////////////////////////
bool LightAttrib::
has_light(Light *light) const {
  return _lights.find(light) != _lights.end();
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void LightAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_light(this);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void LightAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (_operation == O_set && _lights.empty()) {
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

    Lights::const_iterator li;
    for (li = _lights.begin(); li != _lights.end(); ++li) {
      Light *light = (*li);
      out << " " << light->get_type();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived LightAttrib
//               types to return a unique number indicating whether
//               this LightAttrib is equivalent to the other one.
//
//               This should return 0 if the two LightAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two LightAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int LightAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const LightAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  if (_operation != ta->_operation) {
    return (int)_operation - (int)ta->_operation;
  }

  Lights::const_iterator li = _lights.begin();
  Lights::const_iterator oli = ta->_lights.begin();

  while (li != _lights.end() && oli != ta->_lights.end()) {
    Light *light = (*li);
    Light *other_light = (*oli);

    if (light != other_light) {
      return light < other_light ? -1 : 1;
    }

    ++li;
    ++oli;
  }

  if (li != _lights.end()) {
    return 1;
  }
  if (oli != ta->_lights.end()) {
    return -1;
  }
  
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::compose_impl
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
CPT(RenderAttrib) LightAttrib::
compose_impl(const RenderAttrib *other) const {
  const LightAttrib *ta;
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
//     Function: LightAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  // I think in this case the other attrib always wins.  Maybe this
  // needs a bit more thought.  It's hard to imagine that it's even
  // important to compute this properly.
  return other;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived LightAttrib
//               types to specify what the default property for a
//               LightAttrib of this type should be.
//
//               This should return a newly-allocated LightAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of LightAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *LightAttrib::
make_default_impl() const {
  return new LightAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::do_add
//       Access: Private
//  Description: Returns a new LightAttrib that represents all the
//               lights of this attrib, with those of the other one
//               added in.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
do_add(const LightAttrib *other, LightAttrib::Operation op) const {
  Lights::const_iterator ai = _lights.begin();
  Lights::const_iterator bi = other->_lights.begin();

  // Create a new LightAttrib that will hold the result.
  LightAttrib *new_attrib = new LightAttrib;
  new_attrib->_operation = op;
  back_insert_iterator<Lights> result = 
    back_inserter(new_attrib->_lights);

  while (ai != _lights.end() && bi != other->_lights.end()) {
    if ((*ai) < (*bi)) {
      // Here is a light that we have in the original, which is not
      // present in the secondary.
      *result = *ai;
      ++ai;
      ++result;
    } else if ((*bi) < (*ai)) {
      // Here is a new light we have in the secondary, that was not
      // present in the original.
      *result = *bi;
      ++bi;
      ++result;
    } else {
      // Here is a light we have in both.
      *result = *ai;
      ++ai;
      ++bi;
      ++result;
    }
  }

  while (ai != _lights.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  while (bi != other->_lights.end()) {
    *result = *bi;
    ++bi;
    ++result;
  }

  return return_new(new_attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::do_remove
//       Access: Private
//  Description: Returns a new LightAttrib that represents all the
//               lights of this attrib, with those of the other one
//               removed.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) LightAttrib::
do_remove(const LightAttrib *other, LightAttrib::Operation op) const {
  Lights::const_iterator ai = _lights.begin();
  Lights::const_iterator bi = other->_lights.begin();

  // Create a new LightAttrib that will hold the result.
  LightAttrib *new_attrib = new LightAttrib;
  new_attrib->_operation = op;
  back_insert_iterator<Lights> result = 
    back_inserter(new_attrib->_lights);

  while (ai != _lights.end() && bi != other->_lights.end()) {
    if ((*ai) < (*bi)) {
      // Here is a light that we have in the original, which is
      // not present in the secondary.  Keep it.
      *result = *ai;
      ++ai;
      ++result;
    } else if ((*bi) < (*ai)) {
      // Here is a new light we have in the secondary, that was
      // not present in the original.  Ignore it.
      ++bi;
    } else {
      // Here is a light we have in both.  Drop it.
      ++ai;
      ++bi;
    }
  }

  while (ai != _lights.end()) {
    *result = *ai;
    ++ai;
    ++result;
  }

  return return_new(new_attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               LightAttrib.
////////////////////////////////////////////////////////////////////
void LightAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void LightAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8((int)_operation);
  PN_uint16 num_lights = _lights.size();
  nassertv(num_lights == _lights.size());
  dg.add_uint16(num_lights);

  Lights::const_iterator li;
  for (li = _lights.begin(); li != _lights.end(); ++li) {
    Light *light = (*li);
    manager->write_pointer(dg, light->as_node());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int LightAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  Lights::iterator li;
  for (li = _lights.begin(); li != _lights.end(); ++li) {
    PandaNode *node;
    DCAST_INTO_R(node, p_list[pi++], pi);
    (*li) = node->as_light();
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type LightAttrib is encountered
//               in the Bam file.  It should create the LightAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *LightAttrib::
make_from_bam(const FactoryParams &params) {
  LightAttrib *attrib = new LightAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: LightAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new LightAttrib.
////////////////////////////////////////////////////////////////////
void LightAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _operation = (Operation)scan.get_int8();
  int num_lights = scan.get_uint16();

  for (int i = 0; i < num_lights; i++) {
    manager->read_pointer(scan);
    _lights.push_back(NULL);
  }
}
