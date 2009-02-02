// Filename: tinyGeomMunger.cxx
// Created by:  drose (29Apr08)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "tinyGeomMunger.h"
#include "dcast.h"

TypeHandle TinyGeomMunger::_type_handle;

ALLOC_DELETED_CHAIN_DEF(TinyGeomMunger);

////////////////////////////////////////////////////////////////////
//     Function: TinyGeomMunger::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
TinyGeomMunger::
TinyGeomMunger(GraphicsStateGuardian *gsg, const RenderState *state) :
  StandardMunger(gsg, state, 4, NT_uint8, C_color)
{
  // The TinyGSG can apply the color and color scale at runtime.
  //  _munge_color = false;
  //  _munge_color_scale = false;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGeomMunger::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
TinyGeomMunger::
~TinyGeomMunger() {
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGeomMunger::munge_format_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexFormat, converts it if
//               necessary to the appropriate format for rendering.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexFormat) TinyGeomMunger::
munge_format_impl(const GeomVertexFormat *orig,
                  const GeomVertexAnimationSpec &animation) {
  PT(GeomVertexFormat) new_format = new GeomVertexFormat(*orig);
  new_format->set_animation(animation);

  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format(new_format);

  return format;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGeomMunger::premunge_format_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexFormat, converts it if
//               necessary to the appropriate format for rendering.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexFormat) TinyGeomMunger::
premunge_format_impl(const GeomVertexFormat *orig) {
  return orig;
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGeomMunger::compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int TinyGeomMunger::
compare_to_impl(const GeomMunger *other) const {
  return StandardMunger::compare_to_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: TinyGeomMunger::geom_compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int TinyGeomMunger::
geom_compare_to_impl(const GeomMunger *other) const {
  return StandardMunger::compare_to_impl(other);
}
