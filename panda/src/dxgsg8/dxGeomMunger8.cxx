// Filename: dxGeomMunger8.cxx
// Created by:  drose (11Mar05)
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

#include "dxGeomMunger8.h"

qpGeomMunger *DXGeomMunger8::_deleted_chain = NULL;
TypeHandle DXGeomMunger8::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DXGeomMunger8::munge_format_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexFormat, converts it if
//               necessary to the appropriate format for rendering.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexFormat) DXGeomMunger8::
munge_format_impl(const qpGeomVertexFormat *orig) {
  // We have to build a completely new format that includes only the
  // appropriate components, in the appropriate order, in just one
  // array.
  PT(qpGeomVertexFormat) new_format = new qpGeomVertexFormat(*orig);
  PT(qpGeomVertexArrayFormat) new_array_format = new qpGeomVertexArrayFormat;

  const qpGeomVertexDataType *vertex_type = 
    orig->get_data_type(InternalName::get_vertex());
  const qpGeomVertexDataType *normal_type = 
    orig->get_data_type(InternalName::get_normal());
  const qpGeomVertexDataType *color_type = 
    orig->get_data_type(InternalName::get_color());
  const qpGeomVertexDataType *texcoord_type = 
    orig->get_data_type(InternalName::get_texcoord());

  if (vertex_type != (const qpGeomVertexDataType *)NULL) {
    new_array_format->add_data_type
      (InternalName::get_vertex(), 3, qpGeomVertexDataType::NT_float32,
       qpGeomVertexDataType::C_point);
    new_format->remove_data_type(vertex_type->get_name());
  } else {
    // If we don't have a vertex type, not much we can do.
    return orig;
  }

  if (normal_type != (const qpGeomVertexDataType *)NULL) {
    new_array_format->add_data_type
      (InternalName::get_normal(), 3, qpGeomVertexDataType::NT_float32,
       qpGeomVertexDataType::C_vector);
    new_format->remove_data_type(normal_type->get_name());
  }

  if (color_type != (const qpGeomVertexDataType *)NULL) {
    new_array_format->add_data_type
      (InternalName::get_color(), 1, qpGeomVertexDataType::NT_packed_8888,
       qpGeomVertexDataType::C_argb);
    new_format->remove_data_type(color_type->get_name());
  }

  // To support multitexture, we will need to add all of the relevant
  // texcoord types, and in the correct order (or at least in a known
  // order).  For now, we just add the default texcoords only.
  if (texcoord_type != (const qpGeomVertexDataType *)NULL) {
    new_array_format->add_data_type
      (InternalName::get_texcoord(), texcoord_type->get_num_values(),
       qpGeomVertexDataType::NT_float32, qpGeomVertexDataType::C_texcoord);
    new_format->remove_data_type(texcoord_type->get_name());
  }

  if (new_array_format->is_data_subset_of(*orig->get_array(0))) {
    // If the new array format we have built is essentially the same
    // as the first data array anyway, go ahead and keep the original.
    return orig;
  }

  // Use the new format; make sure the DX8-friendly array is first in
  // the list.
  new_format->insert_array(0, new_array_format);
  return qpGeomVertexFormat::register_format(new_format);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGeomMunger8::munge_geom_impl
//       Access: Protected, Virtual
//  Description: Converts a Geom and/or its data as necessary.
////////////////////////////////////////////////////////////////////
void DXGeomMunger8::
munge_geom_impl(CPT(qpGeom) &geom, CPT(qpGeomVertexData) &data) {
}

////////////////////////////////////////////////////////////////////
//     Function: DXGeomMunger8::compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int DXGeomMunger8::
compare_to_impl(const qpGeomMunger *other) const {
  //  const DXGeomMunger8 *om = DCAST(DXGeomMunger8, other);

  return ColorMunger::compare_to_impl(other);
}
