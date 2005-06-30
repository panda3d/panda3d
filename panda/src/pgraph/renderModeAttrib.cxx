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
//               The thickness parameter specifies the thickness to be
//               used for wireframe lines, as well as for ordinary
//               linestrip lines; it also specifies the diameter of
//               points.  (Thick lines are presently only supported in
//               OpenGL; but thick points are supported on either
//               platform.)
//
//               If perspective is true, the point thickness
//               represented is actually a width in 3-d units, and the
//               points should scale according to perspective.  When
//               it is false, the point thickness is actually a width
//               in pixels, and points are a uniform screen size
//               regardless of distance from the camera.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) RenderModeAttrib::
make(RenderModeAttrib::Mode mode, float thickness, bool perspective) {
  RenderModeAttrib *attrib = new RenderModeAttrib(mode, thickness, perspective);
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
  case M_unchanged:
    out << "unchanged(" << get_thickness() << ")";
    break;

  case M_filled:
    out << "filled(" << get_thickness() << ")";
    break;

  case M_wireframe:
    out << "wireframe(" << get_thickness() << ")";
    break;

  case M_point:
    out << "point(" << get_thickness() << ")";
    break;
  }

  if (get_perspective()) {
    out << ", perspective";
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
  if (_thickness != ta->_thickness) {
    return _thickness < ta->_thickness ? -1 : 1;
  }
  if (_perspective != ta->_perspective) {
    return (int)_perspective - (int)ta->_perspective;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: RenderModeAttrib::compose_impl
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
CPT(RenderAttrib) RenderModeAttrib::
compose_impl(const RenderAttrib *other) const {
  const RenderModeAttrib *ta;
  DCAST_INTO_R(ta, other, 0);

  // The special mode M_unchanged means to keep the current mode.
  Mode mode = ta->get_mode();
  if (mode == M_unchanged) {
    mode = get_mode();
  }

  return make(mode, ta->get_thickness(), ta->get_perspective());
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
  return new RenderModeAttrib(M_filled, 1.0f, false);
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
  dg.add_float32(_thickness);
  dg.add_bool(_perspective);
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
  RenderModeAttrib *attrib = new RenderModeAttrib(M_filled, 1.0f, false);
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
  _thickness = scan.get_float32();
  _perspective = scan.get_bool();
}
