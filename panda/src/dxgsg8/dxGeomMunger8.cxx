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
#include "qpgeomVertexReader.h"
#include "qpgeomVertexWriter.h"
#include "config_dxgsg8.h"

qpGeomMunger *DXGeomMunger8::_deleted_chain = NULL;
TypeHandle DXGeomMunger8::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DXGeomMunger8::munge_format_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexFormat, converts it if
//               necessary to the appropriate format for rendering.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexFormat) DXGeomMunger8::
munge_format_impl(const qpGeomVertexFormat *orig,
                  const qpGeomVertexAnimationSpec &animation) {
  if (dxgsg8_cat.is_debug()) {
    if (animation.get_animation_type() != qpGeomVertexAnimationSpec::AT_none) {
      dxgsg8_cat.debug()
        << "preparing animation type " << animation << "\n";
    }
  }
  // We have to build a completely new format that includes only the
  // appropriate components, in the appropriate order, in just one
  // array.
  PT(qpGeomVertexFormat) new_format = new qpGeomVertexFormat(*orig);
  new_format->set_animation(animation);
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

  if (animation.get_animation_type() == qpGeomVertexAnimationSpec::AT_hardware &&
      animation.get_num_transforms() > 0) {
    // If we want hardware animation, we need to reserve space for the
    // blend weights.
    new_array_format->add_data_type
      (InternalName::get_transform_weight(), animation.get_num_transforms() - 1,
       qpGeomVertexDataType::NT_float32, qpGeomVertexDataType::C_other);

    if (animation.get_indexed_transforms()) {
      // Also, if we'll be indexing into the transfom palette, reserve
      // space for the index.
      new_array_format->add_data_type
        (InternalName::get_transform_index(), 1,
         qpGeomVertexDataType::NT_packed_dcba, qpGeomVertexDataType::C_index);
    }                                    
  }

  if (normal_type != (const qpGeomVertexDataType *)NULL) {
    new_array_format->add_data_type
      (InternalName::get_normal(), 3, qpGeomVertexDataType::NT_float32,
       qpGeomVertexDataType::C_vector);
    new_format->remove_data_type(normal_type->get_name());
  }

  if (color_type != (const qpGeomVertexDataType *)NULL) {
    new_array_format->add_data_type
      (InternalName::get_color(), 1, qpGeomVertexDataType::NT_packed_dabc,
       qpGeomVertexDataType::C_color);
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
