// Filename: builderPrim.cxx
// Created by:  drose (10Sep97)
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

#include "builderPrim.h"
#include "notify.h"
// Tell GCC that we'll take care of the instantiation explicitly here.
#ifdef __GNUC__
#pragma implementation
#endif

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrim::nonindexed_copy
//       Access: Public
//  Description: Makes a nonindexed copy of the given indexed prim, by
//               looking up the current values of the indexed
//               coordinates in the given bucket.
////////////////////////////////////////////////////////////////////
BuilderPrim &BuilderPrim::
nonindexed_copy(const BuilderPrimTempl<BuilderVertexI> &copy,
                const BuilderBucket &bucket) {
  clear();

  set_type(copy.get_type());

  if (copy.has_normal()) {
    nassertr(bucket.get_normals() != (Normalf *)NULL, *this);
    set_normal(bucket.get_normals()[copy.get_normal()]);
  }
  if (copy.has_color()) {
    nassertr(bucket.get_colors() != (Colorf *)NULL, *this);
    set_color(bucket.get_colors()[copy.get_color()]);
  }
  if (copy.has_pixel_size()) {
    set_pixel_size(copy.get_pixel_size());
  }

  int num_verts = copy.get_num_verts();
  int i;
  for (i = 0; i < num_verts; i++) {
    const BuilderVertexI &cv = copy.get_vertex(i);
    BuilderVertex v;
    if (cv.has_coord()) {
      v.set_coord(cv.get_coord_value(bucket));
    }

    if (cv.has_normal()) {
      v.set_normal(cv.get_normal_value(bucket));
    }

    if (cv.has_color()) {
      v.set_color(cv.get_color_value(bucket));
    }
    
    BuilderVertexI::tc_const_iterator tci;
    for (tci = cv.tc_begin(); tci != cv.tc_end(); ++tci) {
      const InternalName *name = (*tci).first;
      v.set_texcoord(name, cv.get_texcoord_value(name, bucket));
    }

    if (cv.has_pixel_size()) {
      v.set_pixel_size(cv.get_pixel_size());
    }
    add_vertex(v);
  }
  return *this;
}



////////////////////////////////////////////////////////////////////
//     Function: BuilderPrim::flatten_vertex_properties
//       Access: Public
//  Description: If all the vertices of the primitive have the same
//               normal, color, etc., removes those properties from
//               the vertices and assigns them to the primitive
//               instead.
//
//               This can provide better meshing by removing
//               properties from otherwise shared vertices.
////////////////////////////////////////////////////////////////////
void BuilderPrim::
flatten_vertex_properties() {
  int num_verts = get_num_verts();
  int i;

  if (has_overall_normal()) {
    set_normal(get_normal());

    for (i = 0; i < num_verts; i++) {
      get_vertex(i).clear_normal();
    }
  }

  if (has_overall_color()) {
    set_color(get_color());

    for (i = 0; i < num_verts; i++) {
      get_vertex(i).clear_color();
    }
  }

  if (has_overall_pixel_size()) {
    set_pixel_size(get_pixel_size());

    for (i = 0; i < num_verts; i++) {
      get_vertex(i).clear_pixel_size();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrim::fill_geom
//       Access: Public
//  Description: Fills up the attribute values of a Geom with the
//               indicated arrays.  This creates a nonindexed Geom.
////////////////////////////////////////////////////////////////////
void BuilderPrim::
fill_geom(Geom *geom, const PTA_BuilderV &v_array,
          GeomBindType n_attr, const PTA_BuilderN &n_array,
          GeomBindType c_attr, const PTA_BuilderC &c_array,
          const BuilderPrim::TexCoordFill &texcoords,
          const BuilderBucket &, int, int, int) {

  // WARNING!  This is questionable practice.  We have a
  // PTA_BuilderV etc.; since a BuilderV is just a proxy
  // to a Vertexf, we can get away with casting this to a
  // PTA_Vertexf.

  geom->set_coords((PTA_Vertexf &)v_array);

  if (n_attr != G_OFF) {
    geom->set_normals((PTA_Normalf &)n_array, n_attr);
  }

  if (c_attr != G_OFF) {
    geom->set_colors((PTA_Colorf &)c_array, c_attr);
  }

  TexCoordFill::const_iterator tci;
  for (tci = texcoords.begin(); tci != texcoords.end(); ++tci) {
    const InternalName *name = (*tci).first;
    geom->set_texcoords(name, (*tci).second);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimI::flatten_vertex_properties
//       Access: Public
//  Description: If all the vertices of the primitive have the same
//               normal, color, etc., removes those properties from
//               the vertices and assigns them to the primitive
//               instead.
//
//               This can do nothing in the case of an indexed
//               primitive, because we can't monkey with the vertex
//               properties in this case.
////////////////////////////////////////////////////////////////////
void BuilderPrimI::
flatten_vertex_properties() {
}

////////////////////////////////////////////////////////////////////
//     Function: BuilderPrimI::fill_geom
//       Access: Public
//  Description: Fills up the attribute values of a Geom with the
//               indicated arrays.  This creates an indexed Geom.
////////////////////////////////////////////////////////////////////
void BuilderPrimI::
fill_geom(Geom *geom, const PTA_ushort &v_array,
          GeomBindType n_attr, PTA_ushort n_array,
          GeomBindType c_attr, PTA_ushort c_array,
          const BuilderPrimI::TexCoordFill &texcoords,
          const BuilderBucket &bucket,
          int num_prims, int num_components, int num_verts) {
  PTA_Vertexf v_data = bucket.get_coords();
  PTA_Normalf n_data = bucket.get_normals();
  PTA_Colorf c_data = bucket.get_colors();

  // Make sure the data pointers are NULL if the attribute is off.
  if (n_attr == G_OFF) {
    n_data = NULL;
    n_array = NULL;
  }
  if (c_attr == G_OFF) {
    c_data = NULL;
    c_array = NULL;
  }

  int n_len =
    (n_attr==G_PER_VERTEX) ? num_verts :
    (n_attr==G_PER_COMPONENT) ? num_components :
    (n_attr==G_PER_PRIM) ? num_prims :
    (n_attr==G_OVERALL) ? 1 : 0;
  int c_len =
    (c_attr==G_PER_VERTEX) ? num_verts :
    (c_attr==G_PER_COMPONENT) ? num_components :
    (c_attr==G_PER_PRIM) ? num_prims :
    (c_attr==G_OVERALL) ? 1 : 0;

  // See if we can share some of the index lists.
  if (n_attr != G_OFF &&
      memcmp(v_array, n_array, sizeof(ushort) * n_len)==0) {
    n_array = v_array;
  }
  if (c_attr != G_OFF) {
    if (memcmp(v_array, c_array, sizeof(ushort) * c_len)==0) {
      c_array = v_array;
    } else if (c_len <= n_len &&
               memcmp(n_array, c_array, sizeof(ushort) * c_len)==0) {
      c_array = n_array;
    }
  }

  geom->set_coords(v_data, v_array);

  if (n_attr != G_OFF) {
    geom->set_normals(n_data, n_attr, n_array);
  }

  if (c_attr != G_OFF) {
    geom->set_colors(c_data, c_attr, c_array);
  }

  TexCoordFill::const_iterator tci;
  for (tci = texcoords.begin(); tci != texcoords.end(); ++tci) {
    const InternalName *name = (*tci).first;
    PTA_ushort t_array = (*tci).second;

    PTA_TexCoordf t_data = bucket.get_texcoords(name);
    if (t_data != (TexCoordf *)NULL) {
      int t_len = num_verts;

      // Can we share the index list with some other property?
      if (memcmp(v_array, t_array, sizeof(ushort) * t_len)==0) {
        t_array = v_array;
      } else if (t_len <= n_len &&
                 memcmp(n_array, t_array, sizeof(ushort) * t_len)==0) {
        t_array = n_array;
      } else if (t_len <= c_len &&
                 memcmp(c_array, t_array, sizeof(ushort) * t_len)==0) {
        t_array = c_array;
        
      } else {
        TexCoordFill::const_iterator tci2;
        for (tci2 = texcoords.begin(); tci2 != tci; ++tci2) {
          PTA_ushort t_array2 = (*tci).second;
          if (memcmp(t_array2, t_array, sizeof(ushort) * t_len)==0) {
            t_array = t_array2;
            break;
          }
        }
      }

      geom->set_texcoords(name, t_data, t_array);
    }
  }
}

