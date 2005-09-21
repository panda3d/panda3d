// Filename: materialAttrib.cxx
// Created by:  drose (04Mar02)
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

#include "materialAttrib.h"
#include "attribSlots.h"
#include "graphicsStateGuardianBase.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle MaterialAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::make
//       Access: Published, Static
//  Description: Constructs a new MaterialAttrib object suitable for
//               rendering the indicated material onto geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) MaterialAttrib::
make(const Material *material) {
  MaterialAttrib *attrib = new MaterialAttrib;
  attrib->_material = material;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::make_off
//       Access: Published, Static
//  Description: Constructs a new MaterialAttrib object suitable for
//               rendering unmateriald geometry.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) MaterialAttrib::
make_off() {
  MaterialAttrib *attrib = new MaterialAttrib;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void MaterialAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  if (is_off()) {
    out << "(off)";
  } else {
    out << *_material;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived MaterialAttrib
//               types to return a unique number indicating whether
//               this MaterialAttrib is equivalent to the other one.
//
//               This should return 0 if the two MaterialAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two MaterialAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int MaterialAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const MaterialAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  // Comparing pointers by subtraction is problematic.  Instead of
  // doing this, we'll just depend on the built-in != and < operators
  // for comparing pointers.
  if (_material != ta->_material) {
    return _material < ta->_material ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived MaterialAttrib
//               types to specify what the default property for a
//               MaterialAttrib of this type should be.
//
//               This should return a newly-allocated MaterialAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of MaterialAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *MaterialAttrib::
make_default_impl() const {
  return new MaterialAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: When attribs are stored in a slot-based attrib array,
//               this returns the index of the appropriate slot
//               for this attrib type.
////////////////////////////////////////////////////////////////////
void MaterialAttrib::
store_into_slot(AttribSlots *slots) const {
  slots->_material = this;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               MaterialAttrib.
////////////////////////////////////////////////////////////////////
void MaterialAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void MaterialAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  manager->write_pointer(dg, _material);
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::complete_pointers
//       Access: Public, Virtual
//  Description: Receives an array of pointers, one for each time
//               manager->read_pointer() was called in fillin().
//               Returns the number of pointers processed.
////////////////////////////////////////////////////////////////////
int MaterialAttrib::
complete_pointers(TypedWritable **p_list, BamReader *manager) {
  int pi = RenderAttrib::complete_pointers(p_list, manager);

  TypedWritable *material = p_list[pi++];
  if (material != (TypedWritable *)NULL) {
    _material = DCAST(Material, material);
  }

  return pi;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type MaterialAttrib is encountered
//               in the Bam file.  It should create the MaterialAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *MaterialAttrib::
make_from_bam(const FactoryParams &params) {
  MaterialAttrib *attrib = new MaterialAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: MaterialAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new MaterialAttrib.
////////////////////////////////////////////////////////////////////
void MaterialAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  // Read the _material pointer.
  manager->read_pointer(scan);
}
