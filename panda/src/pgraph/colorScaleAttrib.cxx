// Filename: colorScaleAttrib.cxx
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

#include "colorScaleAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle ColorScaleAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::make
//       Access: Published, Static
//  Description: Constructs a new ColorScaleAttrib object that indicates
//               geometry should be scaled by the indicated factor.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorScaleAttrib::
make(const LVecBase4f &scale) {
  ColorScaleAttrib *attrib = new ColorScaleAttrib(scale);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void ColorScaleAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_color_scale(this);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void ColorScaleAttrib::
output(ostream &out) const {
  out << get_type() << ":(" << get_scale() << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ColorScaleAttrib
//               types to return a unique number indicating whether
//               this ColorScaleAttrib is equivalent to the other one.
//
//               This should return 0 if the two ColorScaleAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two ColorScaleAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int ColorScaleAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const ColorScaleAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  return _scale.compare_to(ta->_scale);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::compose_impl
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
CPT(RenderAttrib) ColorScaleAttrib::
compose_impl(const RenderAttrib *other) const {
  const ColorScaleAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  LVecBase4f new_scale(ta->_scale[0] * _scale[0],
                       ta->_scale[1] * _scale[1],
                       ta->_scale[2] * _scale[2],
                       ta->_scale[3] * _scale[3]);

  ColorScaleAttrib *attrib = new ColorScaleAttrib(new_scale);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) ColorScaleAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  const ColorScaleAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  LVecBase4f new_scale(ta->_scale[0] / _scale[0],
                       ta->_scale[1] / _scale[1],
                       ta->_scale[2] / _scale[2],
                       ta->_scale[3] / _scale[3]);

  ColorScaleAttrib *attrib = new ColorScaleAttrib(new_scale);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived ColorScaleAttrib
//               types to specify what the default property for a
//               ColorScaleAttrib of this type should be.
//
//               This should return a newly-allocated ColorScaleAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of ColorScaleAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *ColorScaleAttrib::
make_default_impl() const {
  return new ColorScaleAttrib(LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f));
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               ColorScaleAttrib.
////////////////////////////////////////////////////////////////////
void ColorScaleAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void ColorScaleAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  _scale.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type ColorScaleAttrib is encountered
//               in the Bam file.  It should create the ColorScaleAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *ColorScaleAttrib::
make_from_bam(const FactoryParams &params) {
  ColorScaleAttrib *attrib = new ColorScaleAttrib(LVecBase4f(1.0f, 1.0f, 1.0f, 1.0f));
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: ColorScaleAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new ColorScaleAttrib.
////////////////////////////////////////////////////////////////////
void ColorScaleAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _scale.read_datagram(scan);
}
