// Filename: xFileMesh.cxx
// Created by:  drose (19Jun01)
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

#include "xFileMesh.h"
#include "xFileFace.h"
#include "xFileVertex.h"
#include "xFileNormal.h"
#include "xFileMaterial.h"
#include "xFileDataNode.h"
#include "config_xfile.h"
#include "string_utils.h"
#include "eggVertexPool.h"
#include "eggVertex.h"
#include "eggPolygon.h"
#include "eggGroupNode.h"

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileMesh::
XFileMesh(CoordinateSystem cs) : _cs(cs) {
  _has_normals = false;
  _has_colors = false;
  _has_uvs = false;
  _has_materials = false;
  _egg_parent = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFileMesh::
~XFileMesh() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::clear
//       Access: Public
//  Description: Empties all data from the mesh.
////////////////////////////////////////////////////////////////////
void XFileMesh::
clear() {
  Vertices::iterator vi;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    XFileVertex *vertex = (*vi);
    delete vertex;
  }
  Normals::iterator ni;
  for (ni = _normals.begin(); ni != _normals.end(); ++ni) {
    XFileNormal *normal = (*ni);
    delete normal;
  }
  Materials::iterator mi;
  for (mi = _materials.begin(); mi != _materials.end(); ++mi) {
    XFileMaterial *material = (*mi);
    delete material;
  }
  Faces::iterator fi;
  for (fi = _faces.begin(); fi != _faces.end(); ++fi) {
    XFileFace *face = (*fi);
    delete face;
  }

  _vertices.clear();
  _normals.clear();
  _materials.clear();
  _faces.clear();

  _unique_vertices.clear();
  _unique_normals.clear();
  _unique_materials.clear();

  _has_normals = false;
  _has_colors = false;
  _has_uvs = false;
  _has_materials = false;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::add_polygon
//       Access: Public
//  Description: Adds the indicated polygon to the mesh.
////////////////////////////////////////////////////////////////////
void XFileMesh::
add_polygon(EggPolygon *egg_poly) {
  XFileFace *face = new XFileFace;
  face->set_from_egg(this, egg_poly);
  _faces.push_back(face);
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::add_vertex
//       Access: Public
//  Description: Creates a new XFileVertex, if one does not already
//               exist for the indicated vertex, and returns its
//               index.
////////////////////////////////////////////////////////////////////
int XFileMesh::
add_vertex(EggVertex *egg_vertex, EggPrimitive *egg_prim) {
  int next_index = _vertices.size();
  XFileVertex *vertex = new XFileVertex;
  vertex->set_from_egg(egg_vertex, egg_prim);
  if (vertex->_has_color) {
    _has_colors = true;
  }
  if (vertex->_has_uv) {
    _has_uvs = true;
  }

  pair<UniqueVertices::iterator, bool> result =
    _unique_vertices.insert(UniqueVertices::value_type(vertex, next_index));

  if (result.second) {
    // Successfully added; this is a new vertex.
    _vertices.push_back(vertex);
    return next_index;
  } else {
    // Not successfully added; there is already a vertex with these
    // properties.  Return that one instead.
    delete vertex;
    return (*result.first).second;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::add_normal
//       Access: Public
//  Description: Creates a new XFileNormal, if one does not already
//               exist for the indicated normal, and returns its
//               index.
////////////////////////////////////////////////////////////////////
int XFileMesh::
add_normal(EggVertex *egg_vertex, EggPrimitive *egg_prim) {
  int next_index = _normals.size();
  XFileNormal *normal = new XFileNormal;
  normal->set_from_egg(egg_vertex, egg_prim);
  if (normal->_has_normal) {
    _has_normals = true;
  }

  pair<UniqueNormals::iterator, bool> result =
    _unique_normals.insert(UniqueNormals::value_type(normal, next_index));

  if (result.second) {
    // Successfully added; this is a new normal.
    _normals.push_back(normal);
    return next_index;
  } else {
    // Not successfully added; there is already a normal with these
    // properties.  Return that one instead.
    delete normal;
    return (*result.first).second;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::add_material
//       Access: Public
//  Description: Creates a new XFileMaterial, if one does not already
//               exist for the indicated material, and returns its
//               index.
////////////////////////////////////////////////////////////////////
int XFileMesh::
add_material(EggPrimitive *egg_prim) {
  int next_index = _materials.size();
  XFileMaterial *material = new XFileMaterial;
  material->set_from_egg(egg_prim);
  if (material->has_material()) {
    _has_materials = true;
  }

  pair<UniqueMaterials::iterator, bool> result =
    _unique_materials.insert(UniqueMaterials::value_type(material, next_index));

  if (result.second) {
    // Successfully added; this is a new material.
    _materials.push_back(material);
    return next_index;
  } else {
    // Not successfully added; there is already a material with these
    // properties.  Return that one instead.
    delete material;
    return (*result.first).second;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::add_vertex
//       Access: Public
//  Description: Adds the newly-created XFileVertex unequivocally to
//               the mesh, returning its index number.  The XFileMesh
//               object becomes the owner of the XFileVertex
//               pointer, and will delete it when it destructs.
////////////////////////////////////////////////////////////////////
int XFileMesh::
add_vertex(XFileVertex *vertex) {
  int next_index = _vertices.size();
  _unique_vertices.insert(UniqueVertices::value_type(vertex, next_index));
  _vertices.push_back(vertex);
  return next_index;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::add_normal
//       Access: Public
//  Description: Adds the newly-created XFileNormal unequivocally to
//               the mesh, returning its index number.  The XFileMesh
//               object becomes the owner of the XFileNormal
//               pointer, and will delete it when it destructs.
////////////////////////////////////////////////////////////////////
int XFileMesh::
add_normal(XFileNormal *normal) {
  int next_index = _normals.size();
  _unique_normals.insert(UniqueNormals::value_type(normal, next_index));
  _normals.push_back(normal);
  return next_index;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::add_material
//       Access: Public
//  Description: Adds the newly-created XFileMaterial unequivocally to
//               the mesh, returning its index number.  The XFileMesh
//               object becomes the owner of the XFileMaterial
//               pointer, and will delete it when it destructs.
////////////////////////////////////////////////////////////////////
int XFileMesh::
add_material(XFileMaterial *material) {
  int next_index = _materials.size();
  _unique_materials.insert(UniqueMaterials::value_type(material, next_index));
  _materials.push_back(material);
  return next_index;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::set_egg_parent
//       Access: Public
//  Description: Specifies the egg node that will eventually be the
//               parent of this mesh, when create_polygons() is later
//               called.
////////////////////////////////////////////////////////////////////
void XFileMesh::
set_egg_parent(EggGroupNode *egg_parent) {
  // We actually put the mesh under its own group.
  EggGroup *egg_group = new EggGroup(get_name());
  egg_parent->add_child(egg_group);

  _egg_parent = egg_group;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::create_polygons
//       Access: Public
//  Description: Creates a slew of EggPolygons according to the faces
//               in the mesh, and adds them to the
//               previously-indicated parent node.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
create_polygons(XFileToEggConverter *converter) {
  nassertr(_egg_parent != (EggGroupNode *)NULL, false);

  EggVertexPool *vpool = new EggVertexPool(get_name());
  _egg_parent->add_child(vpool);
  Faces::const_iterator fi;
  for (fi = _faces.begin(); fi != _faces.end(); ++fi) {
    XFileFace *face = (*fi);

    EggPolygon *egg_poly = new EggPolygon;
    _egg_parent->add_child(egg_poly);

    // Set up the vertices for the polygon.
    XFileFace::Vertices::reverse_iterator vi;
    for (vi = face->_vertices.rbegin(); vi != face->_vertices.rend(); ++vi) {
      int vertex_index = (*vi)._vertex_index;
      int normal_index = (*vi)._normal_index;
      if (vertex_index < 0 || vertex_index >= (int)_vertices.size()) {
        xfile_cat.error()
          << "Vertex index out of range in Mesh.\n";
        return false;
      }
      XFileVertex *vertex = _vertices[vertex_index];
      XFileNormal *normal = (XFileNormal *)NULL;

      if (normal_index >= 0 && normal_index < (int)_normals.size()) {
        normal = _normals[normal_index];
      }

      // Create a temporary EggVertex before adding it to the pool.
      EggVertex temp_vtx;
      temp_vtx.set_external_index(vertex_index);
      temp_vtx.set_pos(LCAST(double, vertex->_point));
      if (vertex->_has_color) {
        temp_vtx.set_color(vertex->_color);
      }
      if (vertex->_has_uv) {
        TexCoordd uv = LCAST(double, vertex->_uv);
        // Windows draws the UV's upside-down.
        uv[1] = 1.0 - uv[1];
        temp_vtx.set_uv(uv);
      }

      if (normal != (XFileNormal *)NULL && normal->_has_normal) {
        temp_vtx.set_normal(LCAST(double, normal->_normal));
      }

      // We are given the vertex in local space; we need to transform
      // it into global space.  If the vertex has been skinned, that
      // means the global space of all of its joints (modified by the
      // matrix_offset provided in the skinning data).
      double net_weight = 0.0;
      LMatrix4d weighted_transform(0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0,
                                   0.0, 0.0, 0.0, 0.0);
      SkinWeights::const_iterator swi;
      for (swi = _skin_weights.begin(); swi != _skin_weights.end(); ++swi) {
        const SkinWeightsData &data = (*swi);
        WeightMap::const_iterator wmi = data._weight_map.find(vertex_index);
        if (wmi != data._weight_map.end()) {
          EggGroup *joint = converter->find_joint(data._joint_name);
          if (joint != (EggGroup *)NULL) {
            double weight = (*wmi).second;
            LMatrix4d mat = LCAST(double, data._matrix_offset);
            mat *= joint->get_node_to_vertex();
            weighted_transform += mat * weight;
            net_weight += weight;
          }
        }
      }

      if (net_weight == 0.0) {
        // The vertex had no joint membership.  Transform it into the
        // appropriate (global) space based on its parent.
        temp_vtx.transform(_egg_parent->get_node_to_vertex());

      } else {
        // The vertex was skinned into one or more joints.  Therefore,
        // transform it according to the blended matrix_offset from
        // the skinning data.
        weighted_transform /= net_weight;
        temp_vtx.transform(weighted_transform);
      }

      // Now get a real EggVertex matching our template.
      EggVertex *egg_vtx = vpool->create_unique_vertex(temp_vtx);
      egg_poly->add_vertex(egg_vtx);
    }

    // And apply the material for the polygon.
    int material_index = face->_material_index;
    if (material_index >= 0 && material_index < (int)_materials.size()) {
      XFileMaterial *material = _materials[material_index];
      material->apply_to_egg(egg_poly, converter);
    }
  }

  // Now go through all of the vertices and skin them up.
  EggVertexPool::iterator vi;
  for (vi = vpool->begin(); vi != vpool->end(); ++vi) {
    EggVertex *egg_vtx = (*vi);
    int vertex_index = egg_vtx->get_external_index();

    SkinWeights::const_iterator swi;
    for (swi = _skin_weights.begin(); swi != _skin_weights.end(); ++swi) {
      const SkinWeightsData &data = (*swi);
      WeightMap::const_iterator wmi = data._weight_map.find(vertex_index);
      if (wmi != data._weight_map.end()) {
        EggGroup *joint = converter->find_joint(data._joint_name);
        if (joint != (EggGroup *)NULL) {
          double weight = (*wmi).second;
          joint->ref_vertex(egg_vtx, weight);
        }
      }
    }
  }
    
  if (!has_normals()) {
    // If we don't have explicit normals, make some up, per the DX
    // spec.  Since the DX spec doesn't mention anything about a
    // crease angle, we should be as generous as possible.
    _egg_parent->recompute_vertex_normals(180.0, _cs);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::has_normals
//       Access: Public
//  Description: Returns true if any of the vertices or faces added to
//               this mesh used a normal, false otherwise.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
has_normals() const {
  return _has_normals;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::has_colors
//       Access: Public
//  Description: Returns true if any of the vertices or faces added to
//               this mesh used a color, false otherwise.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
has_colors() const {
  return _has_colors;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::has_uvs
//       Access: Public
//  Description: Returns true if any of the vertices added to this
//               mesh used a texture coordinate, false otherwise.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
has_uvs() const {
  return _has_uvs;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::has_materials
//       Access: Public
//  Description: Returns true if any of the faces added to this mesh
//               used a real material, false otherwise.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
has_materials() const {
  return _has_materials;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::get_num_materials
//       Access: Public
//  Description: Returns the number of distinct materials associated
//               with the mesh.
////////////////////////////////////////////////////////////////////
int XFileMesh::
get_num_materials() const {
  return _materials.size();
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::get_material
//       Access: Public
//  Description: Returns a pointer to the nth materials associated
//               with the mesh.
////////////////////////////////////////////////////////////////////
XFileMaterial *XFileMesh::
get_material(int n) const {
  nassertr(n >= 0 && n < (int)_materials.size(), (XFileMaterial *)NULL);
  return _materials[n];
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::make_x_mesh
//       Access: Public
//  Description: Creates an X structure corresponding to the mesh.
////////////////////////////////////////////////////////////////////
XFileDataNode *XFileMesh::
make_x_mesh(XFileNode *x_parent, const string &suffix) {
  XFileDataNode *x_mesh = x_parent->add_Mesh("mesh" + suffix);

  // First, fill in the table of vertices.
  XFileDataObject &x_vertices = (*x_mesh)["vertices"];

  Vertices::const_iterator vi;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    XFileVertex *vertex = (*vi);
    x_vertices.add_Vector(x_mesh->get_x_file(), LCAST(double, vertex->_point));
  }
  (*x_mesh)["nVertices"] = x_vertices.size();

  // Then, create the list of faces that index into the above vertices.
  XFileDataObject &x_faces = (*x_mesh)["faces"];
  Faces::const_iterator fi;
  for (fi = _faces.begin(); fi != _faces.end(); ++fi) {
    XFileFace *face = (*fi);

    XFileDataObject &x_mesh_face = x_faces.add_MeshFace(x_mesh->get_x_file());
    XFileDataObject &x_faceVertexIndices = x_mesh_face["faceVertexIndices"];
    XFileFace::Vertices::const_iterator fvi;
    for (fvi = face->_vertices.begin();
         fvi != face->_vertices.end();
         ++fvi) {
      x_faceVertexIndices.add_int((*fvi)._vertex_index);
    }
    x_mesh_face["nFaceVertexIndices"] = x_faceVertexIndices.size();
  }
  (*x_mesh)["nFaces"] = x_faces.size();

  // Now, add in any supplemental data.

  if (has_normals()) {
    // Tack on normals.
    make_x_normals(x_mesh, suffix);
  }
  if (has_colors()) {
    // Tack on colors.
    make_x_colors(x_mesh, suffix);
  }
  if (has_uvs()) {
    // Tack on uvs.
    make_x_uvs(x_mesh, suffix);
  }
  if (has_materials()) {
    // Tack on materials.
    make_x_material_list(x_mesh, suffix);
  }

  return x_mesh;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::make_x_normals
//       Access: Public
//  Description: Creates a MeshNormals table for the mesh.
////////////////////////////////////////////////////////////////////
XFileDataNode *XFileMesh::
make_x_normals(XFileNode *x_mesh, const string &suffix) {
  XFileDataNode *x_meshNormals = x_mesh->add_MeshNormals("norms" + suffix);

  XFileDataObject &x_normals = (*x_meshNormals)["normals"];

  Normals::const_iterator ni;
  for (ni = _normals.begin(); ni != _normals.end(); ++ni) {
    XFileNormal *normal = (*ni);
    x_normals.add_Vector(x_mesh->get_x_file(), LCAST(double, normal->_normal));
  }
  (*x_meshNormals)["nNormals"] = x_normals.size();

  // Then, create the list of faces that index into the above normals.
  XFileDataObject &x_faces = (*x_meshNormals)["faceNormals"];
  Faces::const_iterator fi;
  for (fi = _faces.begin(); fi != _faces.end(); ++fi) {
    XFileFace *face = (*fi);

    XFileDataObject &x_normals_face = x_faces.add_MeshFace(x_mesh->get_x_file());
    XFileDataObject &x_faceVertexIndices = x_normals_face["faceVertexIndices"];
    XFileFace::Vertices::const_iterator fvi;
    for (fvi = face->_vertices.begin();
         fvi != face->_vertices.end();
         ++fvi) {
      x_faceVertexIndices.add_int((*fvi)._normal_index);
    }
    x_normals_face["nFaceVertexIndices"] = x_faceVertexIndices.size();
  }
  (*x_meshNormals)["nFaceNormals"] = x_faces.size();

  return x_meshNormals;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::make_x_colors
//       Access: Public
//  Description: Creates a MeshVertexColors table for the mesh.
////////////////////////////////////////////////////////////////////
XFileDataNode *XFileMesh::
make_x_colors(XFileNode *x_mesh, const string &suffix) {
  XFileDataNode *x_meshColors = x_mesh->add_MeshVertexColors("colors" + suffix);

  XFileDataObject &x_colors = (*x_meshColors)["vertexColors"];

  Vertices::const_iterator vi;
  int i = 0;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    XFileVertex *vertex = (*vi);
    const Colorf &color = vertex->_color;
    x_colors.add_IndexedColor(x_mesh->get_x_file(), i, color);
    i++;
  }

  (*x_meshColors)["nVertexColors"] = x_colors.size();
  
  return x_meshColors;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::make_x_uvs
//       Access: Public
//  Description: Creates a MeshTextureCoords table for the mesh.
////////////////////////////////////////////////////////////////////
XFileDataNode *XFileMesh::
make_x_uvs(XFileNode *x_mesh, const string &suffix) {
  XFileDataNode *x_meshUvs = x_mesh->add_MeshTextureCoords("uvs" + suffix);

  XFileDataObject &x_uvs = (*x_meshUvs)["textureCoords"];
  
  Vertices::const_iterator vi;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    XFileVertex *vertex = (*vi);
    x_uvs.add_Coords2d(x_mesh->get_x_file(), LCAST(double, vertex->_uv));
  }

  (*x_meshUvs)["nTextureCoords"] = x_uvs.size();

  return x_meshUvs;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::make_x_material_list
//       Access: Public
//  Description: Creates a MeshMaterialList table for the mesh.
////////////////////////////////////////////////////////////////////
XFileDataNode *XFileMesh::
make_x_material_list(XFileNode *x_mesh, const string &suffix) {
  XFileDataNode *x_meshMaterials = 
    x_mesh->add_MeshMaterialList("materials" + suffix);

  // First, build up the list of faces the reference the materials.
  XFileDataObject &x_indexes = (*x_meshMaterials)["faceIndexes"];

  Faces::const_iterator fi;
  for (fi = _faces.begin(); fi != _faces.end(); ++fi) {
    XFileFace *face = (*fi);
    x_indexes.add_int(face->_material_index);
  }

  (*x_meshMaterials)["nFaceIndexes"] = x_indexes.size();

  // Now, build up the list of materials themselves.  Each material is
  // a child of the MeshMaterialList node, rather than an element of
  // an array.
  for (size_t i = 0; i < _materials.size(); i++) {
    XFileMaterial *material = _materials[i];

    material->make_x_material(x_meshMaterials,
                              suffix + "_" + format_string(i));
  }

  (*x_meshMaterials)["nMaterials"] = (int)_materials.size();

  return x_meshMaterials;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::read_mesh_data
//       Access: Public
//  Description: Fills the structure based on the raw data from the
//               Mesh template.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
read_mesh_data(const Datagram &raw_data) {
  DatagramIterator di(raw_data);

  clear();

  int i, j;
  int num_vertices = di.get_int32();
  for (i = 0; i < num_vertices; i++) {
    XFileVertex *vertex = new XFileVertex;
    vertex->_point[0] = di.get_float32();
    vertex->_point[1] = di.get_float32();
    vertex->_point[2] = di.get_float32();
    add_vertex(vertex);
  }

  int num_faces = di.get_int32();
  for (i = 0; i < num_faces; i++) {
    XFileFace *face = new XFileFace;

    num_vertices = di.get_int32();
    for (j = 0; j < num_vertices; j++) {
      XFileFace::Vertex vertex;
      vertex._vertex_index = di.get_int32();
      vertex._normal_index = -1;

      face->_vertices.push_back(vertex);
    }
    _faces.push_back(face);
  }

  if (di.get_remaining_size() != 0) {
    xfile_cat.warning()
      << "Ignoring " << di.get_remaining_size() << " trailing Mesh.\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::read_normal_data
//       Access: Public
//  Description: Fills the structure based on the raw data from the
//               MeshNormals template.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
read_normal_data(const Datagram &raw_data) {
  DatagramIterator di(raw_data);

  int num_normals = di.get_int32();
  int i;
  for (i = 0; i < num_normals; i++) {
    XFileNormal *normal = new XFileNormal;
    normal->_normal[0] = di.get_float32();
    normal->_normal[1] = di.get_float32();
    normal->_normal[2] = di.get_float32();
    normal->_has_normal = true;
    add_normal(normal);
  }

  int num_faces = di.get_int32();

  if (num_faces != _faces.size()) {
    xfile_cat.error()
      << "Incorrect number of faces in MeshNormals.\n";
    return false;
  }

  for (i = 0; i < num_faces; i++) {
    XFileFace *face = _faces[i];
    int num_vertices = di.get_int32();
    if (num_vertices != face->_vertices.size()) {
      xfile_cat.error() 
        << "Incorrect number of vertices for face in MeshNormals.\n";
      return false;
    }
    for (int j = 0; j < num_vertices; j++) {
      face->_vertices[j]._normal_index = di.get_int32();
    }
  }

  if (di.get_remaining_size() != 0) {
    xfile_cat.warning()
      << "Ignoring " << di.get_remaining_size()
      << " trailing MeshNormals.\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::read_color_data
//       Access: Public
//  Description: Fills the structure based on the raw data from the
//               MeshVertexColors template.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
read_color_data(const Datagram &raw_data) {
  DatagramIterator di(raw_data);

  int num_colors = di.get_int32();
  int i;
  for (i = 0; i < num_colors; i++) {
    unsigned int vertex_index = di.get_int32();
    if (vertex_index < 0 || vertex_index >= _vertices.size()) {
      xfile_cat.error()
        << "Vertex index out of range in MeshVertexColors.\n";
      return false;
    }
    XFileVertex *vertex = _vertices[vertex_index];
    vertex->_color[0] = di.get_float32();
    vertex->_color[1] = di.get_float32();
    vertex->_color[2] = di.get_float32();
    vertex->_color[3] = di.get_float32();
    vertex->_has_color = true;
  }

  if (di.get_remaining_size() != 0) {
    xfile_cat.warning()
      << "Ignoring " << di.get_remaining_size()
      << " trailing MeshVertexColors.\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::read_uv_data
//       Access: Public
//  Description: Fills the structure based on the raw data from the
//               MeshTextureCoords template.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
read_uv_data(const Datagram &raw_data) {
  DatagramIterator di(raw_data);

  int num_vertices = di.get_int32();
  if (num_vertices != _vertices.size()) {
    xfile_cat.error()
      << "Wrong number of vertices in MeshTextureCoords.\n";
    return false;
  }

  int i;
  for (i = 0; i < num_vertices; i++) {
    XFileVertex *vertex = _vertices[i];
    vertex->_uv[0] = di.get_float32();
    vertex->_uv[1] = di.get_float32();
    vertex->_has_uv = true;
  }

  if (di.get_remaining_size() != 0) {
    xfile_cat.warning()
      << "Ignoring " << di.get_remaining_size()
      << " trailing MeshTextureCoords.\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::read_skin_weights_data
//       Access: Public
//  Description: Fills the structure based on the raw data from the
//               SkinWeights template.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
read_skin_weights_data(const Datagram &raw_data) {
  DatagramIterator di(raw_data);

  // Create a new SkinWeightsData record for the table.  We'll need
  // this data later when we create the vertices.
  _skin_weights.push_back(SkinWeightsData());
  SkinWeightsData &data = _skin_weights.back();

  // The DX system encodes a pointer to a character string in four
  // bytes within the stream.  Weird, in a Microsofty sort of way.
  data._joint_name = (const char *)di.get_uint32();

  int num_weights = di.get_int32();

  vector_int vindices;
  vindices.reserve(num_weights);

  // Unpack the list of vertices first
  int i;
  for (i = 0; i < num_weights; i++) {
    int vindex = di.get_int32();
    if (vindex < 0 || vindex > (int)_vertices.size()) {
      xfile_cat.error()
        << "Illegal vertex index " << vindex << " in SkinWeights.\n";
      return false;
    }
    vindices.push_back(vindex);
  }

  // Then unpack the weight for each vertex.
  for (i = 0; i < num_weights; i++) {
    float weight = di.get_float32();
    data._weight_map[vindices[i]] = weight;
  }

  // Finally, read the matrix offset.
  data._matrix_offset.read_datagram(di);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFileMesh::read_material_list_data
//       Access: Public
//  Description: Fills the structure based on the raw data from the
//               MaterialList template.
////////////////////////////////////////////////////////////////////
bool XFileMesh::
read_material_list_data(const Datagram &raw_data) {
  DatagramIterator di(raw_data);

  di.get_int32();  /* num_materials */
  unsigned int num_faces = di.get_int32();

  if (num_faces > _faces.size()) {
    xfile_cat.error()
      << "Too many faces in MaterialList.\n";
    return false;
  }

  int material_index = -1;
  unsigned int i = 0;
  while (i < num_faces) {
    XFileFace *face = _faces[i];
    material_index = di.get_int32();
    face->_material_index = material_index;
    i++;
  }

  // The rest of the faces get the same material index as the last
  // one in the list.
  while (i < (int)_faces.size()) {
    XFileFace *face = _faces[i];
    face->_material_index = material_index;
    i++;
  }

  if (di.get_remaining_size() != 0) {
    xfile_cat.warning()
      << "Ignoring " << di.get_remaining_size()
      << " trailing MeshMaterialList.\n";
  }

  return true;
}
