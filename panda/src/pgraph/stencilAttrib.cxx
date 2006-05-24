// Filename: stencilAttrib.cxx
// Created by:  aignacio (18May06)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2006, Disney Enterprises, Inc.  All rights reserved
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

#include "stencilAttrib.h"
#include "attribSlots.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle StencilAttrib::_type_handle;

char *StencilAttrib::
stencil_render_state_name_array [StencilAttrib::SRS_total] =
{
  "SRS_front_enable",
  "SRS_back_enable",

  "SRS_front_comparison_function",
  "SRS_front_stencil_fail_operation",
  "SRS_front_stencil_pass_z_fail_operation",
  "SRS_front_stencil_pass_z_pass_operation",

  "SRS_reference",
  "SRS_read_mask",
  "SRS_write_mask",

  "SRS_back_comparison_function",
  "SRS_back_stencil_fail_operation",
  "SRS_back_stencil_pass_z_fail_operation",
  "SRS_back_stencil_pass_z_pass_operation",
};

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::make_begin
//       Access: Published, Static
//  Description: Constructs a new default and writable StencilAttrib.
//               set_render_state can be called.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) StencilAttrib::
make_begin() {
  StencilAttrib *attrib = new StencilAttrib;
  attrib -> _pre = true;

  CPT(RenderAttrib) pt_attrib = attrib;

  return pt_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::make
//       Access: Published, Static
//  Description: Constructs a new default read-only StencilAttrib.
//               set_render_state can not be called on the created
//               object.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) StencilAttrib::
make() {
  StencilAttrib *attrib = new StencilAttrib;
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::make_end
//       Access: Published
//  Description: Constructs a final read-only StencilAttrib object
//               from an existing StencilAttrib.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) StencilAttrib::
make_end() {

  StencilAttrib *attrib = new StencilAttrib;
  StencilAttrib *original_attrib;

  original_attrib = this;

  int index;
  for (index = 0; index < SRS_total; index++) {
    attrib -> _stencil_render_states [index] =
      original_attrib -> _stencil_render_states [index];
  }

  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::output
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
void StencilAttrib::
output(ostream &out) const {

  int index;
  for (index = 0; index < SRS_total; index++) {
    out
      << "(" << stencil_render_state_name_array [index]
      << ", " << _stencil_render_states [index] << ")";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived StencilAttrib
//               types to return a unique number indicating whether
//               this StencilAttrib is equivalent to the other one.
//
//               This should return 0 if the two StencilAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two StencilAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int StencilAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const StencilAttrib *sa;
  DCAST_INTO_R(sa, other, 0);

  int a;
  int b;
  int index;
  int compare_result = 0;

  for (index = 0; index < SRS_total; index++) {
    a = (int) sa -> _stencil_render_states [index];
    b = (int) _stencil_render_states [index];
    if (compare_result = (a - b)) {
      break;
    }
  }

  return compare_result;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived StencilAttrib
//               types to specify what the default property for a
//               StencilAttrib of this type should be.
//
//               This should return a newly-allocated StencilAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of StencilAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *StencilAttrib::
make_default_impl() const {
  return new StencilAttrib;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::store_into_slot
//       Access: Public, Virtual
//  Description: Stores this attrib into the appropriate slot of
//               an object of class AttribSlots.
////////////////////////////////////////////////////////////////////
void StencilAttrib::
store_into_slot(AttribSlots *slots) const {
  slots->_stencil = this;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               StencilAttrib.
////////////////////////////////////////////////////////////////////
void StencilAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void StencilAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  int index;
  for (index = 0; index < SRS_total; index++) {
    dg.add_int32(_stencil_render_states [index]);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type StencilAttrib is encountered
//               in the Bam file.  It should create the StencilAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *StencilAttrib::
make_from_bam(const FactoryParams &params) {
  StencilAttrib *attrib = new StencilAttrib;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: StencilAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new StencilAttrib.
////////////////////////////////////////////////////////////////////
void StencilAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  int index;
  for (index = 0; index < SRS_total; index++) {
    _stencil_render_states [index] = scan.get_int32();
  }
}
