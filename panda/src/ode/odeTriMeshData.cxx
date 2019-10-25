/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file odeTriMeshData.cxx
 * @author joswilso
 * @date 2006-12-27
 */

#include "odeTriMeshData.h"

using std::ostream;

TypeHandle OdeTriMeshData::_type_handle;
OdeTriMeshData::TriMeshDataMap *OdeTriMeshData::_tri_mesh_data_map = nullptr;

void OdeTriMeshData::
link_data(dGeomID id, PT(OdeTriMeshData) data) {
  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug() << get_class_type() << "::link_data(" << id << "->" << data << ")" << "\n";
  }
  get_tri_mesh_data_map()[id] = data;
}

PT(OdeTriMeshData) OdeTriMeshData::
get_data(dGeomID id) {
  const TriMeshDataMap &data_map = get_tri_mesh_data_map();
  TriMeshDataMap::const_iterator iter = data_map.find(id);
  if (iter != data_map.end()) {
    return iter->second;
  }
  return nullptr;
}

void OdeTriMeshData::
unlink_data(dGeomID id) {
  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug() << get_class_type() << "::unlink_data(" << id << ")" << "\n";
  }
  nassertv(_tri_mesh_data_map != nullptr);
  TriMeshDataMap::iterator iter = _tri_mesh_data_map->find(id);
  if (iter != _tri_mesh_data_map->end()) {
    _tri_mesh_data_map->erase(iter);
  }
}

void OdeTriMeshData::
print_data(const std::string &marker) {
  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug() << get_class_type() << "::print_data(" << marker << ")\n";
    const TriMeshDataMap &data_map = get_tri_mesh_data_map();
    TriMeshDataMap::const_iterator iter = data_map.begin();
    for (;iter != data_map.end(); ++iter) {
      odetrimeshdata_cat.debug() << "\t" << iter->first << " : " << iter->second << "\n";
    }
  }
}

void OdeTriMeshData::
remove_data(OdeTriMeshData *data) {
  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug()
      << get_class_type() << "::remove_data(" << data->get_id() << ")" << "\n";
  }
  if (_tri_mesh_data_map == nullptr) {
    return;
  }

  TriMeshDataMap::iterator iter;
  for (iter = _tri_mesh_data_map->begin();
       iter != _tri_mesh_data_map->end();
       ++iter) {
    if (iter->second == data) {
      break;
    }
  }

  while (iter != _tri_mesh_data_map->end()) {
    _tri_mesh_data_map->erase(iter);

    for (iter = _tri_mesh_data_map->begin();
         iter != _tri_mesh_data_map->end();
         ++iter) {
      if (iter->second == data) {
        break;
      }
    }
  }
}


OdeTriMeshData::
OdeTriMeshData(const NodePath& model, bool use_normals) :
  _id(dGeomTriMeshDataCreate()),
  _vertices(nullptr),
  _faces(nullptr),
  _normals(nullptr),
  _num_vertices(0),
  _num_faces(0) {
  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug() << get_type() << "(" << _id << ")" << "\n";
  }

  process_model(model, use_normals);

  if (odetrimeshdata_cat.is_debug()) {
    write_faces(odetrimeshdata_cat.debug());
  }

#ifdef dSINGLE
  if (!use_normals) {
    build_single(_vertices, sizeof(StridedVertex), _num_vertices,
                 _faces, _num_faces * 3, sizeof(StridedTri));
  } else {
    build_single1(_vertices, sizeof(StridedVertex), _num_vertices,
                  _faces, _num_faces * 3, sizeof(StridedTri),
                  _normals);
  }
#else
  if (!use_normals) {
    build_double(_vertices, sizeof(StridedVertex), _num_vertices,
                 _faces, _num_faces * 3, sizeof(StridedTri));
  } else {
    build_double1(_vertices, sizeof(StridedVertex), _num_vertices,
                  _faces, _num_faces * 3, sizeof(StridedTri),
                  _normals);
  }
#endif

  preprocess();
}

// Private copy constructor, shouldn't be copying these objects
OdeTriMeshData::
OdeTriMeshData(const OdeTriMeshData &other) {
}

OdeTriMeshData::
~OdeTriMeshData() {
  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug() << "~" << get_type() << "(" << _id << ")" << "\n";
  }
  destroy();
  if (_vertices != nullptr) {
    PANDA_FREE_ARRAY(_vertices);
    _vertices = nullptr;
    _num_vertices = 0;
  }
  if (_faces != nullptr) {
    PANDA_FREE_ARRAY(_faces);
    _faces = nullptr;
  }
  if (_normals != nullptr) {
    // This is never allocated?  Until we use _normals, assert that we don't
    // accidentally free it here through some mistake.
    nassertv(false);
    PANDA_FREE_ARRAY(_normals);
  }
}

void OdeTriMeshData::
destroy() {
  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug() << get_type() << "::destroy(" << _id << ")" << "\n";
  }
  if (_id != nullptr) {
    dGeomTriMeshDataDestroy(_id);
    remove_data(this);
    _id = nullptr;
  }
}

// Private assignment operator, shouldn't be copying these objects
void OdeTriMeshData::
operator = (const OdeTriMeshData &other) {
}

void OdeTriMeshData::
process_model(const NodePath& model, bool &use_normals) {
  // TODO: assert if _vertices is something other than 0.
  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug()
      << "process_model(" << model << ")" << "\n";
  }
  if (model.is_empty()) {
    return;
  }

  NodePathCollection geomNodePaths = model.find_all_matches("**/+GeomNode");
  if (model.node()->get_type() == GeomNode::get_class_type()) {
    geomNodePaths.add_path(model);
  }

  for (int i = 0; i < geomNodePaths.get_num_paths(); ++i) {
    analyze((GeomNode*)geomNodePaths[i].node());
  }

  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug() << "Found " << _num_vertices << " vertices.\n";
    odetrimeshdata_cat.debug() << "Found " << _num_faces << " faces.\n";
  }

  _vertices = (StridedVertex *)PANDA_MALLOC_ARRAY(_num_vertices * sizeof(StridedVertex));
  _faces = (StridedTri *)PANDA_MALLOC_ARRAY(_num_faces * sizeof(StridedTri));

  _num_vertices = 0, _num_faces = 0;

  for (int i = 0; i < geomNodePaths.get_num_paths(); ++i) {
    process_geom_node((GeomNode*)geomNodePaths[i].node());
    if (odetrimeshdata_cat.is_debug()) {
      odetrimeshdata_cat.debug() << "_num_vertices now at " << _num_vertices << "\n";
    }
  }

  if (odetrimeshdata_cat.is_debug()) {
    odetrimeshdata_cat.debug()
      << "Filled " << _num_faces << " triangles(" << _num_vertices << " vertices)\n";
  }
}

void OdeTriMeshData::
process_geom_node(const GeomNode *geomNode) {
  if (odetrimeshdata_cat.is_debug()) {
    ostream &out = odetrimeshdata_cat.debug();
    out.width(2); out << "" << "process_geom_node(" << *geomNode << ")" << "\n";
  }
  for (int i = 0; i < geomNode->get_num_geoms(); ++i) {
    process_geom(geomNode->get_geom(i));
  }
}

void OdeTriMeshData::
process_geom(const Geom *geom) {
  if (odetrimeshdata_cat.is_debug()) {
    ostream &out = odetrimeshdata_cat.debug();
    out.width(4); out << "" << "process_geom(" << *geom << ")" << "\n";
  }
  if (geom->get_primitive_type() != Geom::PT_polygons) {
    return;
  }

  CPT(GeomVertexData) vData = geom->get_vertex_data();

  for (size_t i = 0; i < geom->get_num_primitives(); ++i) {
    process_primitive(geom->get_primitive(i), vData);
  }
}

void OdeTriMeshData::
process_primitive(const GeomPrimitive *primitive,
                   CPT(GeomVertexData) vData) {
  GeomVertexReader vReader(vData, "vertex");
  GeomVertexReader nReader(vData, "normal");
  LVecBase3f vertex;
  // CPT(GeomPrimitive) dPrimitive = primitive->decompose();
  CPT(GeomPrimitive) dPrimitive = primitive;
  ostream &out = odetrimeshdata_cat.debug();
  out.width(6); out << "" << "process_primitive(" << *dPrimitive << ")" << "\n";


  if (dPrimitive->get_type() == GeomTriangles::get_class_type()) {
    for (int i = 0; i < dPrimitive->get_num_primitives(); i++, _num_faces++) {
      int s = dPrimitive->get_primitive_start(i);
      int e = dPrimitive->get_primitive_end(i);
      out.width(8); out << "" << "primitive " << i << ":" << "\n";
      for (int j = s, m = 0; j < e; j++, m++, _num_vertices++) {
        int vRowIndex = dPrimitive->get_vertex(j);
        vReader.set_row_unsafe(vRowIndex);
        nReader.set_row_unsafe(vRowIndex);
        vertex = vReader.get_data3f();
        // normal = nReader.get_data3f();
        _faces[_num_faces].Indices[m] = _num_vertices;

        _vertices[_num_vertices].Vertex[0] = vertex[0];
        _vertices[_num_vertices].Vertex[1] = vertex[1];
        _vertices[_num_vertices].Vertex[2] = vertex[2];

        out.width(10); out << "" << "vertex " << j << " has: pos(" \
                           << vertex << ") normal(" << "normal"  << ")" << "\n";
      }
    }
  } else if (dPrimitive->get_type() == GeomTristrips::get_class_type()){
    for (int i = 0; i < dPrimitive->get_num_primitives(); i++, _num_faces++) {
      int s = dPrimitive->get_primitive_start(i);
      int e = dPrimitive->get_primitive_end(i);
      out.width(8); out << "" << "primitive " << i << ":" << "\n";
      for (int j = s, m = 0; j < e; j++, m++, _num_vertices++) {
        int vRowIndex = dPrimitive->get_vertex(j);
        vReader.set_row_unsafe(vRowIndex);
        nReader.set_row_unsafe(vRowIndex);
        vertex = vReader.get_data3f();
        // normal = nReader.get_data3f();

        _vertices[_num_vertices].Vertex[0] = vertex[0];
        _vertices[_num_vertices].Vertex[1] = vertex[1];
        _vertices[_num_vertices].Vertex[2] = vertex[2];
        out.width(10); out << "" << "vertex " << j << " has: pos(" \
                           << vertex << ") normal(" << "normal" << ")" << "\n";
        if (m < 3) {
          _faces[_num_faces].Indices[m] = _num_vertices;
        } else {
          _num_faces++;
          if ( m & 1) {
            _faces[_num_faces].Indices[0] = _num_vertices-1;
            _faces[_num_faces].Indices[1] = _num_vertices-2;
            _faces[_num_faces].Indices[2] = _num_vertices;
          } else {
            _faces[_num_faces].Indices[0] = _num_vertices-2;
            _faces[_num_faces].Indices[1] = _num_vertices-1;
            _faces[_num_faces].Indices[2] = _num_vertices;
          }
        }
      }
      out << "\n";
    }
  }
}
void OdeTriMeshData::
analyze(const GeomNode *geomNode) {
  for (int i = 0; i < geomNode->get_num_geoms(); ++i) {
    analyze(geomNode->get_geom(i));
  }
}

void OdeTriMeshData::
analyze(const Geom *geom) {
  if (geom->get_primitive_type() != Geom::PT_polygons) {
    return;
  }

  for (size_t i = 0; i < geom->get_num_primitives(); ++i) {
    analyze(geom->get_primitive(i));
  }
}

void OdeTriMeshData::
analyze(const GeomPrimitive *primitive) {
  for (int i = 0; i < primitive->get_num_primitives(); ++i) {
    _num_vertices += primitive->get_primitive_num_vertices(i);
    _num_faces += primitive->get_primitive_num_faces(i);
  }
}

void OdeTriMeshData::
write_faces(ostream &out) const {
  out<<"\n";
  for (unsigned int i = 0; i < _num_faces; ++i) {
    out.width(2); out << "Face " << i << ":\n";
    for (int j = 0; j < 3; ++j) {
      out.width(4);
      out << "(" << _vertices[_faces[i].Indices[j]].Vertex[0] \
          << ", " << _vertices[_faces[i].Indices[j]].Vertex[1] \
          << ", " << _vertices[_faces[i].Indices[j]].Vertex[2] << ")\n" ;
    }
  }
}

void OdeTriMeshData::
write(ostream &out, unsigned int indent) const {
  out.width(indent); out << "" << get_type() << "(id = " << _id << ") : " \
                         << "" << "Vertices: " << (_id ? _num_vertices : 0) << ", " \
                         << "" << "Triangles: " << (_id ? _num_faces : 0);
}
