// Filename: dxGeomMunger7.cxx
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

#include "dxGeomMunger7.h"
#include "geomVertexReader.h"
#include "geomVertexWriter.h"
#include "config_dxgsg7.h"

GeomMunger *DXGeomMunger7::_deleted_chain = NULL;
TypeHandle DXGeomMunger7::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: DXGeomMunger7::munge_format_impl
//       Access: Protected, Virtual
//  Description: Given a source GeomVertexFormat, converts it if
//               necessary to the appropriate format for rendering.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexFormat) DXGeomMunger7::
munge_format_impl(const GeomVertexFormat *orig,
                  const GeomVertexAnimationSpec &animation) {
  nassertr(animation.get_animation_type() != AT_hardware, NULL);

  // We have to build a completely new format that includes only the
  // appropriate components, in the appropriate order, in just one
  // array.
  PT(GeomVertexFormat) new_format = new GeomVertexFormat(*orig);
  PT(GeomVertexArrayFormat) new_array_format = new GeomVertexArrayFormat;

  const GeomVertexColumn *vertex_type = orig->get_vertex_column();
  const GeomVertexColumn *normal_type = orig->get_normal_column(); 
  const GeomVertexColumn *color_type = orig->get_color_column();

  if (vertex_type != (const GeomVertexColumn *)NULL) {
    new_array_format->add_column
      (InternalName::get_vertex(), 3, NT_float32,
       vertex_type->get_contents());
    new_format->remove_column(vertex_type->get_name());

  } else {
    // If we don't have a vertex type, not much we can do.
    return orig;
  }

  if (normal_type != (const GeomVertexColumn *)NULL) {
    new_array_format->add_column
      (InternalName::get_normal(), 3, NT_float32, C_vector);
    new_format->remove_column(normal_type->get_name());
  }

  if (color_type != (const GeomVertexColumn *)NULL) {
    new_array_format->add_column
      (InternalName::get_color(), 1, NT_packed_dabc, C_color);
    new_format->remove_column(color_type->get_name());
  }

  // To support multitexture, we will need to add all of the relevant
  // texcoord types, and in the correct order.

  // Now set up each of the active texture coordinate stages--or at
  // least those for which we're not generating texture coordinates
  // automatically.

  // Now copy all of the texture coordinates in, in order by stage
  // index.  But we have to reuse previous columns.
  if (_texture != (TextureAttrib *)NULL) {
    typedef pset<const InternalName *> UsedStages;
    UsedStages used_stages;

    int num_stages = _texture->get_num_on_stages();
    for (int i = 0; i < num_stages; ++i) {
      TextureStage *stage = _texture->get_on_stage(i);

      const InternalName *name = stage->get_texcoord_name();
      if (used_stages.insert(name).second) {
        // This is the first time we've encountered this texcoord name.
        const GeomVertexColumn *texcoord_type = orig->get_column(name);
        
        if (texcoord_type != (const GeomVertexColumn *)NULL) {
          new_array_format->add_column
            (name, texcoord_type->get_num_values(), NT_float32, C_texcoord);
        } else {
          // We have to add something as a placeholder, even if the
          // texture coordinates aren't defined.
          new_array_format->add_column(name, 2, NT_float32, C_texcoord);
        }
        new_format->remove_column(name);
      }
    }
  }

  // Make sure the FVF-style array we just built up is first in the
  // list.
  new_format->insert_array(0, new_array_format);

  return GeomVertexFormat::register_format(new_format);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGeomMunger7::compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int DXGeomMunger7::
compare_to_impl(const GeomMunger *other) const {
  const DXGeomMunger7 *om = DCAST(DXGeomMunger7, other);
  if (_texture != om->_texture) {
    return _texture < om->_texture ? -1 : 1;
  }
  if (_tex_gen != om->_tex_gen) {
    return _tex_gen < om->_tex_gen ? -1 : 1;
  }

  return StandardMunger::compare_to_impl(other);
}

////////////////////////////////////////////////////////////////////
//     Function: DXGeomMunger7::geom_compare_to_impl
//       Access: Protected, Virtual
//  Description: Called to compare two GeomMungers who are known to be
//               of the same type, for an apples-to-apples comparison.
//               This will never be called on two pointers of a
//               different type.
////////////////////////////////////////////////////////////////////
int DXGeomMunger7::
geom_compare_to_impl(const GeomMunger *other) const {
  // Unlike GLGeomMunger, we do consider _texture and _tex_gen
  // important for this purpose, since they control the number and
  // order of texture coordinates we might put into the FVF.
  const DXGeomMunger7 *om = DCAST(DXGeomMunger7, other);
  if (_texture != om->_texture) {
    return _texture < om->_texture ? -1 : 1;
  }
  if (_tex_gen != om->_tex_gen) {
    return _tex_gen < om->_tex_gen ? -1 : 1;
  }

  return StandardMunger::geom_compare_to_impl(other);
}
