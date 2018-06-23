/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file objToEggConverter.cxx
 * @author drose
 * @date 2010-12-07
 */

#include "objToEggConverter.h"
#include "config_objegg.h"
#include "eggData.h"
#include "string_utils.h"
#include "streamReader.h"
#include "virtualFileSystem.h"
#include "eggPolygon.h"
#include "nodePath.h"
#include "geomTriangles.h"
#include "geomPoints.h"
#include "colorAttrib.h"
#include "shadeModelAttrib.h"
#include "dcast.h"
#include "triangulator3.h"
#include "config_egg2pg.h"

using std::string;

/**
 *
 */
ObjToEggConverter::
ObjToEggConverter() {
}

/**
 *
 */
ObjToEggConverter::
ObjToEggConverter(const ObjToEggConverter &copy) :
  SomethingToEggConverter(copy)
{
}

/**
 *
 */
ObjToEggConverter::
~ObjToEggConverter() {
}

/**
 * Allocates and returns a new copy of the converter.
 */
SomethingToEggConverter *ObjToEggConverter::
make_copy() {
  return new ObjToEggConverter(*this);
}


/**
 * Returns the English name of the file type this converter supports.
 */
string ObjToEggConverter::
get_name() const {
  return "obj";
}

/**
 * Returns the common extension of the file type this converter supports.
 */
string ObjToEggConverter::
get_extension() const {
  return "obj";
}

/**
 * Returns true if this file type can transparently load compressed files
 * (with a .pz extension), false otherwise.
 */
bool ObjToEggConverter::
supports_compressed() const {
  return true;
}

/**
 * Returns true if this converter can directly convert the model type to
 * internal Panda memory structures, given the indicated options, or false
 * otherwise.  If this returns true, then convert_to_node() may be called to
 * perform the conversion, which may be faster than calling convert_file() if
 * the ultimate goal is a PandaNode anyway.
 */
bool ObjToEggConverter::
supports_convert_to_node(const LoaderOptions &options) const {
  return true;
}

/**
 * Handles the reading of the input file and converting it to egg.  Returns
 * true if successful, false otherwise.
 */
bool ObjToEggConverter::
convert_file(const Filename &filename) {
  clear_error();

  if (_egg_data->get_coordinate_system() == CS_default) {
    _egg_data->set_coordinate_system(CS_zup_right);
  }

  if (!process(filename)) {
    _error = true;
  }
  return !had_error();
}

/**
 * Reads the input file and directly produces a ready-to-render model file as
 * a PandaNode.  Returns NULL on failure, or if it is not supported.  (This
 * functionality is not supported by all converter types; see
 * supports_convert_to_node()).
 */
PT(PandaNode) ObjToEggConverter::
convert_to_node(const LoaderOptions &options, const Filename &filename) {
  clear_error();

  _root_node = new PandaNode("");
  _current_vertex_data = new VertexData(_root_node, "root");

  if (!process_node(filename)) {
    _error = true;
  }

  _current_vertex_data->close_geom(this);
  delete _current_vertex_data;

  if (had_error()) {
    return nullptr;
  }

  return _root_node;
}

/**
 * Reads the file and converts it to egg structures.
 */
bool ObjToEggConverter::
process(const Filename &filename) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::istream *strm = vfs->open_read_file(filename, true);
  if (strm == nullptr) {
    objegg_cat.error()
      << "Couldn't read " << filename << "\n";
    return false;
  }

  _v_table.clear();
  _vn_table.clear();
  _rgb_table.clear();
  _vt_table.clear();
  _xvt_table.clear();
  _ref_plane_res.set(1.0, 1.0);
  _v4_given = false;
  _vt3_given = false;
  _f_given = false;

  _vpool = new EggVertexPool("vpool");
  _egg_data->add_child(_vpool);
  _root_group = new EggGroup("root");
  _egg_data->add_child(_root_group);
  _current_group = _root_group;

  StreamReader sr(strm, true);
  string line = sr.readline();
  _line_number = 1;
  while (!line.empty()) {
    line = trim(line);
    if (line.empty()) {
      line = sr.readline();
      continue;
    }

    while (line[line.length() - 1] == '\\') {
      // If it ends on a backslash, it's a continuation character.
      string line2 = sr.readline();
      ++_line_number;
      if (line2.empty()) {
        break;
      }
      line = line.substr(0, line.length() - 1) + trim(line2);
    }

    if (line.substr(0, 15) == "#_ref_plane_res") {
      process_ref_plane_res(line);
      line = sr.readline();
      continue;
    }

    if (line[0] == '#') {
      line = sr.readline();
      continue;
    }

    if (!process_line(line)) {
      return false;
    }
    line = sr.readline();
    ++_line_number;
  }

  if (!_f_given) {
    generate_egg_points();
  }

  return true;
}

/**
 *
 */
bool ObjToEggConverter::
process_line(const string &line) {
  vector_string words;
  tokenize(line, words, " \t", true);
  nassertr(!words.empty(), false);

  string tag = words[0];
  if (tag == "v") {
    return process_v(words);
  } else if (tag == "vt") {
    return process_vt(words);
  } else if (tag == "xvt") {
    return process_xvt(words);
  } else if (tag == "xvc") {
    return process_xvc(words);
  } else if (tag == "vn") {
    return process_vn(words);
  } else if (tag == "f") {
    return process_f(words);
  } else if (tag == "g") {
    return process_g(words);
  } else {
    bool inserted = _ignored_tags.insert(tag).second;
    if (inserted) {
      objegg_cat.info()
        << "Ignoring tag " << tag << "\n";
    }
  }

  return true;
}

/**
 *
 */
bool ObjToEggConverter::
process_ref_plane_res(const string &line) {
  // the #_ref_plane_res line is a DRZ extension that defines the pixel
  // resolution of the projector device.  It's needed to properly scale the
  // xvt lines.

  vector_string words;
  tokenize(line, words, " \t", true);
  nassertr(!words.empty(), false);

  if (words.size() != 3) {
    objegg_cat.error()
      << "Wrong number of tokens at line " << _line_number << "\n";
    return false;
  }

  bool okflag = true;
  okflag &= string_to_double(words[1], _ref_plane_res[0]);
  okflag &= string_to_double(words[2], _ref_plane_res[1]);

  if (!okflag) {
    objegg_cat.error()
      << "Invalid number at line " << _line_number << ":\n";
    return false;
  }

  return true;
}

/**
 *
 */
bool ObjToEggConverter::
process_v(vector_string &words) {
  if (words.size() != 4 && words.size() != 5 &&
      words.size() != 7 && words.size() != 8) {
    objegg_cat.error()
      << "Wrong number of tokens at line " << _line_number << "\n";
    return false;
  }

  bool okflag = true;
  LPoint4d pos;
  okflag &= string_to_double(words[1], pos[0]);
  okflag &= string_to_double(words[2], pos[1]);
  okflag &= string_to_double(words[3], pos[2]);
  if (words.size() == 5 || words.size() == 8) {
    okflag &= string_to_double(words[4], pos[3]);
    _v4_given = true;
  } else {
    pos[3] = 1.0;
  }

  if (!okflag) {
    objegg_cat.error()
      << "Invalid number at line " << _line_number << "\n";
    return false;
  }

  _v_table.push_back(pos);

  // Meshlab format might include an RGB color following the vertex position.
  if (words.size() == 7 && words.size() == 8) {
    size_t si = words.size();
    LVecBase3d rgb;
    okflag &= string_to_double(words[si - 3], rgb[0]);
    okflag &= string_to_double(words[si - 2], rgb[1]);
    okflag &= string_to_double(words[si - 1], rgb[2]);

    if (!okflag) {
      objegg_cat.error()
        << "Invalid number at line " << _line_number << "\n";
      return false;
    }
    while (_rgb_table.size() + 1 < _v_table.size()) {
      _rgb_table.push_back(LVecBase3d(1.0, 1.0, 1.0));
    }
    _rgb_table.push_back(rgb);
  }

  return true;
}

/**
 *
 */
bool ObjToEggConverter::
process_vt(vector_string &words) {
  if (words.size() != 3 && words.size() != 4) {
    objegg_cat.error()
      << "Wrong number of tokens at line " << _line_number << "\n";
    return false;
  }

  bool okflag = true;
  LTexCoord3d uvw;
  okflag &= string_to_double(words[1], uvw[0]);
  okflag &= string_to_double(words[2], uvw[1]);
  if (words.size() == 4) {
    okflag &= string_to_double(words[3], uvw[2]);
    _vt3_given = true;
  } else {
    uvw[2] = 0.0;
  }

  if (!okflag) {
    objegg_cat.error()
      << "Invalid number at line " << _line_number << "\n";
    return false;
  }

  _vt_table.push_back(uvw);

  return true;
}

/**
 * "xvt" is an extended column invented by DRZ.  It includes texture
 * coordinates in pixel space of the projector device, as well as for each
 * camera.  We map it to the nominal texture coordinates here.
 */
bool ObjToEggConverter::
process_xvt(vector_string &words) {
  if (words.size() < 3) {
    objegg_cat.error()
      << "Wrong number of tokens at line " << _line_number << "\n";
    return false;
  }

  bool okflag = true;
  LTexCoordd uv;
  okflag &= string_to_double(words[1], uv[0]);
  okflag &= string_to_double(words[2], uv[1]);

  if (!okflag) {
    objegg_cat.error()
      << "Invalid number at line " << _line_number << "\n";
    return false;
  }

  uv[0] /= _ref_plane_res[0];
  uv[1] = 1.0 - uv[1] / _ref_plane_res[1];

  _xvt_table.push_back(uv);

  return true;
}

/**
 * "xvc" is another extended column invented by DRZ.  We quietly ignore it.
 */
bool ObjToEggConverter::
process_xvc(vector_string &words) {
  return true;
}

/**
 *
 */
bool ObjToEggConverter::
process_vn(vector_string &words) {
  if (words.size() != 4) {
    objegg_cat.error()
      << "Wrong number of tokens at line " << _line_number << "\n";
    return false;
  }

  bool okflag = true;
  LVector3d normal;
  okflag &= string_to_double(words[1], normal[0]);
  okflag &= string_to_double(words[2], normal[1]);
  okflag &= string_to_double(words[3], normal[2]);

  if (!okflag) {
    objegg_cat.error()
      << "Invalid number at line " << _line_number << "\n";
    return false;
  }
  normal.normalize();

  _vn_table.push_back(normal);

  return true;
}

/**
 * Defines a face in the obj file.
 */
bool ObjToEggConverter::
process_f(vector_string &words) {
  _f_given = true;

  PT(EggPolygon) poly = new EggPolygon;
  for (size_t i = 1; i < words.size(); ++i) {
    EggVertex *vertex = get_face_vertex(words[i]);
    if (vertex == nullptr) {
      return false;
    }
    poly->add_vertex(vertex);
  }
  _current_group->add_child(poly);

  return true;
}

/**
 * Defines a group in the obj file.
 */
bool ObjToEggConverter::
process_g(vector_string &words) {
  EggGroup *group = _root_group;

  // We assume the group names define a hierarchy of more-specific to less-
  // specific group names, so that the first group name is the bottommost
  // node, and the last group name is the topmost node.

  // Thus, iterate from the back to the front.
  size_t i = words.size();
  while (i > 1) {
    --i;
    EggNode *child = group->find_child(words[i]);
    if (child == nullptr || !child->is_of_type(EggGroup::get_class_type())) {
      child = new EggGroup(words[i]);
      group->add_child(child);
    }
    group = DCAST(EggGroup, child);
  }

  _current_group = group;
  return true;
}

/**
 * Returns or creates a vertex in the vpool according to the indicated face
 * reference.
 */
EggVertex *ObjToEggConverter::
get_face_vertex(const string &reference) {
  VertexEntry entry(this, reference);

  // Synthesize a vertex.
  EggVertex synth;

  if (entry._vi != 0) {
    if (_v4_given) {
      synth.set_pos(LCAST(double, _v_table[entry._vi - 1]));
    } else {
      LPoint4d pos = _v_table[entry._vi - 1];
      synth.set_pos(LPoint3d(pos[0], pos[1], pos[2]));
    }

    if (entry._vi - 1 < (int)_rgb_table.size()) {
      // We have a per-vertex color too.
      LRGBColord rgb = _rgb_table[entry._vi - 1];
      LColor rgba(rgb[0], rgb[1], rgb[2], 1.0);
      synth.set_color(rgba);
    }
  }

  if (entry._vti != 0) {
    // We have a texture coordinate; apply it.
    if (_vt3_given) {
      synth.set_uvw("", _vt_table[entry._vti - 1]);
    } else {
      LTexCoord3d uvw = _vt_table[entry._vti - 1];
      synth.set_uv("", LTexCoordd(uvw[0], uvw[1]));
    }
  } else if (entry._vi - 1 < (int)_xvt_table.size()) {
    // We have an xvt texture coordinate.
    synth.set_uv("", _xvt_table[entry._vi - 1]);
  }

  if (entry._vni != 0) {
    // We have a normal; apply it.
    synth.set_normal(_vn_table[entry._vni - 1]);
  }

  return _vpool->create_unique_vertex(synth);
}

/**
 * If an obj file defines no faces, create a bunch of EggVertex objects to
 * illustrate the vertex positions at least.
 */
void ObjToEggConverter::
generate_egg_points() {
  for (size_t vi = 0; vi < _v_table.size(); ++vi) {
    const LVecBase4d &p = _v_table[vi];
    _vpool->make_new_vertex(LVecBase3d(p[0], p[1], p[2]));
  }
}


/**
 * Reads the file and converts it to PandaNode structures.
 */
bool ObjToEggConverter::
process_node(const Filename &filename) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::istream *strm = vfs->open_read_file(filename, true);
  if (strm == nullptr) {
    objegg_cat.error()
      << "Couldn't read " << filename << "\n";
    return false;
  }

  _v_table.clear();
  _vn_table.clear();
  _rgb_table.clear();
  _vt_table.clear();
  _xvt_table.clear();
  _ref_plane_res.set(1.0, 1.0);
  _v4_given = false;
  _vt3_given = false;
  _f_given = false;

  StreamReader sr(strm, true);
  string line = sr.readline();
  _line_number = 1;
  while (!line.empty()) {
    line = trim(line);
    if (line.empty()) {
      line = sr.readline();
      continue;
    }

    if (line.substr(0, 15) == "#_ref_plane_res") {
      process_ref_plane_res(line);
      line = sr.readline();
      continue;
    }

    if (line[0] == '#') {
      line = sr.readline();
      continue;
    }

    if (!process_line_node(line)) {
      return false;
    }
    line = sr.readline();
    ++_line_number;
  }

  if (!_f_given) {
    generate_points();
  }

  return true;
}

/**
 *
 */
bool ObjToEggConverter::
process_line_node(const string &line) {
  vector_string words;
  tokenize(line, words, " \t", true);
  nassertr(!words.empty(), false);

  string tag = words[0];
  if (tag == "v") {
    return process_v(words);
  } else if (tag == "vt") {
    return process_vt(words);
  } else if (tag == "xvt") {
    return process_xvt(words);
  } else if (tag == "xvc") {
    return process_xvc(words);
  } else if (tag == "vn") {
    return process_vn(words);
  } else if (tag == "f") {
    return process_f_node(words);
  } else if (tag == "g") {
    return process_g_node(words);
  } else {
    bool inserted = _ignored_tags.insert(tag).second;
    if (inserted) {
      objegg_cat.info()
        << "Ignoring tag " << tag << "\n";
    }
  }

  return true;
}

/**
 * Defines a face in the obj file.
 */
bool ObjToEggConverter::
process_f_node(vector_string &words) {
  _f_given = true;

  bool all_vn = true;
  //int non_vn_index = -1;

  pvector<VertexEntry> verts;
  verts.reserve(words.size() - 1);
  for (size_t i = 1; i < words.size(); ++i) {
    VertexEntry entry(this, words[i]);
    verts.push_back(entry);
    if (entry._vni == 0) {
      all_vn = false;
      //non_vn_index = i;
    }
  }

  if (verts.size() < 3) {
    // Not enough vertices.
    objegg_cat.error()
      << "Degenerate face at " << _line_number << "\n";
    return false;
  }

  int synth_vni = 0;
  if (!all_vn) {
    // Synthesize a normal if we need it.
    LNormald normal = LNormald::zero();
    for (size_t i = 0; i < verts.size(); ++i) {
      int vi0 = verts[i]._vi;
      int vi1 = verts[(i + 1) % verts.size()]._vi;
      if (vi0 == 0 || vi1 == 0) {
        continue;
      }
      const LVecBase4d &p0 = _v_table[vi0 - 1];
      const LVecBase4d &p1 = _v_table[vi1 - 1];

      normal[0] += p0[1] * p1[2] - p0[2] * p1[1];
      normal[1] += p0[2] * p1[0] - p0[0] * p1[2];
      normal[2] += p0[0] * p1[1] - p0[1] * p1[0];
    }
    normal.normalize();
    synth_vni = add_synth_normal(normal);
  }

  Triangulator3 tri;
  int num_tris = 1;

  if (verts.size() != 3) {
    // We have to triangulate a higher-order polygon.
    for (size_t i = 0; i < verts.size(); ++i) {
      const LVecBase4d &p = _v_table[verts[i]._vi - 1];
      tri.add_vertex(p[0], p[1], p[2]);
      tri.add_polygon_vertex(i);
    }

    tri.triangulate();
    num_tris = tri.get_num_triangles();
  }

  if (_current_vertex_data->_prim->get_num_vertices() + 3 * num_tris > egg_max_indices ||
      _current_vertex_data->_entries.size() + verts.size() > (size_t)egg_max_vertices) {
    // We'll exceed our specified limit with these triangles; start a new
    // Geom.
    _current_vertex_data->close_geom(this);
  }

  if (verts.size() == 3) {
    // It's already a triangle; add it directly.
    _current_vertex_data->add_triangle(this, verts[0], verts[1], verts[2], synth_vni);

  } else {
    // Get the triangulated results.
    for (int ti = 0; ti < num_tris; ++ti) {
      int i0 = tri.get_triangle_v0(ti);
      int i1 = tri.get_triangle_v1(ti);
      int i2 = tri.get_triangle_v2(ti);
      _current_vertex_data->add_triangle(this, verts[i0], verts[i1], verts[i2], synth_vni);
    }
  }

  return true;
}

/**
 * Defines a group in the obj file.
 */
bool ObjToEggConverter::
process_g_node(vector_string &words) {
  _current_vertex_data->close_geom(this);
  delete _current_vertex_data;
  _current_vertex_data = nullptr;

  NodePath np(_root_node);

  // We assume the group names define a hierarchy of more-specific to less-
  // specific group names, so that the first group name is the bottommost
  // node, and the last group name is the topmost node.

  // Thus, iterate from the back to the front.
  size_t i = words.size();
  string name;
  while (i > 2) {
    --i;
    name = words[i];
    NodePath child = np.find(name);
    if (!child) {
      child = np.attach_new_node(name);
    }
    np = child;
  }

  if (i > 1) {
    --i;
    name = words[i];
  }

  _current_vertex_data = new VertexData(np.node(), name);

  return true;
}

/**
 * If an obj file defines no faces, create a bunch of GeomPoints to illustrate
 * the vertex positions at least.
 */
void ObjToEggConverter::
generate_points() {
  CPT(GeomVertexFormat) format = GeomVertexFormat::get_v3();
  PT(GeomVertexData) vdata = new GeomVertexData("points", format, GeomEnums::UH_static);
  vdata->set_num_rows(_v_table.size());
  GeomVertexWriter vertex_writer(vdata, InternalName::get_vertex());

  for (size_t vi = 0; vi < _v_table.size(); ++vi) {
    const LVecBase4d &p = _v_table[vi];
    vertex_writer.add_data3d(p[0], p[1], p[2]);
  }

  PT(GeomPrimitive) prim = new GeomPoints(GeomEnums::UH_static);
  prim->add_next_vertices(_v_table.size());
  prim->close_primitive();

  PT(Geom) geom = new Geom(vdata);
  geom->add_primitive(prim);

  PT(GeomNode) geom_node = new GeomNode("points");
  geom_node->add_geom(geom);
  _root_node->add_child(geom_node);
}

/**
 * Adds a new normal to the synth_vn table, or returns an existing normal.  In
 * either case returns the 1-based index number to the normal.
 */
int ObjToEggConverter::
add_synth_normal(const LVecBase3d &normal) {
  std::pair<UniqueVec3Table::iterator, bool> result = _unique_synth_vn_table.insert(UniqueVec3Table::value_type(normal, _unique_synth_vn_table.size()));
  UniqueVec3Table::iterator ni = result.first;
  int index = (*ni).second;

  if (result.second) {
    // If the normal was added to the table, it's a unique normal, and now we
    // have to add it to the table too.
    _synth_vn_table.push_back(normal);
  }

  return index + 1;
}

/**
 * Creates a VertexEntry from the n/n/n string format in the obj file face
 * reference.
 */
ObjToEggConverter::VertexEntry::
VertexEntry(const ObjToEggConverter *converter, const string &obj_vertex) {
  _vi = 0;
  _vti = 0;
  _vni = 0;
  _synth_vni = 0;

  vector_string words;
  tokenize(obj_vertex, words, "/", false);
  nassertv(!words.empty());

  for (size_t i = 0; i < words.size(); ++i) {
    int index;
    if (trim(words[i]).empty()) {
      index = 0;
    } else {
      if (!string_to_int(words[i], index)) {
        index = 0;
      }
    }

    switch (i) {
    case 0:
      _vi = index;
      if (_vi < 0) {
        _vi = (int)converter->_v_table.size() + _vi;
      }
      if (_vi < 0 || _vi - 1 >= (int)converter->_v_table.size()) {
        _vi = 0;
      }
      break;

    case 1:
      _vti = index;
      if (_vti < 0) {
        _vti = (int)converter->_vt_table.size() + _vti;
      }
      if (_vti < 0 || _vti - 1 >= (int)converter->_vt_table.size()) {
        _vti = 0;
      }
      break;

    case 2:
      _vni = index;
      if (_vni < 0) {
        _vni = (int)converter->_vn_table.size() + _vni;
      }
      if (_vni < 0 || _vni - 1 >= (int)converter->_vn_table.size()) {
        _vni = 0;
      }
      break;
    }
  }
}

/**
 *
 */
ObjToEggConverter::VertexData::
VertexData(PandaNode *parent, const string &name) :
  _parent(parent), _name(name)
{
  _geom_node = nullptr;

  _v4_given = false;
  _vt3_given = false;
  _vt_given = false;
  _rgb_given = false;
  _vn_given = false;

  _prim = new GeomTriangles(GeomEnums::UH_static);
}

/**
 * Adds a new entry to the vertex data for the indicated VertexEntry, or
 * returns an equivalent vertex already present.
 */
int ObjToEggConverter::VertexData::
add_vertex(const ObjToEggConverter *converter, const VertexEntry &entry) {
  std::pair<UniqueVertexEntries::iterator, bool> result;
  UniqueVertexEntries::iterator ni;
  int index;

  if (entry._vni != 0 || entry._synth_vni != 0) {
    // If we are storing a vertex with a normal, see if we have already stored
    // a vertex without a normal first.
    VertexEntry no_normal(entry);
    no_normal._vni = 0;
    no_normal._synth_vni = 0;
    ni = _unique_entries.find(no_normal);
    if (ni != _unique_entries.end()) {
      // We did have such a vertex!  In this case, repurpose this vertex,
      // resetting it to contain this normal.
      index = (*ni).second;
      _unique_entries.erase(ni);
      result = _unique_entries.insert(UniqueVertexEntries::value_type(entry, index));
      nassertr(result.second, index);
      nassertr(_entries[index] == no_normal, index);
      _entries[index]._vni = entry._vni;
      _entries[index]._synth_vni = entry._synth_vni;
      return index;
    }
  } else if (entry._vni == 0 && entry._synth_vni == 0) {
    // If we are storing a vertex *without* any normal, see if we have already
    // stored a vertex with a normal first.
    ni = _unique_entries.lower_bound(entry);
    if (ni != _unique_entries.end() && (*ni).first.matches_except_normal(entry)) {
      // We had such a vertex, so use it.
      index = (*ni).second;
      return index;
    }
  }

  // We didn't already have a vertex we could repurpose, so try to add exactly
  // the desired vertex.
  result = _unique_entries.insert(UniqueVertexEntries::value_type(entry, _entries.size()));
  ni = result.first;
  index = (*ni).second;

  if (result.second) {
    // If the vertex was added to the table, it's a unique vertex, and now we
    // have to add it to the vertex data too.
    _entries.push_back(entry);

    if (converter->_v4_given) {
      _v4_given = true;
    }
    if (converter->_vt3_given) {
      _vt3_given = true;
    }
    if (entry._vti != 0) {
      _vt_given = true;
    } else if (entry._vi - 1 < (int)converter->_xvt_table.size()) {
      // We have an xvt texture coordinate.
      _vt_given = true;
    }
    if (entry._vi - 1 < (int)converter->_rgb_table.size()) {
      // We have a per-vertex color too.
      _rgb_given = true;
    }
    if (entry._vni != 0) {
      _vn_given = true;
    }
  }

  return index;
}

/**
 * Adds a triangle to the primitive, as a triple of three VertexEntry objects,
 * which are each added to the vertex pool.  If synth_vni is not 0, it is
 * assigned to the last vertex.
 */
void ObjToEggConverter::VertexData::
add_triangle(const ObjToEggConverter *converter, const VertexEntry &v0,
             const VertexEntry &v1, const VertexEntry &v2,
             int synth_vni) {
  int v0i, v1i, v2i;

  v0i = add_vertex(converter, v0);
  v1i = add_vertex(converter, v1);

  if (synth_vni != 0) {
    VertexEntry v2n(v2);
    v2n._synth_vni = synth_vni;
    v2i = add_vertex(converter, v2n);
  } else {
    v2i = add_vertex(converter, v2);
  }

  _prim->add_vertices(v0i, v1i, v2i);
  _prim->close_primitive();
}

/**
 * Finishes the current geom and stores it as a child in the root.  Prepares
 * for new geoms.
 */
void ObjToEggConverter::VertexData::
close_geom(const ObjToEggConverter *converter) {
  if (_prim->get_num_vertices() != 0) {
    // Create a new format that includes only the columns we actually used.
    PT(GeomVertexArrayFormat) aformat = new GeomVertexArrayFormat;
    if (_v4_given) {
      aformat->add_column(InternalName::get_vertex(), 4,
                          GeomEnums::NT_stdfloat, GeomEnums::C_point);
    } else {
      aformat->add_column(InternalName::get_vertex(), 3,
                          GeomEnums::NT_stdfloat, GeomEnums::C_point);
    }

    // We always add normals--if no normals appeared in the file, we
    // synthesize them.
    aformat->add_column(InternalName::get_normal(), 3,
                        GeomEnums::NT_stdfloat, GeomEnums::C_vector);

    if (_vt_given) {
      if (_vt3_given) {
        aformat->add_column(InternalName::get_texcoord(), 3,
                            GeomEnums::NT_stdfloat, GeomEnums::C_texcoord);
      } else {
        aformat->add_column(InternalName::get_texcoord(), 2,
                            GeomEnums::NT_stdfloat, GeomEnums::C_texcoord);
      }
    }

    if (_rgb_given) {
      aformat->add_column(InternalName::get_color(), 4,
                          GeomEnums::NT_uint8, GeomEnums::C_color);
    }

    CPT(GeomVertexFormat) format = GeomVertexFormat::register_format(aformat);

    // Create and populate the vertex data.
    PT(GeomVertexData) vdata = new GeomVertexData(_name, format, GeomEnums::UH_static);
    GeomVertexWriter vertex_writer(vdata, InternalName::get_vertex());
    GeomVertexWriter normal_writer(vdata, InternalName::get_normal());
    GeomVertexWriter texcoord_writer(vdata, InternalName::get_texcoord());
    GeomVertexWriter color_writer(vdata, InternalName::get_color());

    for (size_t i = 0; i < _entries.size(); ++i) {
      const VertexEntry &entry = _entries[i];

      if (entry._vi != 0) {
        vertex_writer.set_row(i);
        vertex_writer.add_data4d(converter->_v_table[entry._vi - 1]);
      }
      if (entry._vti != 0) {
        texcoord_writer.set_row(i);
        texcoord_writer.add_data3d(converter->_vt_table[entry._vti - 1]);
      } else if (entry._vi - 1 < (int)converter->_xvt_table.size()) {
        // We have an xvt texture coordinate.
        texcoord_writer.set_row(i);
        texcoord_writer.add_data2d(converter->_xvt_table[entry._vi - 1]);
      }
      if (entry._vni != 0) {
        normal_writer.set_row(i);
        normal_writer.add_data3d(converter->_vn_table[entry._vni - 1]);
      } else if (entry._synth_vni != 0) {
        normal_writer.set_row(i);
        normal_writer.add_data3d(converter->_synth_vn_table[entry._synth_vni - 1]);
      } else {
        // In this case, the normal isn't used and doesn't matter; we fill it
        // in a unit vector just for neatness.
        normal_writer.set_row(i);
        normal_writer.add_data3d(0, 0, 1);
      }
      if (_rgb_given) {
        if (entry._vi - 1 < (int)converter->_rgb_table.size()) {
          color_writer.set_row(i);
          color_writer.add_data3d(converter->_rgb_table[entry._vi - 1]);
        }
      }
    }

    // Transform to zup-right.
    vdata->transform_vertices(LMatrix4::convert_mat(CS_zup_right, CS_default));

    // Now create a Geom with this data.
    CPT(RenderState) state = RenderState::make_empty();
    if (_rgb_given) {
      state = state->add_attrib(ColorAttrib::make_vertex());
    } else {
      state = state->add_attrib(ColorAttrib::make_flat(LColor(1, 1, 1, 1)));
    }
    if (!_vn_given) {
      // We have synthesized these normals; specify the flat-shading attrib.
      state = state->add_attrib(ShadeModelAttrib::make(ShadeModelAttrib::M_flat));
      _prim->set_shade_model(GeomEnums::SM_flat_last_vertex);
    }

    PT(Geom) geom = new Geom(vdata);
    geom->add_primitive(_prim);

    if (_geom_node == nullptr) {
      _geom_node = new GeomNode(_name);
      _parent->add_child(_geom_node);
    }

    _geom_node->add_geom(geom, state);
  }

  _prim = new GeomTriangles(GeomEnums::UH_static);
  _entries.clear();
  _unique_entries.clear();
}
