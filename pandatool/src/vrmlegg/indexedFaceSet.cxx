/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file indexedFaceSet.cxx
 * @author drose
 * @date 1999-06-24
 */

#include "indexedFaceSet.h"
#include "vrmlAppearance.h"
#include "vrmlNodeType.h"
#include "vrmlNode.h"
#include "pnotify.h"

#include "eggGroup.h"
#include "eggVertex.h"
#include "eggVertexPool.h"
#include "eggPolygon.h"

using std::cerr;

/**
 *
 */
IndexedFaceSet::
IndexedFaceSet(const VrmlNode *geometry, const VRMLAppearance &appearance) :
  _geometry(geometry), _appearance(appearance)
{
  get_coord_values();
  get_polys();
  get_colors();
  _has_normals = get_normals();
  if (!_per_vertex_normals.empty()) {
    assign_per_vertex_normals();
  }
  get_uvs();
  if (!_per_vertex_uvs.empty()) {
    assign_per_vertex_uvs();
  }
}


/**
 *
 */
void IndexedFaceSet::
convert_to_egg(EggGroup *group, const LMatrix4d &net_transform) {
  EggVertexPool *vpool = new EggVertexPool(group->get_name());
  group->add_child(vpool);

  make_polys(vpool, group, net_transform);
  if (!_has_normals && _appearance._has_material) {
    compute_normals(group);
  }
}


/**
 *
 */
void IndexedFaceSet::
get_coord_values() {
  const VrmlNode *coord = _geometry->get_value("coord")._sfnode._p;

  if (coord != nullptr) {
    const MFArray *point = coord->get_value("point")._mf;
    MFArray::const_iterator ci;
    for (ci = point->begin(); ci != point->end(); ++ci) {
      const double *p = (*ci)._sfvec;
      _coord_values.push_back(LVertexd(p[0], p[1], p[2]));
    }
  }
}


/**
 *
 */
void IndexedFaceSet::
get_polys() {
  const MFArray *coordIndex = _geometry->get_value("coordIndex")._mf;
  VrmlPolygon poly;

  MFArray::const_iterator ci;
  for (ci = coordIndex->begin(); ci != coordIndex->end(); ++ci) {
    if ((*ci)._sfint32 < 0) {
      _polys.push_back(poly);
      poly._verts.clear();
    } else {
      const LVertexd &p = _coord_values[(*ci)._sfint32];
      VrmlVertex vert;
      vert._index = (*ci)._sfint32;
      vert._pos = p;
      poly._verts.push_back(vert);
    }
  }
}

/**
 * Builds up a vector of LColor pointers corresponding to the VRML color node.
 */
void IndexedFaceSet::
get_vrml_colors(const VrmlNode *color_node, double transparency,
                pvector<UnalignedLVecBase4> &color_list) {
  const MFArray *color = color_node->get_value("color")._mf;
  MFArray::const_iterator ci;
  for (ci = color->begin(); ci != color->end(); ++ci) {
    const double *p = (*ci)._sfvec;
    LColor color(p[0], p[1], p[2], 1.0 - transparency);
    color_list.push_back(color);
  }
}

/**
 * Builds up a vector of double array pointers corresponding to the VRML
 * normal node.
 */
void IndexedFaceSet::
get_vrml_normals(const VrmlNode *normal_node,
                 pvector<LNormald> &normal_list) {
  const MFArray *point = normal_node->get_value("vector")._mf;
  MFArray::const_iterator ci;
  for (ci = point->begin(); ci != point->end(); ++ci) {
    const double *p = (*ci)._sfvec;
    LNormald normal(p[0], p[1], p[2]);
    normal_list.push_back(normal);
  }
}

/**
 * Builds up a vector of double array pointers corresponding to the VRML
 * texCoord node.
 */
void IndexedFaceSet::
get_vrml_uvs(const VrmlNode *texCoord_node,
             pvector<LTexCoordd> &uv_list) {
  const MFArray *point = texCoord_node->get_value("point")._mf;
  MFArray::const_iterator ci;
  for (ci = point->begin(); ci != point->end(); ++ci) {
    const double *p = (*ci)._sfvec;
    LTexCoordd uv(p[0], p[1]);
    uv_list.push_back(uv);
  }
}


/**
 *
 */
bool IndexedFaceSet::
get_colors() {
  const VrmlNode *color = _geometry->get_value("color")._sfnode._p;
  if (color != nullptr) {
    // Vertex or face colors.
    pvector<UnalignedLVecBase4> color_list;
    get_vrml_colors(color, _appearance._transparency, color_list);

    bool colorPerVertex = _geometry->get_value("colorPerVertex")._sfbool;
    MFArray *colorIndex = _geometry->get_value("colorIndex")._mf;
    if (colorPerVertex) {
      MFArray::const_iterator ci;
      size_t pi = 0;
      size_t pv = 0;
      for (ci = colorIndex->begin(); ci != colorIndex->end(); ++ci) {
        if ((*ci)._sfint32 < 0) {
          // End of poly.
          if (pv != _polys[pi]._verts.size()) {
            cerr << "Color indices don't match up!\n";
            return false;
          }
          pi++;
          pv = 0;
        } else {
          if (pi >= _polys.size() || pv >= _polys[pi]._verts.size()) {
            cerr << "Color indices don't match up!\n";
            return false;
          }
          _polys[pi]._verts[pv]._attrib.set_color(color_list[(*ci)._sfint32]);
          pv++;
        }
      }
      if (pi != _polys.size()) {
        cerr << "Not enough color indices!\n";
        return false;
      }
    } else {
      if (!colorIndex->empty()) {
        MFArray::const_iterator ci;
        size_t pi = 0;
        if (colorIndex->size() != _polys.size()) {
          cerr << "Wrong number of color indices!\n";
          return false;
        }
        for (ci = colorIndex->begin(); ci != colorIndex->end(); ++ci) {
          if ((*ci)._sfint32 < 0 || (*ci)._sfint32 >= (int)color_list.size()) {
            cerr << "Invalid color index!\n";
            return false;
          }
          _polys[pi]._attrib.set_color(color_list[(*ci)._sfint32]);
          pi++;
        }
      } else {
        if (color_list.size() != _polys.size()) {
          cerr << "Wrong number of colors!\n";
          return false;
        }
        for (size_t pi = 0; pi < color_list.size(); pi++) {
          _polys[pi]._attrib.set_color(color_list[pi]);
        }
      }
    }
    return true;
  }
  return false;
}

/**
 *
 */
bool IndexedFaceSet::
get_normals() {
  const VrmlNode *normal = _geometry->get_value("normal")._sfnode._p;
  if (normal != nullptr) {
    // Vertex or face normals.
    pvector<LNormald> normal_list;
    get_vrml_normals(normal, normal_list);

    bool normalPerVertex = _geometry->get_value("normalPerVertex")._sfbool;
    MFArray *normalIndex = _geometry->get_value("normalIndex")._mf;
    MFArray::const_iterator ci;

    if (normalPerVertex &&
        normal_list.size() == _polys.size() &&
        normalIndex->empty()) {
      // Here's an interesting formZ bug.  We end up with a VRML file that
      // claims to have normals per vertex, yet there is no normal index list,
      // and there are exactly enough normals in the list to indicate one
      // normal per face.  Silly formZ.
      normalPerVertex = false;
    }

    if (normalPerVertex) {

      if (normalIndex->empty()) {
        // If we have *no* normal index array, but we do have per-vertex
        // normals, assume the VRML writer meant to imply a one-to-one
        // mapping.  This works around a broken formZ VRML file writer.
        for (size_t i = 0; i < normal_list.size(); i++) {
          VrmlFieldValue fv;
          fv._sfint32 = i;
          (*normalIndex).push_back(fv);
        }
      }

      // It's possible that this .wrl file indexes normals directly into the
      // vertex array, instead of into the polygon list.  Check for this
      // possibility.  This can only happen if the number of normal indices
      // exactly matches the number of vertices, and none of the indices is
      // -1.
      bool linear_list = (normalIndex->size() == _coord_values.size());
      for (ci = normalIndex->begin();
           ci != normalIndex->end() && linear_list;
           ++ci) {
        linear_list = ((*ci)._sfint32 >= 0);
      }

      if (linear_list) {
        // Ok, we do have such a list.  This .wrl file seems to store its
        // texture coordinates one per vertex, instead of one per polygon
        // vertex.
        _per_vertex_normals.reserve(_coord_values.size());

        for (ci = normalIndex->begin(); ci != normalIndex->end(); ++ci) {
          size_t vi = (*ci)._sfint32;
          nassertr(vi >= 0, false);
          if (vi >= normal_list.size()) {
            cerr << "Invalid normal index: " << vi << "\n";
            return false;
          }
          _per_vertex_normals.push_back(normal_list[vi]);
        }
        nassertr(_per_vertex_normals.size() == _coord_values.size(), false);

      } else {
        // This is a "correct" .wrl file that stores its texture coordinates
        // one per polygon vertex.  This allows a shared vertex to contain two
        // different normal values in differing polygons (meaning it's not
        // actually shared).

        MFArray::const_iterator ci;
        size_t pi = 0;
        size_t pv = 0;
        for (ci = normalIndex->begin(); ci != normalIndex->end(); ++ci) {
          if ((*ci)._sfint32 < 0) {
            // End of poly.
            if (pv != _polys[pi]._verts.size()) {
              cerr << "Normal indices don't match up!\n";
              return false;
            }
            pi++;
            pv = 0;
          } else {
            if (pi >= _polys.size() || pv >= _polys[pi]._verts.size()) {
              cerr << "Normal indices don't match up!\n";
              return false;
            }
            const LNormald &d = normal_list[(*ci)._sfint32];
            _polys[pi]._verts[pv]._attrib.set_normal(d);
            pv++;
          }
        }
        if (pi != _polys.size()) {
          cerr << "Not enough normal indices!\n";
          return false;
        }
      }
    } else {
      if (!normalIndex->empty()) {
        size_t pi = 0;
        if (normalIndex->size() != _polys.size()) {
          cerr << "Wrong number of normal indices!\n";
          return false;
        }
        for (ci = normalIndex->begin(); ci != normalIndex->end(); ++ci) {
          if ((*ci)._sfint32 < 0 || (*ci)._sfint32 >= (int)normal_list.size()) {
            cerr << "Invalid normal index!\n";
            return false;
          }
          const LNormald &d = normal_list[(*ci)._sfint32];
          _polys[pi]._attrib.set_normal(d);
          pi++;
        }
      } else {
        if (normal_list.size() != _polys.size()) {
          cerr << "Wrong number of normals!\n";
          return false;
        }
        for (size_t pi = 0; pi < normal_list.size(); pi++) {
          const LNormald &d = normal_list[pi];
          _polys[pi]._attrib.set_normal(d);
        }
      }
    }
    return true;
  }
  return false;
}

/**
 * Once the array of _per_vertex_normals has been filled (by a broken .wrl
 * file that indexes the normal's directly into the vertex array instead of
 * per polygon vertex), go back through the polygons and assign the normals by
 * index number.
 */
void IndexedFaceSet::
assign_per_vertex_normals() {
  for (size_t pi = 0; pi < _polys.size(); pi++) {
    for (size_t pv = 0; pv < _polys[pi]._verts.size(); pv++) {
      VrmlVertex &vv = _polys[pi]._verts[pv];
      if (vv._index >= 0 && vv._index < (int)_per_vertex_normals.size()) {
        const LNormald &d = _per_vertex_normals[vv._index];
        vv._attrib.set_normal(d);
      }
    }
  }
}


/**
 *
 */
bool IndexedFaceSet::
get_uvs() {
  const VrmlNode *texCoord = _geometry->get_value("texCoord")._sfnode._p;
  if (texCoord != nullptr) {
    // Vertex or face texCoords.
    pvector<LTexCoordd> uv_list;
    get_vrml_uvs(texCoord, uv_list);

    MFArray *texCoordIndex = _geometry->get_value("texCoordIndex")._mf;
    MFArray::const_iterator ci;

    if (texCoordIndex->empty()) {
      // If we have *no* texture coordinate index array, but we do have
      // texture coordinates, assume the VRML writer meant to imply a one-to-
      // one mapping.  This works around a broken formZ VRML file writer.
      for (size_t i = 0; i < uv_list.size(); i++) {
        VrmlFieldValue fv;
        fv._sfint32 = i;
        (*texCoordIndex).push_back(fv);
      }
    }

    // It's possible that this .wrl file indexes texture coordinates directly
    // into the vertex array, instead of into the polygon list.  Check for
    // this possibility.  This can only happen if the number of texture
    // coordinate indices exactly matches the number of vertices, and none of
    // the indices is -1.
    bool linear_list = (texCoordIndex->size() == _coord_values.size());
    for (ci = texCoordIndex->begin();
         ci != texCoordIndex->end() && linear_list;
         ++ci) {
      linear_list = ((*ci)._sfint32 >= 0);
    }

    if (linear_list) {
      // Ok, we do have such a list.  This .wrl file seems to store its
      // texture coordinates one per vertex, instead of one per polygon
      // vertex.
      _per_vertex_uvs.reserve(_coord_values.size());

      for (ci = texCoordIndex->begin(); ci != texCoordIndex->end(); ++ci) {
        size_t vi = (*ci)._sfint32;
        nassertr(vi >= 0, false);
        if (vi >= uv_list.size()) {
          cerr << "Invalid texCoord index: " << vi << "\n";
          return false;
        }
        _per_vertex_uvs.push_back(uv_list[vi]);
      }
      nassertr(_per_vertex_uvs.size() == _coord_values.size(), false);

    } else {
      // This is a "correct" .wrl file that stores its texture coordinates one
      // per polygon vertex.  This allows a shared vertex to contain two
      // different texture coordinate values in differing polygons (meaning
      // it's not actually shared).

      size_t pi = 0;
      size_t pv = 0;
      for (ci = texCoordIndex->begin(); ci != texCoordIndex->end(); ++ci) {
        if ((*ci)._sfint32 < 0) {
          // End of poly.
          if (pv != _polys[pi]._verts.size()) {
            cerr << "texCoord indices don't match up!\n";
            return false;
          }
          pi++;
          pv = 0;
        } else {
          if (pi >= _polys.size() || pv >= _polys[pi]._verts.size()) {
            cerr << "texCoord indices don't match up!\n";
            return false;
          }
          _polys[pi]._verts[pv]._attrib.set_uv(uv_list[(*ci)._sfint32]);
          pv++;
        }
      }
      if (pi != _polys.size()) {
        cerr << "Not enough texCoord indices!\n";
        return false;
      }
    }
    return true;
  }
  return false;
}

/**
 * Once the array of _per_vertex_uvs has been filled (by a broken .wrl file
 * that indexes the uv's directly into the vertex array instead of per polygon
 * vertex), go back through the polygons and assign the UV's by index number.
 */
void IndexedFaceSet::
assign_per_vertex_uvs() {
  for (size_t pi = 0; pi < _polys.size(); pi++) {
    for (size_t pv = 0; pv < _polys[pi]._verts.size(); pv++) {
      VrmlVertex &vv = _polys[pi]._verts[pv];
      if (vv._index >= 0 && vv._index < (int)_per_vertex_uvs.size()) {
        const LTexCoordd &d = _per_vertex_uvs[vv._index];
        vv._attrib.set_uv(d);
      }
    }
  }
}


/**
 *
 */
void IndexedFaceSet::
make_polys(EggVertexPool *vpool, EggGroup *group,
           const LMatrix4d &net_transform) {
  bool ccw = _geometry->get_value("ccw")._sfbool;
  bool solid = _geometry->get_value("solid")._sfbool;

  for (size_t pi = 0; pi < _polys.size(); pi++) {
    EggPolygon *poly = new EggPolygon;
    group->add_child(poly);
    poly->copy_attributes(_polys[pi]._attrib);

    if (!poly->has_color() && _appearance._has_material) {
      poly->set_color(_appearance._color);
    }

    if (_appearance._tex != nullptr) {
      poly->set_texture(_appearance._tex);
    }

    if (!solid) {
      poly->set_bface_flag(true);
    }

    if (ccw) {
      // The vertices are counterclockwise, same as Egg.
      for (int pv = 0; pv < (int)_polys[pi]._verts.size(); pv++) {
        EggVertex vert(_polys[pi]._verts[pv]._attrib);
        LVertexd pos =
          _polys[pi]._verts[pv]._pos * net_transform;
        vert.set_pos(pos);

        poly->add_vertex(vpool->create_unique_vertex(vert));
      }
    } else {
      // The vertices are clockwise, so add 'em in reverse order.
      for (int pv = (int)_polys[pi]._verts.size() - 1; pv >= 0; pv--) {
        EggVertex vert(_polys[pi]._verts[pv]._attrib);
        LVertexd pos =
          _polys[pi]._verts[pv]._pos * net_transform;
        vert.set_pos(pos);

        poly->add_vertex(vpool->create_unique_vertex(vert));
      }
    }
  }
}


/**
 *
 */
void IndexedFaceSet::
compute_normals(EggGroup *group) {
  const VrmlNode *normal = _geometry->get_value("normal")._sfnode._p;
  if (normal == nullptr) {
    // Compute normals.
    double creaseAngle = _geometry->get_value("creaseAngle")._sffloat;
    if (creaseAngle == 0.0) {
      group->recompute_polygon_normals();
    } else {
      group->recompute_vertex_normals(rad_2_deg(creaseAngle));
    }
  }
}
