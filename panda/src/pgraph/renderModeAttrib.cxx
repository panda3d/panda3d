// Filename: renderModeAttrib.cxx
// Created by:  drose (14Mar02)
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

#include "renderModeAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle RenderModeAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::make
//       Access: Published, Static
//  Description: Constructs a new RenderModeAttrib object that specifies
//               whether to draw polygons in the normal, filled mode,
//               or wireframe mode, or in some other yet-to-be-defined
//               mode.
//
//               The line_width is relevant only if mode is
//               M_wireframe, and specifies the thickness of the
//               lines, in pixels, to use for wireframe.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RenderModeAttrib::
make(RenderModeAttrib::Mode mode, float line_width) {
  if (mode != M_wireframe) {
    line_width = 0.0f;
  }
  RenderModeAttrib *attrib = new RenderModeAttrib(mode, line_width);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void RenderModeAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_render_mode(this);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void RenderModeAttrib::
output(ostream &out) const {
  out << get_type() << ":";
  switch (get_mode()) {
  case M_filled:
    out << "filled";
    break;

  case M_wireframe:
    out << "wireframe(" << get_line_width() << ")";
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderModeAttrib
//               types to return a unique number indicating whether
//               this RenderModeAttrib is equivalent to the other one.
//
//               This should return 0 if the two RenderModeAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two RenderModeAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int RenderModeAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const RenderModeAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  if (_mode != ta->_mode) {
    return (int)_mode - (int)ta->_mode;
  }
  if (_line_width != ta->_line_width) {
    return _line_width < ta->_line_width ? -1 : 1;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderModeAttrib
//               types to specify what the default property for a
//               RenderModeAttrib of this type should be.
//
//               This should return a newly-allocated RenderModeAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of RenderModeAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *RenderModeAttrib::
make_default_impl() const {
  return new RenderModeAttrib(M_filled, 0.0f);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               RenderModeAttrib.
////////////////////////////////////////////////////////////////////
void RenderModeAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void RenderModeAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  dg.add_int8(_mode);
  dg.add_float32(_line_width);
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type RenderModeAttrib is encountered
//               in the Bam file.  It should create the RenderModeAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *RenderModeAttrib::
make_from_bam(const FactoryParams &params) {
  RenderModeAttrib *attrib = new RenderModeAttrib(M_filled, 0.0f);
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new RenderModeAttrib.
////////////////////////////////////////////////////////////////////
void RenderModeAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mode = (Mode)scan.get_int8();
  _line_width = scan.get_float32();
}
