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

#include "config_grutil.h"
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
#include "lens.h"
#include "look_at.h"

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PfmFile::
PfmFile() {
  _zero_special = false;
  _vis_inverse = false;
  _vis_2d = false;
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::Copy Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PfmFile::
PfmFile(const PfmFile &copy) :
  _table(copy._table),
  _x_size(copy._x_size),
  _y_size(copy._y_size),
  _scale(copy._scale),
  _num_channels(copy._num_channels),
  _zero_special(copy._zero_special),
  _vis_inverse(copy._vis_inverse),
  _vis_2d(copy._vis_2d)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::Copy Assignment
//       Access: Published
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
  _vis_inverse = copy._vis_inverse;
  _vis_2d = copy._vis_2d;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::clear
//       Access: Published
//  Description: Eliminates all data in the file.
////////////////////////////////////////////////////////////////////
void PfmFile::
clear() {
  _x_size = 0;
  _y_size = 0;
  _scale = 1.0;
  _num_channels = 3;
  _table.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::clear
//       Access: Published
//  Description: Resets to an empty table with a specific size.
////////////////////////////////////////////////////////////////////
void PfmFile::
clear(int x_size, int y_size, int num_channels) {
  nassertv(num_channels == 1 || num_channels == 3);
  nassertv(x_size >= 0 && y_size >= 0);
  _x_size = x_size;
  _y_size = y_size;
  _scale = 1.0;
  _num_channels = _num_channels;

  _table.clear();
  int size = _x_size * _y_size;
  _table.insert(_table.end(), size, LPoint3::zero());
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::read
//       Access: Published
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
    grutil_cat.error()
      << "Could not find " << fullpath << "\n";
    return false;
  }

  if (grutil_cat.is_debug()) {
    grutil_cat.debug()
      << "Reading PFM file " << filename << "\n";
  }

  istream *in = file->open_read_file(true);
  bool success = read(*in);
  vfs->close_read_file(in);

  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::read
//       Access: Published
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
    grutil_cat.error()
      << "Not a pfm file.\n";
    return false;
  }

  int width, height;
  PN_stdfloat scale;
  in >> width >> height >> scale;
  if (!in) {
    grutil_cat.error()
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
      LPoint3 point = LPoint3::zero();
      for (int ci = 0; ci < _num_channels; ++ci) {
        PN_float32 data;
        in.read((char *)&data, sizeof(data));
        LittleEndian value(&data, sizeof(data));
        PN_float32 result;
        value.store_value(&result, sizeof(result));
        if (!cnan(result)) {
          point[ci] = result;
        }
      }
      _table.push_back(point);
    }
  } else {
    for (int i = 0; i < size; ++i) {
      LPoint3 point = LPoint3::zero();
      for (int ci = 0; ci < _num_channels; ++ci) {
        PN_float32 data;
        in.read((char *)&data, sizeof(data));
        BigEndian value(&data, sizeof(data));
        PN_float32 result;
        value.store_value(&result, sizeof(result));
        if (!cnan(result)) {
          point[ci] = result;
        }
      }
      _table.push_back(point);
    }
  }

  if (in.fail() && !in.eof()) {
    return false;
  }

  nassertr(sizeof(PN_float32) == 4, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::write
//       Access: Published
//  Description: Writes the PFM data to the indicated file, returning
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PfmFile::
write(const Filename &fullpath) {
  Filename filename = Filename::binary_filename(fullpath);
  pofstream out;
  if (!filename.open_write(out)) {
    grutil_cat.error()
      << "Unable to open " << filename << "\n";
    return false;
  }

  if (grutil_cat.is_debug()) {
    grutil_cat.debug()
      << "Writing PFM file " << filename << "\n";
  }

  return write(out);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::write
//       Access: Published
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

  PN_stdfloat scale = cabs(_scale);
  if (scale == 0.0f) {
    scale = 1.0f;
  }
#ifndef WORDS_BIGENDIAN
  // Little-endian computers must write a negative scale to indicate
  // the little-endian nature of the output.
  scale = -scale;
#endif
  out << scale << "\n";

  int size = _x_size * _y_size;
  for (int i = 0; i < size; ++i) {
    const LPoint3 &point = _table[i];
    for (int ci = 0; ci < _num_channels; ++ci) {
      PN_float32 data = point[ci];
      out.write((const char *)&data, sizeof(data));
    }
  }

  if (out.fail()) {
    return false;
  }
  nassertr(sizeof(PN_float32) == 4, false);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::calc_average_point
//       Access: Published
//  Description: Computes the unweighted average point of all points
//               within the box centered at (x, y) with the indicated
//               Manhattan-distance radius.  Missing points are
//               assigned the value of their nearest neighbor.
//               Returns true if successful, or false if the point
//               value cannot be determined.
////////////////////////////////////////////////////////////////////
bool PfmFile::
calc_average_point(LPoint3 &result, double x, double y, double radius) const {
  result = LPoint3::zero();

  int min_x = int(ceil(x - radius));
  int min_y = int(ceil(y - radius));
  int max_x = int(floor(x + radius));
  int max_y = int(floor(y + radius));

  // We first construct a mini-grid of x_size by y_size integer values
  // to index into the main table.  This indirection allows us to fill
  // in the holes in the mini-grid with the nearest known values
  // before we compute the average.
  int x_size = max_x - min_x + 1;
  int y_size = max_y - min_y + 1;
  int size = x_size * y_size;
  if (size == 0) {
    return false;
  }

  pvector<MiniGridCell> mini_grid;
  mini_grid.insert(mini_grid.end(), size, MiniGridCell());

  // Now collect the known data points and apply them to the
  // mini-grid.
  min_x = max(min_x, 0);
  min_y = max(min_y, 0);
  max_x = min(max_x, _x_size - 1);
  max_y = min(max_y, _y_size - 1);

  bool got_any = false;
  int xi, yi;
  for (yi = min_y; yi <= max_y; ++yi) {
    for (xi = min_x; xi <= max_x; ++xi) {
      const LPoint3 &p = _table[yi * _x_size + xi];
      if (_zero_special && p == LPoint3::zero()) {
        continue;
      }

      int gi = (yi - min_y) * y_size + (xi - min_x);
      nassertr(gi >= 0 && gi < size, false);
      mini_grid[gi]._ti = yi * _x_size + xi;
      mini_grid[gi]._dist = 0;
      got_any = true;
    }
  }

  if (!got_any) {
    return false;
  }

  // Now recursively fill in any missing holes in the mini-grid.
  for (yi = 0; yi < y_size; ++yi) {
    for (xi = 0; xi < x_size; ++xi) {
      int gi = yi * x_size + xi;
      if (mini_grid[gi]._dist == 0) {
        int ti = mini_grid[gi]._ti;
        fill_mini_grid(&mini_grid[0], x_size, y_size, xi + 1, yi, 1, ti);
        fill_mini_grid(&mini_grid[0], x_size, y_size, xi - 1, yi, 1, ti);
        fill_mini_grid(&mini_grid[0], x_size, y_size, xi, yi + 1, 1, ti);
        fill_mini_grid(&mini_grid[0], x_size, y_size, xi, yi - 1, 1, ti);
      }
    }
  }

  // Now the mini-grid is completely filled, so we can compute the
  // average.
  for (int gi = 0; gi < size; ++gi) {
    int ti = mini_grid[gi]._ti;
    nassertr(ti >= 0 && ti < (int)_table.size(), false);
    result += _table[ti];
  }

  result /= PN_stdfloat(size);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::calc_min_max
//       Access: Published
//  Description: Calculates the minimum and maximum x, y, and z depth
//               component values, representing the bounding box of
//               depth values, and places them in the indicated
//               vectors.  Returns true if successful, false if the
//               mesh contains no points.
////////////////////////////////////////////////////////////////////
bool PfmFile::
calc_min_max(LVecBase3 &min_depth, LVecBase3 &max_depth) const {
  bool any_points = false;

  min_depth = LVecBase3::zero();
  max_depth = LVecBase3::zero();

  Table::const_iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    const LPoint3 &p = (*ti);
    if (_zero_special && p == LPoint3::zero()) {
      continue;
    }
    
    if (!any_points) {
      min_depth = p;
      max_depth = p;
      any_points = true;
    } else {
      min_depth[0] = min(min_depth[0], p[0]);
      min_depth[1] = min(min_depth[1], p[1]);
      min_depth[2] = min(min_depth[2], p[2]);
      max_depth[0] = max(max_depth[0], p[0]);
      max_depth[1] = max(max_depth[1], p[1]);
      max_depth[2] = max(max_depth[2], p[2]);
    }
  }

  return any_points;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::resize
//       Access: Published
//  Description: Applies a simple filter to resample the pfm file
//               in-place to the indicated size.  Don't confuse this
//               with applying a scale to all of the points via
//               xform().
////////////////////////////////////////////////////////////////////
void PfmFile::
resize(int new_x_size, int new_y_size) {
  if (_x_size == 0 || _y_size == 0 || new_x_size == 0 || new_y_size == 0) {
    clear(new_x_size, new_y_size, _num_channels);
    return;
  }

  if (new_x_size == _x_size && new_y_size == _y_size) {
    return;
  }

  Table new_data;
  new_data.reserve(new_x_size * new_y_size);

  double from_x0, from_x1, from_y0, from_y1;

  double x_scale = 1.0;
  double y_scale = 1.0;

  if (new_x_size > 1) {
    x_scale = (double)(_x_size - 1) / (double)(new_x_size - 1);
  }
  if (new_y_size > 1) {
    y_scale = (double)(_y_size - 1) / (double)(new_y_size - 1);
  }

  from_y0 = 0.0;
  for (int to_y = 0; to_y < new_y_size; ++to_y) {
    from_y1 = (to_y + 0.5) * y_scale;
    from_y1 = min(from_y1, (double) _y_size);

    from_x0 = 0.0;
    for (int to_x = 0; to_x < new_x_size; ++to_x) {
      from_x1 = (to_x + 0.5) * x_scale;
      from_x1 = min(from_x1, (double) _x_size);

      // Now the box from (from_x0, from_y0) - (from_x1, from_y1)
      // but not including (from_x1, from_y1) maps to the pixel (to_x, to_y).
      LPoint3 result;
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
//       Access: Published
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
//       Access: Published
//  Description: Applies the indicated transform matrix to all points
//               in-place.
////////////////////////////////////////////////////////////////////
void PfmFile::
xform(const LMatrix4 &transform) {
  nassertv(is_valid());

  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    if (_zero_special && (*ti) == LPoint3::zero()) {
      continue;
    }

    (*ti) = (*ti) * transform;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::project
//       Access: Published
//  Description: Adjusts each (x, y, z) point of the Pfm file by
//               projecting it through the indicated lens, converting
//               each point to a (u, v, 0) texture coordinate.  The
//               resulting file can be generated to a mesh (with
//               set_vis_inverse(true) and generate_vis_mesh())
//               that will apply the lens distortion to an arbitrary
//               texture image.
////////////////////////////////////////////////////////////////////
void PfmFile::
project(const Lens *lens) {
  nassertv(is_valid());

  static LMatrix4 to_uv(0.5, 0.0, 0.0, 0.0,
                        0.0, 0.5, 0.0, 0.0, 
                        0.0, 0.0, 0.0, 0.0, 
                        0.5, 0.5, 0.0, 1.0);
  
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    if (_zero_special && (*ti) == LPoint3::zero()) {
      continue;
    }

    LPoint3 &p = (*ti);
    LPoint3 film;
    lens->project(p, film);
    p = to_uv.xform_point(film);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::compute_planar_bounds
//       Access: Published
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
  LPoint3 p0, p1, p2;
  compute_sample_point(p0, 0.5 + point_dist, 0.5 - point_dist, sample_radius);
  compute_sample_point(p1, 0.5 + point_dist, 0.5 + point_dist, sample_radius);
  compute_sample_point(p2, 0.5 - point_dist, 0.5 + point_dist, sample_radius);

  LPoint3 normal;

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
  LMatrix4 rotate;
  look_at(rotate, normal, p1 - p0);

  LMatrix4 rinv;
  rinv.invert_from(rotate);

  LPoint3 trans = p0 * rinv;
  rinv.set_row(3, -trans);
  rotate.invert_from(rinv);

  // Now determine the minmax in the XZ plane.
  PN_stdfloat min_x, min_z, max_x, max_z;
  bool got_point = false;
  Table::const_iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    if (_zero_special && (*ti) == LPoint3::zero()) {
      continue;
    }
    LPoint3 point = (*ti) * rinv;
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
    (LPoint3(min_x, 0, min_z), LPoint3(max_x, 0, min_z),
     LPoint3(min_x, 0, max_z), LPoint3(max_x, 0, max_z),
     LPoint3(min_x, 0, min_z), LPoint3(max_x, 0, min_z),
     LPoint3(min_x, 0, max_z), LPoint3(max_x, 0, max_z));

  // Rotate the bounding volume back into the original space of the
  // screen.
  bounds->xform(rotate);

  return bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::generate_vis_points
//       Access: Published
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
    if (_vis_2d) {
      format = GeomVertexFormat::get_v3t2();
    } else {
      // We need a 3-d texture coordinate if we're inverting the vis
      // and it's 3-d.
      GeomVertexArrayFormat *v3t3 = new GeomVertexArrayFormat
        (InternalName::get_vertex(), 3, 
         Geom::NT_stdfloat, Geom::C_point,
         InternalName::get_texcoord(), 3, 
         Geom::NT_stdfloat, Geom::C_texcoord);
      format = GeomVertexFormat::register_format(v3t3);
    }
  } else {
    format = GeomVertexFormat::get_v3t2();
  }

  PT(GeomVertexData) vdata = new GeomVertexData
    ("points", format, Geom::UH_static);
  vdata->set_num_rows(_x_size * _y_size);
  GeomVertexWriter vertex(vdata, InternalName::get_vertex());
  GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

  LPoint2 uv_scale(1.0, 1.0);
  if (_x_size > 1) {
    uv_scale[0] = 1.0f / PN_stdfloat(_x_size - 1);
  }
  if (_y_size > 1) {
    uv_scale[1] = 1.0f / PN_stdfloat(_y_size - 1);
  }
  
  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      const LPoint3 &point = get_point(xi, yi);
      LPoint2 uv(PN_stdfloat(xi) * uv_scale[0],
                 PN_stdfloat(yi) * uv_scale[1]);
      if (_vis_inverse) {
        vertex.add_data2(uv);
        texcoord.add_data3(point);
      } else if (_vis_2d) {
        vertex.add_data2(point[0], point[1]);
        texcoord.add_data2(uv);
      } else {
        vertex.add_data3(point);
        texcoord.add_data2(uv);
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
//       Access: Published
//  Description: Creates a triangle mesh with the points of the pfm as
//               3-d coordinates in space, and texture coordinates
//               ranging from 0 .. 1 based on the position within the
//               pfm grid.
////////////////////////////////////////////////////////////////////
NodePath PfmFile::
generate_vis_mesh(MeshFace face) const {
  nassertr(is_valid(), NodePath());
  nassertr(face != 0, NodePath());

  if (_x_size == 1 || _y_size == 1) {
    // Can't generate a 1-d mesh, so generate points in this case.
    return generate_vis_points();
  }
  
  PT(GeomNode) gnode = new GeomNode("");

  if (face & MF_front) {
    make_vis_mesh_geom(gnode, false);
  }

  if (face & MF_back) {
    make_vis_mesh_geom(gnode, true);
  }

  return NodePath(gnode);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::make_vis_mesh_geom
//       Access: Private
//  Description: Returns a triangle mesh for the pfm.  If inverted is
//               true, the mesh is facing the opposite direction.
////////////////////////////////////////////////////////////////////
void PfmFile::
make_vis_mesh_geom(GeomNode *gnode, bool inverted) const {
  int num_x_cells = 1;
  int num_y_cells = 1;

  int x_size = _x_size;
  int y_size = _y_size;

  // This is the number of independent vertices we will require.
  int num_vertices = x_size * y_size;
  if (num_vertices == 0) {
    // Trivial no-op.
    return;
  }

  // This is the max number of vertex indices we might add to the
  // GeomTriangles.  (We might actually add fewer than this due to
  // omitting the occasional missing data point.)
  int max_indices = (x_size - 1) * (y_size - 1) * 6;

  while (num_vertices > pfm_vis_max_vertices || max_indices > pfm_vis_max_indices) {
    // Too many vertices in one mesh.  Subdivide the mesh into smaller
    // pieces.
    if (num_x_cells > num_y_cells) {
      ++num_y_cells;
    } else {
      ++num_x_cells;
    }

    x_size = (_x_size + num_x_cells - 1) / num_x_cells + 1;
    y_size = (_y_size + num_y_cells - 1) / num_y_cells + 1;

    num_vertices = x_size * y_size;
    max_indices = (x_size - 1) * (y_size - 1) * 6;
  }

  // OK, now we know how many cells we need.
  if (grutil_cat.is_debug()) {
    grutil_cat.debug()
      << "Generating mesh with " << num_x_cells << " x " << num_y_cells
      << " pieces.\n";
  }

  CPT(GeomVertexFormat) format;
  if (_vis_2d) {
    // No normals needed if we're just generating a 2-d mesh.
    format = GeomVertexFormat::get_v3t2();
  } else {
    if (_vis_inverse) {
      // We need a 3-d texture coordinate if we're inverting the vis
      // and it's 3-d.  But we still don't need normals in that case.
      GeomVertexArrayFormat *v3t3 = new GeomVertexArrayFormat
        (InternalName::get_vertex(), 3, 
         Geom::NT_stdfloat, Geom::C_point,
         InternalName::get_texcoord(), 3, 
         Geom::NT_stdfloat, Geom::C_texcoord);
      format = GeomVertexFormat::register_format(v3t3);
    } else {
      // Otherwise, we only need a 2-d texture coordinate, and we do
      // want normals.
      format = GeomVertexFormat::get_v3n3t2();
    }
  }

  for (int yci = 0; yci < num_y_cells; ++yci) {
    int y_begin = (yci * _y_size) / num_y_cells;
    int y_end = ((yci + 1) * _y_size) / num_y_cells;

    // Include the first vertex from the next strip in this strip's
    // vertices, so we are connected.
    y_end = min(y_end + 1, _y_size);

    y_size = y_end - y_begin;
    if (y_size == 0) {
      continue;
    }

    for (int xci = 0; xci < num_x_cells; ++xci) {
      int x_begin = (xci * _x_size) / num_x_cells;
      int x_end = ((xci + 1) * _x_size) / num_x_cells;
      x_end = min(x_end + 1, _x_size);
      x_size = x_end - x_begin;
      if (x_size == 0) {
        continue;
      }

      num_vertices = x_size * y_size;
      max_indices = (x_size - 1) * (y_size - 1) * 6;

      ostringstream mesh_name;
      mesh_name << "mesh_" << xci << "_" << yci;
      PT(GeomVertexData) vdata = new GeomVertexData
        (mesh_name.str(), format, Geom::UH_static);

      vdata->set_num_rows(num_vertices);
      GeomVertexWriter vertex(vdata, InternalName::get_vertex());
      GeomVertexWriter normal(vdata, InternalName::get_normal());
      GeomVertexWriter texcoord(vdata, InternalName::get_texcoord());

      for (int yi = y_begin; yi < y_end; ++yi) {
        for (int xi = x_begin; xi < x_end; ++xi) {
          const LPoint3 &point = get_point(xi, yi);
          LPoint2 uv(PN_stdfloat(xi) / PN_stdfloat(_x_size - 1),
                     PN_stdfloat(yi) / PN_stdfloat(_y_size - 1));

          if (_vis_inverse) {
            vertex.add_data2(uv);
            texcoord.add_data3(point);
          } else if (_vis_2d) {
            vertex.add_data2(point[0], point[1]);
            texcoord.add_data2(uv);
          } else {
            vertex.add_data3(point);
            texcoord.add_data2(uv);
            
            // Calculate the normal based on two neighboring vertices.
            LPoint3 v[3];
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
        
            LVector3 n = LVector3::zero();
            for (int i = 0; i < 3; ++i) {
              const LPoint3 &v0 = v[i];
              const LPoint3 &v1 = v[(i + 1) % 3];
              n[0] += v0[1] * v1[2] - v0[2] * v1[1];
              n[1] += v0[2] * v1[0] - v0[0] * v1[2];
              n[2] += v0[0] * v1[1] - v0[1] * v1[0];
            }
            n.normalize();
            nassertv(!n.is_nan());
            if (inverted) {
              n = -n;
            }
            normal.add_data3(n);
          }
        }
      }
  
      PT(Geom) geom = new Geom(vdata);
      PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);

      tris->reserve_num_vertices(max_indices);

      for (int yi = y_begin; yi < y_end - 1; ++yi) {
        for (int xi = x_begin; xi < x_end - 1; ++xi) {

          if (_zero_special) {
            if (get_point(xi, yi) == LPoint3::zero() ||
                get_point(xi, yi + 1) == LPoint3::zero() ||
                get_point(xi + 1, yi + 1) == LPoint3::zero() ||
                get_point(xi + 1, yi) == LPoint3::zero()) {
              continue;
            }
          }

          int xi0 = xi - x_begin;
          int yi0 = yi - y_begin;

          int vi0 = ((xi0) + (yi0) * x_size);
          int vi1 = ((xi0) + (yi0 + 1) * x_size);
          int vi2 = ((xi0 + 1) + (yi0 + 1) * x_size);
          int vi3 = ((xi0 + 1) + (yi0) * x_size);
          
          if (inverted) {
            tris->add_vertices(vi2, vi0, vi1);
            tris->close_primitive();
            
            tris->add_vertices(vi3, vi0, vi2);
            tris->close_primitive();
          } else {
            tris->add_vertices(vi2, vi1, vi0);
            tris->close_primitive();
            
            tris->add_vertices(vi3, vi2, vi0);
            tris->close_primitive();
          }
        }
      }
      geom->add_primitive(tris);
      gnode->add_geom(geom);
    }
  }
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
compute_sample_point(LPoint3 &result,
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
box_filter_region(LPoint3 &result,
                  double x0, double y0, double x1, double y1) const {
  result = LPoint3::zero();
  double coverage = 0.0;

  if (x1 < x0 || y1 < y0) {
    return;
  }
  nassertv(y0 >= 0.0 && y1 >= 0.0);

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
box_filter_line(LPoint3 &result, double &coverage,
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
box_filter_point(LPoint3 &result, double &coverage,
                 int x, int y, double x_contrib, double y_contrib) const {
  const LPoint3 &point = get_point(x, y);
  if (_zero_special && point == LPoint3::zero()) {
    return;
  }

  double contrib = x_contrib * y_contrib;
  result += point * contrib;
  coverage += contrib;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::fill_mini_grid
//       Access: Private
//  Description: A support function for calc_average_point(), this
//               recursively fills in the holes in the mini_grid data
//               with the index to the nearest value.
////////////////////////////////////////////////////////////////////
void PfmFile::
fill_mini_grid(MiniGridCell *mini_grid, int x_size, int y_size, 
               int xi, int yi, int dist, int ti) const {
  if (xi < 0 || xi >= x_size || yi < 0 || yi >= y_size) {
    // Out of bounds.
    return;
  }

  int gi = yi * x_size + xi;
  if (mini_grid[gi]._dist == -1 || mini_grid[gi]._dist > dist) {
    // Here's an undefined value that we need to populate.
    mini_grid[gi]._dist = dist;
    mini_grid[gi]._ti = ti;
    fill_mini_grid(mini_grid, x_size, y_size, xi + 1, yi, dist + 1, ti);
    fill_mini_grid(mini_grid, x_size, y_size, xi - 1, yi, dist + 1, ti);
    fill_mini_grid(mini_grid, x_size, y_size, xi, yi + 1, dist + 1, ti);
    fill_mini_grid(mini_grid, x_size, y_size, xi, yi - 1, dist + 1, ti);
  }
}
