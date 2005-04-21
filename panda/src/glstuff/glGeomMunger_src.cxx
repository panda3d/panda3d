// Filename: glGeomMunger_src.cxx
// Created by:  drose (10Mar05)
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

#include "dcast.h"

qpGeomMunger *CLP(GeomMunger)::_deleted_chain = NULL;
TypeHandle CLP(GeomMunger)::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: CLP(GeomMunger)::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
CLP(GeomMunger)::
~CLP(GeomMunger)() {
  // We need to remove this pointer from all of the geom contexts that
  // reference this object.
  GeomContexts::iterator gci;
  for (gci = _geom_contexts.begin(); gci != _geom_contexts.end(); ++gci) {
    (*gci)->remove_munger(this);
  }
  _geom_contexts.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GeomMunger)::munge_format_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexFormat, converts it if
//               necessary to the appropriate format for rendering.
////////////////////////////////////////////////////////////////////
CPT(qpGeomVertexFormat) CLP(GeomMunger)::
munge_format_impl(const qpGeomVertexFormat *orig,
                  const qpGeomVertexAnimationSpec &animation) {
  PT(qpGeomVertexFormat) new_format = new qpGeomVertexFormat(*orig);
  new_format->set_animation(animation);

  const qpGeomVertexColumn *color_type = orig->get_color_column();
  if (color_type != (qpGeomVertexColumn *)NULL &&
      color_type->get_numeric_type() == NT_packed_dabc) {
    // We need to convert the color format; OpenGL doesn't support the
    // byte order of DirectX's packed ARGB format.
    int color_array = orig->get_array_with(InternalName::get_color());

    PT(qpGeomVertexArrayFormat) new_array_format = new_format->modify_array(color_array);

    // Replace the existing color format with the new format.
    new_array_format->add_column
      (InternalName::get_color(), 4, NT_uint8,
       C_color, color_type->get_start());
  }

  if (animation.get_animation_type() == AT_hardware &&
      animation.get_num_transforms() > 0) {
    // If we want hardware animation, we need to reserve space for the
    // blend weights.

    PT(qpGeomVertexArrayFormat) new_array_format = new qpGeomVertexArrayFormat;
    new_array_format->add_column
      (InternalName::get_transform_weight(), animation.get_num_transforms() - 1,
       NT_float32, C_other);

    if (animation.get_indexed_transforms()) {
      // Also, if we'll be indexing into the transform table, reserve
      // space for the index.

      // TODO: We should examine the maximum palette index so we can
      // decide whether we need 16-bit indices.  That implies saving
      // the maximum palette index, presumably in the AnimationSpec.
      // At the moment, I don't think any existing hardware supports
      // more than 255 indices anyway.
      new_array_format->add_column
        (InternalName::get_transform_index(), animation.get_num_transforms(),
         NT_uint8, C_index);
    }                                    

    // Make sure the old weights and indices are removed, just in
    // case.
    new_format->remove_column(InternalName::get_transform_weight());
    new_format->remove_column(InternalName::get_transform_index());

    // And we don't need the transform_blend table any more.
    new_format->remove_column(InternalName::get_transform_blend());

    new_format->add_array(new_array_format);
  }

  /*
  if (true) {
    // Split out the interleaved array into n parallel arrays.
    CPT(qpGeomVertexFormat) format = new_format;
    new_format = new qpGeomVertexFormat;
    for (int i = 0; i < format->get_num_columns(); i++) {
      const qpGeomVertexColumn *column = format->get_column(i);
      PT(qpGeomVertexArrayFormat) new_array_format = new qpGeomVertexArrayFormat;
      new_array_format->add_column(column->get_name(), column->get_num_components(),
                                      column->get_numeric_type());
      new_format->add_array(new_array_format);
    }
  }
  */

  return qpGeomVertexFormat::register_format(new_format);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GeomMunger)::compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int CLP(GeomMunger)::
compare_to_impl(const qpGeomMunger *other) const {
  const CLP(GeomMunger) *om = DCAST(CLP(GeomMunger), other);
  if (_texture != om->_texture) {
    return _texture < om->_texture ? -1 : 1;
  }
  if (_tex_gen != om->_tex_gen) {
    return _tex_gen < om->_tex_gen ? -1 : 1;
  }

  return StandardMunger::compare_to_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: CLP(GeomMunger)::geom_compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int CLP(GeomMunger)::
geom_compare_to_impl(const qpGeomMunger *other) const {
  // We don't consider _texture and _tex_gen for this purpose; they
  // affect only whether the GL display list should be regenerated or
  // not, and don't require reconverting the vertices.
  return StandardMunger::geom_compare_to_impl(other);
}
