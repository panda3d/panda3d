// Filename: pfmFile.cxx
// Created by:  drose (23Dec10)
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

#include "config_pfm.h"
#include "pfmFile.h"
#include "virtualFileSystem.h"
#include "pandaFileStream.h"
#include "littleEndian.h"
#include "bigEndian.h"
#include "cmath.h"
#include "geomNode.h"
#include "geom.h"
#include "geomVertexData.h"
#include "geomVertexFormat.h"
#include "geomPoints.h"
#include "geomTriangles.h"
#include "geomVertexWriter.h"
#include "look_at.h"

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PfmFile::
PfmFile() {
  _zero_special = false;
  _vis_inverse = false;
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
PfmFile::
PfmFile(const PfmFile &copy) :
  _table(copy._table),
  _x_size(copy._x_size),
  _y_size(copy._y_size),
  _scale(copy._scale),
  _num_channels(copy._num_channels),
  _zero_special(copy._zero_special)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::Copy Assignment
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void PfmFile::
operator = (const PfmFile &copy) {
  _table = copy._table;
  _x_size = copy._x_size;
  _y_size = copy._y_size;
  _scale = copy._scale;
  _num_channels = copy._num_channels;
  _zero_special = copy._zero_special;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::clear
//       Access: Public
//  Description: Eliminates all data in the file.
////////////////////////////////////////////////////////////////////
void PfmFile::
clear() {
  _x_size = 0;
  _y_size = 0;
  _num_channels = 0;
  _table.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::read
//       Access: Public
//  Description: Reads the PFM data from the indicated file, returning
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PfmFile::
read(const Filename &fullpath) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename filename = Filename::binary_filename(fullpath);
  PT(VirtualFile) file = vfs->get_file(filename);
  if (file == (VirtualFile *)NULL) {
    // No such file.
    pfm_cat.error()
      << "Could not find " << fullpath << "\n";
    return false;
  }

  if (pfm_cat.is_debug()) {
    pfm_cat.debug()
      << "Reading PFM file " << filename << "\n";
  }

  istream *in = file->open_read_file(true);
  bool success = read(*in);
  vfs->close_read_file(in);

  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::read
//       Access: Public
//  Description: Reads the PFM data from the indicated stream,
//               returning true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PfmFile::
read(istream &in) {
  clear();

  string identifier;
  in >> identifier;

  if (identifier == "PF") {
    _num_channels = 3;
  } else if (identifier == "Pf") {
    _num_channels = 1;
  } else {
    pfm_cat.error()
      << "Not a pfm file.\n";
    return false;
  }

  int width, height;
  float scale;
  in >> width >> height >> scale;
  if (!in) {
    pfm_cat.error()
      << "Error parsing pfm header.\n";
    return false;
  }

  // Skip the last newline/whitespace character before the raw data
  // begins.
  in.get();

  bool little_endian = false;
  if (scale < 0) {
    scale = -scale;
    little_endian = true;
  }
  if (pfm_force_littleendian) {
    little_endian = true;
  }
  if (pfm_reverse_dimensions) {
    int t = width;
    width = height;
    height = t;
  }

  _x_size = width;
  _y_size = height;
  _scale = scale;

  // So far, so good.  Now read the data.
  int size = _x_size * _y_size;
  _table.reserve(size);

  if (little_endian) {
    for (int i = 0; i < size; ++i) {
      LPoint3f point = LPoint3f::zero();
      for (int ci = 0; ci < _num_channels; ++ci) {
        float data;
        in.read((char *)&data, sizeof(data));
        LittleEndian value(&data, sizeof(data));
        value.store_value(&(point[ci]), sizeof(point[ci]));
      }
      _table.push_back(point);
    }
  } else {
    for (int i = 0; i < size; ++i) {
      LPoint3f point = LPoint3f::zero();
      for (int ci = 0; ci < _num_channels; ++ci) {
        float data;
        in.read((char *)&data, sizeof(data));
        BigEndian value(&data, sizeof(data));
        value.store_value(&(point[ci]), sizeof(point[ci]));
      }
      _table.push_back(point);
    }
  }

  if (in.fail() && !in.eof()) {
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::write
//       Access: Public
//  Description: Writes the PFM data to the indicated file, returning
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PfmFile::
write(const Filename &fullpath) {
  Filename filename = Filename::binary_filename(fullpath);
  pofstream out;
  if (!filename.open_write(out)) {
    pfm_cat.error()
      << "Unable to open " << filename << "\n";
    return false;
  }

  if (pfm_cat.is_debug()) {
    pfm_cat.debug()
      << "Writing PFM file " << filename << "\n";
  }

  return write(out);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::write
//       Access: Public
//  Description: Writes the PFM data to the indicated stream,
//               returning true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PfmFile::
write(ostream &out) {
  nassertr(is_valid(), false);

  if (_num_channels == 1) {
    out << "Pf\n";
  } else {
    out << "PF\n";
  }
  out << _x_size << " " << _y_size << "\n";

  float scale = cabs(_scale);
  if (scale == 0.0f) {
    scale = 1.0f;
  }
#ifndef WORDS_BIGENDIAN
  // Little-endian must write negative values for scale.
  scale = -scale;
#endif
  out << scale << "\n";

  int size = _x_size * _y_size;
  for (int i = 0; i < size; ++i) {
    const LPoint3f &point = _table[i];
    for (int ci = 0; ci < _num_channels; ++ci) {
      float data = point[ci];
      out.write((const char *)&data, sizeof(data));
    }
  }

  if (out.fail()) {
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::resize
//       Access: Public
//  Description: Applies a simple filter to resample the pfm file
//               in-place to the indicated size.  Don't confuse this
//               with applying a scale to all of the points via
//               xform().
////////////////////////////////////////////////////////////////////
void PfmFile::
resize(int new_x_size, int new_y_size) {
  Table new_data;
  new_data.reserve(new_x_size * new_y_size);

  double from_x0, from_x1, from_y0, from_y1;

  double x_scale = (double)(_x_size - 1) / (double)(new_x_size - 1);
  double y_scale = (double)(_y_size - 1) / (double)(new_y_size - 1);

  from_y0 = 0.0;
  for (int to_y = 0; to_y < new_y_size; ++to_y) {
    from_y1 = (to_y + 0.5) * y_scale;
    from_y1 = min(from_y1, _y_size);

    from_x0 = 0.0;
    for (int to_x = 0; to_x < new_x_size; ++to_x) {
      from_x1 = (to_x + 0.5) * x_scale;
      from_x1 = min(from_x1, _x_size);

      // Now the box from (from_x0, from_y0) - (from_x1, from_y1)
      // but not including (from_x1, from_y1) maps to the pixel (to_x, to_y).
      LPoint3f result;
      box_filter_region(result, from_x0, from_y0, from_x1, from_y1);
      new_data.push_back(result);

      from_x0 = from_x1;
    }
    from_y0 = from_y1;
  }

  _table.swap(new_data);
  _x_size = new_x_size;
  _y_size = new_y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::reverse_rows
//       Access: Public
//  Description: Performs an in-place reversal of the row (y) data.
////////////////////////////////////////////////////////////////////
void PfmFile::
reverse_rows() {
  nassertv(is_valid());

  Table reversed;
  reversed.reserve(_table.size());
  for (int yi = 0; yi < _y_size; ++yi) {
    int source_yi = _y_size - 1 - yi;
    int start = source_yi * _x_size;
    reversed.insert(reversed.end(), 
                    _table.begin() + start, _table.begin() + start + _x_size);
  }

  nassertv(reversed.size() == _table.size());
  _table.swap(reversed);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::xform
//       Access: Public
//  Description: Applies the indicated transform matrix to all points
//               in-place.
////////////////////////////////////////////////////////////////////
void PfmFile::
xform(const LMatrix4f &transform) {
  nassertv(is_valid());

  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    if (_zero_special && (*ti) == LPoint3f::zero()) {
      continue;
    }

    (*ti) = (*ti) * transform;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::compute_planar_bounds
//       Access: Public
//  Description: Computes the minmax bounding volume of the points in
//               3-D space, assuming the points represent a
//               mostly-planar surface.
//
//               This algorithm works by sampling the (square)
//               sample_radius pixels at three of the four point_dist
//               corners around the center (cx - pd, cx + pd) and so
//               on, to determine the plane of the surface.  Then all
//               of the points are projected into that plane and the
//               bounding volume within that plane is determined.
//
//               point_dist and sample_radius are in UV space, i.e. in
//               the range 0..1.
////////////////////////////////////////////////////////////////////
PT(BoundingHexahedron) PfmFile::
compute_planar_bounds(double point_dist, double sample_radius) const {
  LPoint3f p0, p1, p2;
  compute_sample_point(p0, 0.5 + point_dist, 0.5 - point_dist, sample_radius);
  compute_sample_point(p1, 0.5 + point_dist, 0.5 + point_dist, sample_radius);
  compute_sample_point(p2, 0.5 - point_dist, 0.5 + point_dist, sample_radius);

  LPoint3f normal;

  normal[0] = p0[1] * p1[2] - p0[2] * p1[1];
  normal[1] = p0[2] * p1[0] - p0[0] * p1[2];
  normal[2] = p0[0] * p1[1] - p0[1] * p1[0];

  normal[0] += p1[1] * p2[2] - p1[2] * p2[1];
  normal[1] += p1[2] * p2[0] - p1[0] * p2[2];
  normal[2] += p1[0] * p2[1] - p1[1] * p2[0];

  normal[0] += p2[1] * p0[2] - p2[2] * p0[1];
  normal[1] += p2[2] * p0[0] - p2[0] * p0[2];
  normal[2] += p2[0] * p0[1] - p2[1] * p0[0];

  normal.normalize();

  // Compute the transform necessary to rotate all of the points into
  // the Y = 0 plane.
  LMatrix4f rotate;
  look_at(rotate, normal, p1 - p0);

  LMatrix4f rinv;
  rinv.invert_from(rotate);

  LPoint3f trans = p0 * rinv;
  rinv.set_row(3, -trans);
  rotate.invert_from(rinv);

  // Now determine the minmax in the XZ plane.
  float min_x, min_z, max_x, max_z;
  bool got_point = false;
  Table::const_iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    if (_zero_special && (*ti) == LPoint3f::zero()) {
      continue;
    }
    LPoint3f point = (*ti) * rinv;
    if (!got_point) {
      min_x = point[0];
      min_z = point[2];
      max_x = point[0];
      max_z = point[2];
      got_point = true;
    } else {
      min_x = min(min_x, point[0]);
      min_z = min(min_z, point[2]);
      max_x = max(max_x, point[0]);
      max_z = max(max_z, point[2]);
    }
  }

  PT(BoundingHexahedron) bounds = new BoundingHexahedron
    (LPoint3f(min_x, 0, min_z), LPoint3f(max_x, 0, min_z),
     LPoint3f(min_x, 0, max_z), LPoint3f(max_x, 0, max_z),
     LPoint3f(min_x, 0, min_z), LPoint3f(max_x, 0, min_z),
     LPoint3f(min_x, 0, max_z), LPoint3f(max_x, 0, max_z));

  // Rotate the bounding volume back into the original space of the
  // screen.
  bounds->xform(rotate);

  return bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::generate_vis_points
//       Access: Public
//  Description: Creates a point cloud with the points of the pfm as
//               3-d coordinates in space, and texture coordinates
//               ranging from 0 .. 1 based on the position within the
//               pfm grid.
////////////////////////////////////////////////////////////////////
NodePath PfmFile::
generate_vis_points() const {
  nassertr(is_valid(), NodePath());

  CPT(GeomVertexFormat) format;
  if (_vis_inverse) {
    // We need a 3-d texture coordinate if we're inverted the vis.
    GeomVertexArrayFormat *v3t3 = new GeomVertexArrayFormat
      (InternalName::get_vertex(), 3, 
       Geom::NT_float32, Geom::C_point,
       InternalName::get_texcoord(), 3, 
       Geom::NT_float32, Geom::C_texcoord);
    format = GeomVertexFormat::register_format(v3t3);
  } else {
    format = GeomVertexFormat::get_v3t2();
  }

  PT(GeomVertexData) vdata = new GeomVertexData
    ("points", format, Geom::UH_static);
  vdata->set_num_rows(_x_size * _y_size);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());
  
  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      const LPoint3f &point = get_point(xi, yi);
      LPoint2f uv(float(xi) / float(_x_size - 1),
                  float(yi) / float(_y_size - 1));
      if (_vis_inverse) {
        vertex.add_data2f(uv);
        texcoord.add_data3f(point);
      } else {
        vertex.add_data3f(point);
        texcoord.add_data2f(uv);
      }
    }
  }
  
  PT(Geom) geom = new Geom(vdata);
  PT(GeomPoints) points = new GeomPoints(Geom::UH_static);
  points->add_next_vertices(_x_size * _y_size);
  geom->add_primitive(points);
  
  PT(GeomNode) gnode = new GeomNode("");
  gnode->add_geom(geom);
  return NodePath(gnode);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::generate_vis_mesh
//       Access: Public
//  Description: Creates a triangle mesh with the points of the pfm as
//               3-d coordinates in space, and texture coordinates
//               ranging from 0 .. 1 based on the position within the
//               pfm grid.
////////////////////////////////////////////////////////////////////
NodePath PfmFile::
generate_vis_mesh() const {
  nassertr(is_valid(), NodePath());
  
  PT(GeomNode) gnode = new GeomNode("");

  PT(Geom) geom1 = make_vis_mesh_geom(false);
  gnode->add_geom(geom1);

  PT(Geom) geom2 = make_vis_mesh_geom(true);
  gnode->add_geom(geom2);

  return NodePath(gnode);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::make_vis_mesh_geom
//       Access: Private
//  Description: Returns a triangle mesh for the pfm.  If inverted is
//               true, the mesh is facing the opposite direction.
////////////////////////////////////////////////////////////////////
PT(Geom) PfmFile::
make_vis_mesh_geom(bool inverted) const {

  CPT(GeomVertexFormat) format;
  if (_vis_inverse) {
    // We need a 3-d texture coordinate if we're inverted the vis.
    // But we don't need normals in that case.
    GeomVertexArrayFormat *v3t3 = new GeomVertexArrayFormat
      (InternalName::get_vertex(), 3, 
       Geom::NT_float32, Geom::C_point,
       InternalName::get_texcoord(), 3, 
       Geom::NT_float32, Geom::C_texcoord);
    format = GeomVertexFormat::register_format(v3t3);
  } else {
    // Otherwise, we only need a 2-d texture coordinate, and we do
    // want normals.
    format = GeomVertexFormat::get_v3n3t2();
  }

  PT(GeomVertexData) vdata = new GeomVertexData
    ("mesh", format, Geom::UH_static);
  int num_vertices = _x_size * _y_size;
  vdata->set_num_rows(num_vertices);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter normal(vdata, InternalName::get_normal());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      const LPoint3f &point = get_point(xi, yi);
      LPoint2f uv(float(xi) / float(_x_size - 1),
                  float(yi) / float(_y_size - 1));

      if (_vis_inverse) {
        vertex.add_data2f(uv);
        texcoord.add_data3f(point);
      } else {
        vertex.add_data3f(point);
        texcoord.add_data2f(uv);

        // Calculate the normal based on two neighboring vertices.
        LPoint3f v[3];
        v[0] = get_point(xi, yi);
        if (xi + 1 < _x_size) {
          v[1] = get_point(xi + 1, yi);
        } else {
          v[1] = v[0];
          v[0] = get_point(xi - 1, yi);
        }
        
        if (yi + 1 < _y_size) {
          v[2] = get_point(xi, yi + 1);
        } else {
          v[2] = v[0];
          v[0] = get_point(xi, yi - 1);
        }
        
        LVector3f n = LVector3f::zero();
        for (int i = 0; i < 3; ++i) {
          const LPoint3f &v0 = v[i];
          const LPoint3f &v1 = v[(i + 1) % 3];
          n[0] += v0[1] * v1[2] - v0[2] * v1[1];
          n[1] += v0[2] * v1[0] - v0[0] * v1[2];
          n[2] += v0[0] * v1[1] - v0[1] * v1[0];
        }
        n.normalize();
        if (inverted) {
          n = -n;
        }
        normal.add_data3f(n);
      }
    }
  }
  
  PT(Geom) geom = new Geom(vdata);
  PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);

  if (num_vertices > 0xffff) {
    // We need 32-bit indices.
    tris->set_index_type(Geom::NT_uint32);
  }

  // We get direct access to the vertices data so we can speed things
  // up by pre-specifying the number of vertices.  Need a better
  // interface to do this same thing using the high-level access
  // methods.
  int num_indices = (_x_size - 1) * (_y_size - 1) * 6;

  PT(GeomVertexArrayData) indices = tris->modify_vertices();
  indices->set_num_rows(num_indices);
  GeomVertexWriter index(indices, 0);

  int actual_num_indices = 0;
  for (int yi = 0; yi < _y_size - 1; ++yi) {
    for (int xi = 0; xi < _x_size - 1; ++xi) {

      if (_zero_special) {
        if (get_point(xi, yi) == LPoint3f::zero() ||
            get_point(xi, yi + 1) == LPoint3f::zero() ||
            get_point(xi + 1, yi + 1) == LPoint3f::zero() ||
            get_point(xi + 1, yi) == LPoint3f::zero()) {
          continue;
        }
      }

      int vi0 = ((xi) + (yi) * _x_size);
      int vi1 = ((xi) + (yi + 1) * _x_size);
      int vi2 = ((xi + 1) + (yi + 1) * _x_size);
      int vi3 = ((xi + 1) + (yi) * _x_size);

      if (inverted) {
        index.add_data1i(vi2);
        index.add_data1i(vi0);
        index.add_data1i(vi1);
        
        index.add_data1i(vi3);
        index.add_data1i(vi0);
        index.add_data1i(vi2);
      } else {
        index.add_data1i(vi2);
        index.add_data1i(vi1);
        index.add_data1i(vi0);
        
        index.add_data1i(vi3);
        index.add_data1i(vi2);
        index.add_data1i(vi0);
      }

      actual_num_indices += 6;
    }
  }
  indices->set_num_rows(actual_num_indices);
  geom->add_primitive(tris);

  return geom;
}


////////////////////////////////////////////////////////////////////
//     Function: PfmFile::compute_sample_point
//       Access: Private
//  Description: Computes the average of all the point within
//               sample_radius (manhattan distance) and the indicated
//               point.
//
//               Unlike box_filter_*(), these point values are given
//               in UV space, in the range 0..1.
////////////////////////////////////////////////////////////////////
void PfmFile::
compute_sample_point(LPoint3f &result,
                     double x, double y, double sample_radius) const {
  x *= _x_size;
  y *= _y_size;
  double xr = sample_radius * _x_size;
  double yr = sample_radius * _y_size;
  box_filter_region(result, x - xr, y - yr, x + xr, y + yr);
}


////////////////////////////////////////////////////////////////////
//     Function: PfmFile::box_filter_region
//       Access: Private
//  Description: Averages all the points in the rectangle from x0
//               .. y0 to x1 .. y1 into result.  The region may be
//               defined by floating-point boundaries; the result will
//               be weighted by the degree of coverage of each
//               included point.
////////////////////////////////////////////////////////////////////
void PfmFile::
box_filter_region(LPoint3f &result,
                  double x0, double y0, double x1, double y1) const {
  result = LPoint3f::zero();
  double coverage = 0.0;

  assert(y0 >= 0.0 && y1 >= 0.0);

  int y = (int)y0;
  // Get the first (partial) row
  box_filter_line(result, coverage, x0, y, x1, (double)(y+1)-y0);

  int y_last = (int)y1;
  if (y < y_last) {
    y++;
    while (y < y_last) {
      // Get each consecutive (complete) row
      box_filter_line(result, coverage, x0, y, x1, 1.0);
      y++;
    }

    // Get the final (partial) row
    double y_contrib = y1 - (double)y_last;
    if (y_contrib > 0.0001) {
      box_filter_line(result, coverage, x0, y, x1, y_contrib);
    }
  }

  if (coverage != 0.0) {
    result /= coverage;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::box_filter_line
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void PfmFile::
box_filter_line(LPoint3f &result, double &coverage,
                double x0, int y, double x1, double y_contrib) const {
  int x = (int)x0;
  // Get the first (partial) xel
  box_filter_point(result, coverage, x, y, (double)(x+1)-x0, y_contrib);

  int x_last = (int)x1;
  if (x < x_last) {
    x++;
    while (x < x_last) {
      // Get each consecutive (complete) xel
      box_filter_point(result, coverage, x, y, 1.0, y_contrib);
      x++;
    }

    // Get the final (partial) xel
    double x_contrib = x1 - (double)x_last;
    if (x_contrib > 0.0001) {
      box_filter_point(result, coverage, x, y, x_contrib, y_contrib);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::box_filter_point
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void PfmFile::
box_filter_point(LPoint3f &result, double &coverage,
                 int x, int y, double x_contrib, double y_contrib) const {
  const LPoint3f &point = get_point(x, y);
  if (_zero_special && point == LPoint3f::zero()) {
    return;
  }

  double contrib = x_contrib * y_contrib;
  result += point * contrib;
  coverage += contrib;
}
