// Filename: objToEggConverter.cxx
// Created by:  drose (07Dec10)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "objToEggConverter.h"
#include "config_objegg.h"
#include "eggData.h"
#include "string_utils.h"
#include "streamReader.h"
#include "virtualFileSystem.h"
#include "eggPolygon.h"
#include "dcast.h"

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ObjToEggConverter::
ObjToEggConverter() {
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ObjToEggConverter::
ObjToEggConverter(const ObjToEggConverter &copy) :
  SomethingToEggConverter(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
ObjToEggConverter::
~ObjToEggConverter() {
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::make_copy
//       Access: Public, Virtual
//  Description: Allocates and returns a new copy of the converter.
////////////////////////////////////////////////////////////////////
SomethingToEggConverter *ObjToEggConverter::
make_copy() {
  return new ObjToEggConverter(*this);
}


////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::get_name
//       Access: Public, Virtual
//  Description: Returns the English name of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string ObjToEggConverter::
get_name() const {
  return "obj";
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::get_extension
//       Access: Public, Virtual
//  Description: Returns the common extension of the file type this
//               converter supports.
////////////////////////////////////////////////////////////////////
string ObjToEggConverter::
get_extension() const {
  return "obj";
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::supports_compressed
//       Access: Published, Virtual
//  Description: Returns true if this file type can transparently load
//               compressed files (with a .pz extension), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool ObjToEggConverter::
supports_compressed() const {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::convert_file
//       Access: Public, Virtual
//  Description: Handles the reading of the input file and converting
//               it to egg.  Returns true if successful, false
//               otherwise.
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::process
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
bool ObjToEggConverter::
process(const Filename &filename) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *strm = vfs->open_read_file(filename, true);
  if (strm == NULL) {
    objegg_cat.error() 
      << "Couldn't read " << filename << "\n";
    return false;
  }

  _vi = 1;
  _vti = 1;
  _xvti = 1;
  _vni = 1;
  _ref_plane_res.set(1.0, 1.0);

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

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::process_line
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::process_line
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
bool ObjToEggConverter::
process_ref_plane_res(const string &line) {
  // the #_ref_plane_res line is a DRZ extension that defines the
  // pixel resolution of the projector device.  It's needed to
  // properly scale the xvt lines.

  vector_string words;
  tokenize(line, words, " \t", true);
  nassertr(!words.empty(), false);

  if (words.size() != 3) {
    objegg_cat.error()
      << "Wrong number of tokens at line " << _line_number << "\n";
    return false;
  }

  bool okflag = true;
  LPoint3d pos;
  okflag &= string_to_double(words[1], _ref_plane_res[0]);
  okflag &= string_to_double(words[2], _ref_plane_res[1]);

  if (!okflag) {
    objegg_cat.error()
      << "Invalid number at line " << _line_number << "\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::process_v
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
bool ObjToEggConverter::
process_v(vector_string &words) {
  if (words.size() != 4 && words.size() != 7) {
    objegg_cat.error()
      << "Wrong number of tokens at line " << _line_number << "\n";
    return false;
  }
  
  bool okflag = true;
  LPoint3d pos;
  okflag &= string_to_double(words[1], pos[0]);
  okflag &= string_to_double(words[2], pos[1]);
  okflag &= string_to_double(words[3], pos[2]);

  if (!okflag) {
    objegg_cat.error()
      << "Invalid number at line " << _line_number << "\n";
    return false;
  }

  EggVertex *vertex = get_vertex(_vi);
  vertex->set_pos(pos);

  // Meshlab format might include an RGB color following the vertex
  // position.
  if (words.size() == 7) {
    double r, g, b;
    okflag &= string_to_double(words[4], r);
    okflag &= string_to_double(words[5], g);
    okflag &= string_to_double(words[6], b);

    if (!okflag) {
      objegg_cat.error()
        << "Invalid number at line " << _line_number << "\n";
      return false;
    }
    vertex->set_color(LColor(r, g, b, 1.0));
  }

  ++_vi;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::process_vt
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
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
  }

  if (!okflag) {
    objegg_cat.error()
      << "Invalid number at line " << _line_number << "\n";
    return false;
  }

  EggVertex *vertex = get_vertex(_vti);
  if (words.size() == 4) {
    vertex->set_uvw("", uvw);
  } else {
    vertex->set_uv("", LTexCoordd(uvw[0], uvw[1]));
  }
  ++_vti;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::process_xvt
//       Access: Protected
//  Description: "xvt" is an extended column invented by DRZ.  It
//               includes texture coordinates in pixel space of the
//               projector device, as well as for each camera.  We map
//               it to the nominal texture coordinates here.
////////////////////////////////////////////////////////////////////
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

  EggVertex *vertex = get_vertex(_xvti);
  vertex->set_uv("", uv);
  ++_xvti;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::process_vn
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
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

  EggVertex *vertex = get_vertex(_vni);
  vertex->set_normal(normal);
  ++_vni;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::process_f
//       Access: Protected
//  Description: Defines a face in the obj file.
////////////////////////////////////////////////////////////////////
bool ObjToEggConverter::
process_f(vector_string &words) {
  PT(EggPolygon) poly = new EggPolygon;
  for (size_t i = 1; i < words.size(); ++i) {
    EggVertex *vertex = get_face_vertex(words[i]);
    if (vertex == NULL) {
      return false;
    }
    poly->add_vertex(vertex);
  }
  _current_group->add_child(poly);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::process_g
//       Access: Protected
//  Description: Defines a group in the obj file.
////////////////////////////////////////////////////////////////////
bool ObjToEggConverter::
process_g(vector_string &words) {
  EggGroup *group = _root_group;

  // We assume the group names define a hierarchy of more-specific to
  // less-specific group names, so that the first group name is the
  // bottommost node, and the last group name is the topmost node.

  // Thus, iterate from the back to the front.
  size_t i = words.size();
  while (i > 1) {
    --i;
    EggNode *child = group->find_child(words[i]);
    if (child == NULL || !child->is_of_type(EggGroup::get_class_type())) {
      child = new EggGroup(words[i]);
      group->add_child(child);
    }
    group = DCAST(EggGroup, child);
  }

  _current_group = group;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::get_vertex
//       Access: Protected
//  Description: Returns or creates a vertex in the vpool with the
//               given index.
////////////////////////////////////////////////////////////////////
EggVertex *ObjToEggConverter::
get_vertex(int n) {
  if (n < 0) {
    // A negative index means to count backward from the end.
    n = _vi + n;
  }
  EggVertex *vertex = _vpool->get_vertex(n);
  if (vertex == NULL) {
    vertex = new EggVertex;
    _vpool->add_vertex(vertex, n);
  }

  return vertex;
}

////////////////////////////////////////////////////////////////////
//     Function: ObjToEggConverter::get_face_vertex
//       Access: Protected
//  Description: Returns or creates a vertex in the vpool according to
//               the indicated face reference.
////////////////////////////////////////////////////////////////////
EggVertex *ObjToEggConverter::
get_face_vertex(const string &reference) {
  vector_string words;
  tokenize(reference, words, "/", false);
  nassertr(!words.empty(), NULL);

  vector<EggVertex *> vertices;
  vertices.reserve(words.size());
  for (size_t i = 0; i < words.size(); ++i) {
    if (trim(words[i]).empty()) {
      if (i == 0) {
        objegg_cat.error()                    
          << "Invalid null vertex at line " << _line_number << "\n";
        return NULL;
      } else {
        vertices.push_back(vertices[0]);
        continue;
      }
    }

    int index;
    if (!string_to_int(words[i], index)) {
      objegg_cat.error()
        << "Invalid integer " << words[i] << " at line " << _line_number << "\n";
      return NULL;
    }
    EggVertex *vertex = get_vertex(index);
    if (vertex == NULL){ 
      objegg_cat.error()
        << "Invalid vertex " << index << " at line " << _line_number << "\n";
      return NULL;
    }
    vertices.push_back(vertex);
  }
  nassertr(!vertices.empty(), NULL);
  nassertr(vertices.size() == words.size(), NULL);

  if (vertices.size() == 1) {
    // Just a pos reference.
    return vertices[0];

  } else if (vertices.size() == 2) {
    // Pos + uv.
    if (vertices[0] == vertices[1]) {
      return vertices[0];
    }
    // Synthesize a vertex.
    EggVertex synth(*vertices[0]);
    if (vertices[1]->has_uv("")) {
      synth.set_uv("", vertices[1]->get_uv(""));
    } else if (vertices[1]->has_uvw("")) {
      synth.set_uvw("", vertices[1]->get_uvw(""));
    }

    return _vpool->create_unique_vertex(synth);

  } else if (vertices.size() >= 3) {
    // pos + uv + normal.
    if (vertices[0] == vertices[1] && vertices[0] == vertices[2]) {
      return vertices[0];
    }

    // Synthesize a vertex.
    EggVertex synth(*vertices[0]);
    if (vertices[1]->has_uv("")) {
      synth.set_uv("", vertices[1]->get_uv(""));
    } else if (vertices[1]->has_uvw("")) {
      synth.set_uvw("", vertices[1]->get_uvw(""));
    }
    if (vertices[2]->has_normal()) {
      synth.set_normal(vertices[2]->get_normal());
    }

    return _vpool->create_unique_vertex(synth);
  }

  return NULL;
}
