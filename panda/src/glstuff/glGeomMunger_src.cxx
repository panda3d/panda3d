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
munge_format_impl(const qpGeomVertexFormat *orig) {
  CPT(qpGeomVertexFormat) format = orig;

  const qpGeomVertexDataType *color_type = 
    format->get_data_type(InternalName::get_color());
  if (color_type != (qpGeomVertexDataType *)NULL &&
      color_type->get_numeric_type() == qpGeomVertexDataType::NT_packed_argb) {
    // We need to convert the color format; OpenGL doesn't support the
    // byte order of DirectX's packed_argb format.
    int color_array = format->get_array_with(InternalName::get_color());

    PT(qpGeomVertexFormat) new_format = new qpGeomVertexFormat(*format);
    PT(qpGeomVertexArrayFormat) new_array_format = new_format->modify_array(color_array);

    // Replace the existing color format with the new format.
    new_array_format->add_data_type
      (InternalName::get_color(), 4, qpGeomVertexDataType::NT_uint8,
       color_type->get_start());

    format = qpGeomVertexFormat::register_format(new_format);
  }

  /*
  if (true) {
    // Split out the interleaved array into n parallel arrays.
    PT(qpGeomVertexFormat) new_format = new qpGeomVertexFormat;
    for (int i = 0; i < format->get_num_data_types(); i++) {
      const qpGeomVertexDataType *data_type = format->get_data_type(i);
      PT(qpGeomVertexArrayFormat) new_array_format = new qpGeomVertexArrayFormat;
      new_array_format->add_data_type(data_type->get_name(), data_type->get_num_components(),
                                      data_type->get_numeric_type());
      new_format->add_array(new_array_format);
    }
    format = qpGeomVertexFormat::register_format(new_format);
  }
  */

  return format;
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

  return ColorMunger::compare_to_impl(other);
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
  return ColorMunger::geom_compare_to_impl(other);
}
