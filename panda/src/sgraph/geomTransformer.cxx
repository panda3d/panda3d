// Filename: geomTransformer.cxx
// Created by:  drose (23May00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "geomTransformer.h"
#include "geomNode.h"


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomTransformer::
GeomTransformer() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
GeomTransformer::
~GeomTransformer() {
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_vertices
//       Access: Public
//  Description: Transforms the vertices and the normals in the
//               indicated Geom by the indicated matrix.  Returns true
//               if the Geom was changed, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_vertices(Geom *geom, const LMatrix4f &mat) {
  bool transformed = false;

  nassertr(geom != (Geom *)NULL, false);

  PTA_Vertexf coords;
  PTA_ushort index;

  geom->get_coords(coords, index);

  if (!coords.empty()) {
    // Look up the Geom's coords in our table--have we already
    // transformed this array?
    SourceVertices sv;
    sv._mat = mat;
    sv._coords = coords;

    PTA_Vertexf &new_coords = _vertices[sv];

    if (new_coords.is_null()) {
      // We have not transformed the array yet.  Do so now.
      new_coords.reserve(coords.size());
      PTA_Vertexf::const_iterator vi;
      for (vi = coords.begin(); vi != coords.end(); ++vi) {
        new_coords.push_back((*vi) * mat);
      }
      nassertr(new_coords.size() == coords.size(), false);
    }

    geom->set_coords(new_coords, index);
    transformed = true;
  }

  // Now do the same thing for normals.
  PTA_Normalf norms;
  GeomBindType bind;

  geom->get_normals(norms, bind, index);

  if (bind != G_OFF) {
    SourceNormals sn;
    sn._mat = mat;
    sn._norms = norms;

    PTA_Normalf &new_norms = _normals[sn];

    if (new_norms.is_null()) {
      // We have not transformed the array yet.  Do so now.
      new_norms.reserve(norms.size());
      PTA_Normalf::const_iterator ni;
      for (ni = norms.begin(); ni != norms.end(); ++ni) {
        Normalf new_norm = (*ni) * mat;
        new_norm.normalize();
        new_norms.push_back(new_norm);
      }
      nassertr(new_norms.size() == norms.size(), false);
    }

    geom->set_normals(new_norms, bind, index);
    transformed = true;
  }

  return transformed;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_vertices
//       Access: Public
//  Description: Transforms the vertices and the normals in all of the
//               Geoms within the indicated GeomNode by the indicated
//               matrix.  Does not destructively change Geoms;
//               instead, a copy will be made of each Geom to be
//               changed, in case multiple GeomNodes reference the
//               same Geom. Returns true if the GeomNode was changed,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_vertices(GeomNode *node, const LMatrix4f &mat) {
  bool any_changed = false;

  GeomNode::Geoms new_geoms;

  GeomNode::Geoms::const_iterator gi;
  for (gi = node->_geoms.begin(); gi != node->_geoms.end(); ++gi) {
    dDrawable *drawable = (*gi);
    if (drawable->is_of_type(Geom::get_class_type())) {
      Geom *geom = DCAST(Geom, drawable);
      PT(Geom) new_geom = geom->make_copy();
      if (transform_vertices(new_geom, mat)) {
        new_geoms.push_back(new_geom.p());
        any_changed = true;
      } else {
        new_geoms.push_back(geom);
      }
    } else {
      new_geoms.push_back(drawable);
    }
  }

  if (any_changed) {
    node->_geoms = new_geoms;
    node->unprepare();
    node->mark_bound_stale();
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_texcoords
//       Access: Public
//  Description: Transforms the texture coordinates in the indicated
//               Geom by the indicated matrix.  Returns true if the
//               Geom was changed, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_texcoords(Geom *geom, const LMatrix4f &mat) {
  bool transformed = false;

  nassertr(geom != (Geom *)NULL, false);

  PTA_TexCoordf texcoords;
  GeomBindType bind;
  PTA_ushort index;

  geom->get_texcoords(texcoords, bind, index);

  if (bind != G_OFF) {
    // Look up the Geom's texcoords in our table--have we already
    // transformed this array?
    SourceTexCoords stc;
    stc._mat = mat;
    stc._texcoords = texcoords;

    PTA_TexCoordf &new_texcoords = _texcoords[stc];

    if (new_texcoords.is_null()) {
      // We have not transformed the array yet.  Do so now.
      new_texcoords.reserve(texcoords.size());
      PTA_TexCoordf::const_iterator tci;
      for (tci = texcoords.begin(); tci != texcoords.end(); ++tci) {
        const TexCoordf &tc = (*tci);
        LVecBase4f v4(tc[0], tc[1], 0.0f, 1.0f);
        v4 = v4 * mat;
        new_texcoords.push_back(TexCoordf(v4[0] / v4[3], v4[1] / v4[3]));
      }
      nassertr(new_texcoords.size() == texcoords.size(), false);
    }

    geom->set_texcoords(new_texcoords, bind, index);
    transformed = true;
  }

  return transformed;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_texcoords
//       Access: Public
//  Description: Transforms the texcoords and the normals in all of the
//               Geoms within the indicated GeomNode by the indicated
//               matrix.  Does not destructively change Geoms;
//               instead, a copy will be made of each Geom to be
//               changed, in case multiple GeomNodes reference the
//               same Geom. Returns true if the GeomNode was changed,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_texcoords(GeomNode *node, const LMatrix4f &mat) {
  bool any_changed = false;

  GeomNode::Geoms new_geoms;

  GeomNode::Geoms::const_iterator gi;
  for (gi = node->_geoms.begin(); gi != node->_geoms.end(); ++gi) {
    dDrawable *drawable = (*gi);
    if (drawable->is_of_type(Geom::get_class_type())) {
      Geom *geom = DCAST(Geom, drawable);
      PT(Geom) new_geom = geom->make_copy();
      if (transform_texcoords(new_geom, mat)) {
        new_geoms.push_back(new_geom.p());
        any_changed = true;
      } else {
        new_geoms.push_back(geom);
      }
    } else {
      new_geoms.push_back(drawable);
    }
  }

  if (any_changed) {
    node->_geoms = new_geoms;
    return true;
  }

  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::set_color
//       Access: Public
//  Description: Overrides the color indicated within the Geom with
//               the given replacement color.  Returns true if the
//               Geom was changed, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
set_color(Geom *geom, const Colorf &color) {
  // In this case, we always replace whatever color array was there
  // with a new color array containing just this color.

  // We do want to share this one-element array between Geoms, though.
  PTA_Colorf &new_colors = _fcolors[color];

  if (new_colors.is_null()) {
    // We haven't seen this color before; define a new color array.
    new_colors.push_back(color);
  }

  geom->set_colors(new_colors, G_OVERALL);
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_texcoords
//       Access: Public
//  Description: Overrides the color indicated within the GeomNode
//               with the given replacement color.  Returns true if
//               any Geom in the GeomNode was changed, false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
set_color(GeomNode *node, const Colorf &color) {
  bool any_changed = false;

  GeomNode::Geoms new_geoms;

  GeomNode::Geoms::const_iterator gi;
  for (gi = node->_geoms.begin(); gi != node->_geoms.end(); ++gi) {
    dDrawable *drawable = (*gi);
    if (drawable->is_of_type(Geom::get_class_type())) {
      Geom *geom = DCAST(Geom, drawable);
      PT(Geom) new_geom = geom->make_copy();
      if (set_color(new_geom, color)) {
        new_geoms.push_back(new_geom.p());
        any_changed = true;
      } else {
        new_geoms.push_back(geom);
      }
    } else {
      new_geoms.push_back(drawable);
    }
  }

  if (any_changed) {
    node->_geoms = new_geoms;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_colors
//       Access: Public
//  Description: Transforms the colors in the indicated
//               Geom by the indicated matrix.  Returns true if the
//               Geom was changed, false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_colors(Geom *geom, const LMatrix4f &mat,
                 float alpha_scale, float alpha_offset) {
  bool transformed = false;

  nassertr(geom != (Geom *)NULL, false);

  PTA_Colorf colors;
  GeomBindType bind;
  PTA_ushort index;

  geom->get_colors(colors, bind, index);

  if (bind != G_OFF) {
    // Look up the Geom's colors in our table--have we already
    // transformed this array?
    SourceColors sc;
    sc._mat = mat;
    sc._alpha_scale = alpha_scale;
    sc._alpha_offset = alpha_offset;
    sc._colors = colors;

    PTA_Colorf &new_colors = _tcolors[sc];

    if (new_colors.is_null()) {
      // We have not transformed the array yet.  Do so now.
      new_colors.reserve(colors.size());
      PTA_Colorf::const_iterator ci;
      for (ci = colors.begin(); ci != colors.end(); ++ci) {
        const Colorf &c = (*ci);

        LPoint3f temp(c[0], c[1], c[2]);
        temp = temp * mat;
        float alpha = (c[3] * alpha_scale) + alpha_offset;
        
        Colorf transformed(temp[0], temp[1], temp[2], alpha);
        new_colors.push_back(transformed);
      }
      nassertr(new_colors.size() == colors.size(), false);
    }

    geom->set_colors(new_colors, bind, index);
    transformed = true;
  }

  return transformed;
}


////////////////////////////////////////////////////////////////////
//     Function: GeomTransformer::transform_colors
//       Access: Public
//  Description: Transforms the colors and the normals in all of the
//               Geoms within the indicated GeomNode by the indicated
//               matrix.  Does not destructively change Geoms;
//               instead, a copy will be made of each Geom to be
//               changed, in case multiple GeomNodes reference the
//               same Geom. Returns true if the GeomNode was changed,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool GeomTransformer::
transform_colors(GeomNode *node, const LMatrix4f &mat,
                 float alpha_scale, float alpha_offset) {
  bool any_changed = false;

  GeomNode::Geoms new_geoms;

  GeomNode::Geoms::const_iterator gi;
  for (gi = node->_geoms.begin(); gi != node->_geoms.end(); ++gi) {
    dDrawable *drawable = (*gi);
    if (drawable->is_of_type(Geom::get_class_type())) {
      Geom *geom = DCAST(Geom, drawable);
      PT(Geom) new_geom = geom->make_copy();
      if (transform_colors(new_geom, mat, alpha_scale, alpha_offset)) {
        new_geoms.push_back(new_geom.p());
        any_changed = true;
      } else {
        new_geoms.push_back(geom);
      }
    } else {
      new_geoms.push_back(drawable);
    }
  }

  if (any_changed) {
    node->_geoms = new_geoms;
    return true;
  }

  return false;
}
