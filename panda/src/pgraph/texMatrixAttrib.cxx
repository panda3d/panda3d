// Filename: texMatrixAttrib.cxx
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

#include "texMatrixAttrib.h"
#include "graphicsStateGuardianBase.h"
#include "dcast.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "datagram.h"
#include "datagramIterator.h"

TypeHandle TexMatrixAttrib::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::make
//       Access: Published, Static
//  Description: Constructs a new TexMatrixAttrib object that indicates
//               geometry should be scaled by the indicated factor.
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexMatrixAttrib::
make(const LMatrix4f &mat) {
  TexMatrixAttrib *attrib = new TexMatrixAttrib(mat);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::issue
//       Access: Public, Virtual
//  Description: Calls the appropriate method on the indicated GSG
//               to issue the graphics commands appropriate to the
//               given attribute.  This is normally called
//               (indirectly) only from
//               GraphicsStateGuardian::set_state() or modify_state().
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
issue(GraphicsStateGuardianBase *gsg) const {
  gsg->issue_tex_matrix(this);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
output(ostream &out) const {
  out << get_type() << ":(" << get_mat() << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::compare_to_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TexMatrixAttrib
//               types to return a unique number indicating whether
//               this TexMatrixAttrib is equivalent to the other one.
//
//               This should return 0 if the two TexMatrixAttrib objects
//               are equivalent, a number less than zero if this one
//               should be sorted before the other one, and a number
//               greater than zero otherwise.
//
//               This will only be called with two TexMatrixAttrib
//               objects whose get_type() functions return the same.
////////////////////////////////////////////////////////////////////
int TexMatrixAttrib::
compare_to_impl(const RenderAttrib *other) const {
  const TexMatrixAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  return _mat.compare_to(ta->_mat);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::compose_impl
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
CPT(RenderAttrib) TexMatrixAttrib::
compose_impl(const RenderAttrib *other) const {
  const TexMatrixAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  LMatrix4f new_mat = ta->_mat * _mat;

  TexMatrixAttrib *attrib = new TexMatrixAttrib(new_mat);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::invert_compose_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived RenderAttrib
//               types to specify how two consecutive RenderAttrib
//               objects of the same type interact.
//
//               See invert_compose() and compose_impl().
////////////////////////////////////////////////////////////////////
CPT(RenderAttrib) TexMatrixAttrib::
invert_compose_impl(const RenderAttrib *other) const {
  const TexMatrixAttrib *ta;
  DCAST_INTO_R(ta, other, 0);
  LMatrix4f new_mat;
  new_mat.invert_from(_mat);
  new_mat = ta->_mat * new_mat;

  TexMatrixAttrib *attrib = new TexMatrixAttrib(new_mat);
  return return_new(attrib);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::make_default_impl
//       Access: Protected, Virtual
//  Description: Intended to be overridden by derived TexMatrixAttrib
//               types to specify what the default property for a
//               TexMatrixAttrib of this type should be.
//
//               This should return a newly-allocated TexMatrixAttrib of
//               the same type that corresponds to whatever the
//               standard default for this kind of TexMatrixAttrib is.
////////////////////////////////////////////////////////////////////
RenderAttrib *TexMatrixAttrib::
make_default_impl() const {
  return new TexMatrixAttrib(LMatrix4f::ident_mat());
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::register_with_read_factory
//       Access: Public, Static
//  Description: Tells the BamReader how to create objects of type
//               TexMatrixAttrib.
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_from_bam);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::write_datagram
//       Access: Public, Virtual
//  Description: Writes the contents of this object to the datagram
//               for shipping out to a Bam file.
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
write_datagram(BamWriter *manager, Datagram &dg) {
  RenderAttrib::write_datagram(manager, dg);

  _mat.write_datagram(dg);
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::make_from_bam
//       Access: Protected, Static
//  Description: This function is called by the BamReader's factory
//               when a new object of type TexMatrixAttrib is encountered
//               in the Bam file.  It should create the TexMatrixAttrib
//               and extract its information from the file.
////////////////////////////////////////////////////////////////////
TypedWritable *TexMatrixAttrib::
make_from_bam(const FactoryParams &params) {
  TexMatrixAttrib *attrib = new TexMatrixAttrib(LMatrix4f::ident_mat());
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  attrib->fillin(scan, manager);

  return attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TexMatrixAttrib::fillin
//       Access: Protected
//  Description: This internal function is called by make_from_bam to
//               read in all of the relevant data from the BamFile for
//               the new TexMatrixAttrib.
////////////////////////////////////////////////////////////////////
void TexMatrixAttrib::
fillin(DatagramIterator &scan, BamReader *manager) {
  RenderAttrib::fillin(scan, manager);

  _mat.read_datagram(scan);
}
