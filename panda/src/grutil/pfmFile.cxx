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
#include "pnmImage.h"
#include "pnmWriter.h"
#include "string_utils.h"
#include "lens.h"
#include "look_at.h"

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
PfmFile::
PfmFile() {
  _has_no_data_value = false;
  _no_data_value = LPoint3::zero();
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
  _has_no_data_value(copy._has_no_data_value),
  _no_data_value(copy._no_data_value),
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
  _has_no_data_value = copy._has_no_data_value;
  _no_data_value = copy._no_data_value;
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
  _num_channels = num_channels;

  _table.clear();
  int size = _x_size * _y_size;
  _table.insert(_table.end(), size, LPoint3::zero());
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::read
//       Access: Published
//  Description: Reads the PFM data from the indicated file, returning
//               true on success, false on failure.
//
//               This can also handle reading a standard image file
//               supported by PNMImage; it will be quietly converted
//               to a floating-point type.
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
  bool success = read(*in, fullpath);
  vfs->close_read_file(in);

  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::read
//       Access: Published
//  Description: Reads the PFM data from the indicated stream,
//               returning true on success, false on failure.
//
//               This can also handle reading a standard image file
//               supported by PNMImage; it will be quietly converted
//               to a floating-point type.
////////////////////////////////////////////////////////////////////
bool PfmFile::
read(istream &in, const Filename &fullpath, const string &magic_number) {
  clear();

  string identifier = magic_number;
  PNMImageHeader::read_magic_number(&in, identifier, 2);

  if (identifier == "PF") {
    _num_channels = 3;
  } else if (identifier == "Pf") {
    _num_channels = 1;
  } else {
    // Not a PFM file.  Maybe it's a more conventional image file that
    // we can read into a PFM.
    PNMImage pnm;
    PNMReader *reader = pnm.make_reader
      (&in, false, fullpath, identifier, NULL, false);
    if (reader == (PNMReader *)NULL) {
      grutil_cat.error()
        << "Not a PFM file or known image file type: " << fullpath << "\n";
      return false;
    }

    if (!pnm.read(reader)) {
      grutil_cat.error()
        << "Invalid image file: " << fullpath << "\n";
      return false;
    }

    return load(pnm);
  }

  int width, height;
  PN_stdfloat scale;
  in >> width >> height >> scale;
  if (!in) {
    grutil_cat.error()
      << "Error parsing PFM header: " << fullpath << "\n";
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
//
//               This can also handle writing a standard image file
//               supported by PNMImage, if the filename extension is
//               some image type's extension fother than "pfm"; it
//               will be quietly converted to the appropriate integer
//               type.
////////////////////////////////////////////////////////////////////
bool PfmFile::
write(const Filename &fullpath) {
  if (!is_valid()) {
    grutil_cat.error()
      << "PFM file is invalid.\n";
    return false;
  }

  Filename filename = Filename::binary_filename(fullpath);
  pofstream out;
  if (!filename.open_write(out)) {
    grutil_cat.error()
      << "Unable to open " << filename << "\n";
    return false;
  }

  string extension = downcase(fullpath.get_extension());
  if (extension != "pfm") {
    // Maybe we're trying to write a different kind of image file.
    PNMImage pnm;
    PNMWriter *writer = pnm.make_writer(&out, false, fullpath, NULL);
    if (writer != (PNMWriter *)NULL) {
      // Yep.
      if (store(pnm)) {
        return pnm.write(writer);
      }
      // Couldn't make an image.  Carry on directly.
      delete writer;
    }
  }
  
  if (grutil_cat.is_debug()) {
    grutil_cat.debug()
      << "Writing PFM file " << filename << "\n";
  }

  return write(out, fullpath);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::write
//       Access: Published
//  Description: Writes the PFM data to the indicated stream,
//               returning true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool PfmFile::
write(ostream &out, const Filename &fullpath) {
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
//     Function: PfmFile::load
//       Access: Published
//  Description: Fills the PfmFile with the data from the indicated
//               PNMImage, converted to floating-point values.
////////////////////////////////////////////////////////////////////
bool PfmFile::
load(const PNMImage &pnmimage) {
  if (!pnmimage.is_valid()) {
    clear();
    return false;
  }

  // Get the number of channels, ignoring alpha.
  int num_channels;
  if (pnmimage.get_num_channels() >= 3) {
    num_channels = 3;
  } else {
    num_channels = 1;
  }

  clear(pnmimage.get_x_size(), pnmimage.get_y_size(), num_channels);
  if (num_channels == 1) {
    for (int yi = 0; yi < pnmimage.get_y_size(); ++yi) {
      for (int xi = 0; xi < pnmimage.get_x_size(); ++xi) {
        double gray = pnmimage.get_gray(xi, yi);
        set_point(xi, yi, LVecBase3(gray, gray, gray));
      }
    }
  } else {
    for (int yi = 0; yi < pnmimage.get_y_size(); ++yi) {
      for (int xi = 0; xi < pnmimage.get_x_size(); ++xi) {
        LRGBColord xel = pnmimage.get_xel(xi, yi);
        set_point(xi, yi, LVecBase3(xel[0], xel[1], xel[2]));
      }
    }
  }
  return true;
}


////////////////////////////////////////////////////////////////////
//     Function: PfmFile::store
//       Access: Published
//  Description: Copies the data to the indicated PNMImage, converting
//               to RGB values.
////////////////////////////////////////////////////////////////////
bool PfmFile::
store(PNMImage &pnmimage) const {
  if (!is_valid()) {
    pnmimage.clear();
    return false;
  }

  int num_channels = get_num_channels();
  pnmimage.clear(get_x_size(), get_y_size(), num_channels, PGM_MAXMAXVAL);
  if (num_channels == 1) {
    for (int yi = 0; yi < get_y_size(); ++yi) {
      for (int xi = 0; xi < get_x_size(); ++xi) {
        LPoint3 point = get_point(xi, yi);
        pnmimage.set_gray(xi, yi, point[0]);
      }
    }
  } else {
    for (int yi = 0; yi < get_y_size(); ++yi) {
      for (int xi = 0; xi < get_x_size(); ++xi) {
        LPoint3 point = get_point(xi, yi);
        pnmimage.set_xel(xi, yi, point[0], point[1], point[2]);
      }
    }
  }
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
calc_average_point(LPoint3 &result, PN_stdfloat x, PN_stdfloat y, PN_stdfloat radius) const {
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
      if (_has_no_data_value && p == _no_data_value) {
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
    if (_has_no_data_value && p == _no_data_value) {
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
//     Function: PfmFile::calc_autocrop
//       Access: Published
//  Description: Computes the minimum range of x and y across the PFM
//               file that include all points.  If there are no points
//               with no_data_value in the grid--that is, all points
//               are included--then this will return (0, get_x_size(),
//               0, get_y_size()).
////////////////////////////////////////////////////////////////////
bool PfmFile::
calc_autocrop(int &x_begin, int &x_end, int &y_begin, int &y_end) const {
  y_begin = 0;
  while (is_row_empty(y_begin, 0, _x_size)) {
    ++y_begin;
    if (y_begin >= _y_size) {
      // We've reached the end; the entire grid is empty.
      x_begin = x_end = y_begin = y_end = 0;
      return false;
    }
  }

  y_end = _y_size;
  while (is_row_empty(y_end - 1, 0, _x_size)) {
    --y_end;
    nassertr(y_end > y_begin, false);
  }

  // Now we've got the top and bottom bounds.
  x_begin = 0;
  while (is_column_empty(x_begin, y_begin, y_end)) {
    ++x_begin;
    nassertr(x_begin < _x_size, false);
  }

  x_end = _x_size;
  while (is_column_empty(x_end - 1, y_begin, y_end)) {
    --x_end;
    nassertr(x_end > x_begin, false);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::is_row_empty
//       Access: Published
//  Description: Returns true if all of the points on row y, in the range
//               [x_begin, x_end), are the no_data value, or false if
//               any one of these points has a value.
////////////////////////////////////////////////////////////////////
bool PfmFile::
is_row_empty(int y, int x_begin, int x_end) const {
  nassertr(y >= 0 && y < _y_size && 
           x_begin >= 0 && x_begin <= x_end && x_end <= _x_size, false);

  if (!_has_no_data_value) {
    return false;
  }
  for (int x = x_begin; x < x_end; ++x) {
    if (_table[y * _x_size + x] != _no_data_value) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::is_column_empty
//       Access: Published
//  Description: Returns true if all of the points on column x, from
//               [y_begin, y_end), are the no_data value, or false if
//               any one of these points has a value.
////////////////////////////////////////////////////////////////////
bool PfmFile::
is_column_empty(int x, int y_begin, int y_end) const {
  nassertr(x >= 0 && x < _x_size && 
           y_begin >= 0 && y_begin <= y_end && y_end <= _y_size, false);

  if (!_has_no_data_value) {
    return false;
  }
  for (int y = y_begin; y < y_end; ++y) {
    if (_table[y * _x_size + x] != _no_data_value) {
      return false;
    }
  }

  return true;
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

  PN_stdfloat from_x0, from_x1, from_y0, from_y1;

  PN_stdfloat x_scale = 1.0;
  PN_stdfloat y_scale = 1.0;

  if (new_x_size > 1) {
    x_scale = (PN_stdfloat)(_x_size - 1) / (PN_stdfloat)(new_x_size - 1);
  }
  if (new_y_size > 1) {
    y_scale = (PN_stdfloat)(_y_size - 1) / (PN_stdfloat)(new_y_size - 1);
  }

  from_y0 = 0.0;
  for (int to_y = 0; to_y < new_y_size; ++to_y) {
    from_y1 = (to_y + 0.5) * y_scale;
    from_y1 = min(from_y1, (PN_stdfloat) _y_size);

    from_x0 = 0.0;
    for (int to_x = 0; to_x < new_x_size; ++to_x) {
      from_x1 = (to_x + 0.5) * x_scale;
      from_x1 = min(from_x1, (PN_stdfloat) _x_size);

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
//     Function: PfmFile::flip
//       Access: Published
//  Description: Reverses, transposes, and/or rotates the table
//               in-place according to the specified parameters.  If
//               flip_x is true, the x axis is reversed; if flip_y is
//               true, the y axis is reversed.  Then, if transpose is
//               true, the x and y axes are exchanged.  These
//               parameters can be used to select any combination of
//               90-degree or 180-degree rotations and flips.
////////////////////////////////////////////////////////////////////
void PfmFile::
flip(bool flip_x, bool flip_y, bool transpose) {
  nassertv(is_valid());

  Table flipped;
  flipped.reserve(_table.size());

  if (transpose) {
    // Transposed case.  X becomes Y, Y becomes X.
    for (int xi = 0; xi < _x_size; ++xi) {
      int source_xi = flip_x ? xi : _x_size - 1 - xi;
      for (int yi = 0; yi < _y_size; ++yi) {
        int source_yi = flip_y ? yi : _y_size - 1 - yi;
        const LPoint3 &p = _table[source_yi * _x_size + source_xi];
        flipped.push_back(p);
      }
    }

    int t = _x_size;
    _x_size = _y_size;
    _y_size = t;

  } else {
    // Non-transposed.  X is X, Y is Y.
    for (int yi = 0; yi < _y_size; ++yi) {
      int source_yi = flip_y ? yi : _y_size - 1 - yi;
      for (int xi = 0; xi < _x_size; ++xi) {
        int source_xi = flip_x ? xi : _x_size - 1 - xi;
        const LPoint3 &p = _table[source_yi * _x_size + source_xi];
        flipped.push_back(p);
      }
    }
  }

  nassertv(flipped.size() == _table.size());
  _table.swap(flipped);
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
    if (_has_no_data_value && (*ti) == _no_data_value) {
      continue;
    }
    transform.xform_point_general_in_place(*ti);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::project
//       Access: Published
//  Description: Adjusts each (x, y, z) point of the Pfm file by
//               projecting it through the indicated lens, converting
//               each point to a (u, v, w) texture coordinate.  The
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
                        0.0, 0.0, 1.0, 0.0, 
                        0.5, 0.5, 0.0, 1.0);
  
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    if (_has_no_data_value && (*ti) == _no_data_value) {
      continue;
    }

    LPoint3 &p = (*ti);
    LPoint3 film;
    lens->project(p, film);
    p = to_uv.xform_point(film);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::merge
//       Access: Published
//  Description: Wherever there is missing data in this PfmFile (that
//               is, wherever has_point() returns false), copy data
//               from the other PfmFile, which must be exactly the
//               same dimensions as this one.
////////////////////////////////////////////////////////////////////
void PfmFile::
merge(const PfmFile &other) {
  nassertv(is_valid() && other.is_valid());
  nassertv(other._x_size == _x_size && other._y_size == _y_size);

  if (!_has_no_data_value) {
    // Trivial no-op.
    return;
  }

  for (size_t i = 0; i < _table.size(); ++i) {
    if (_table[i] == _no_data_value) {
      _table[i] = other._table[i];
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::apply_crop
//       Access: Published
//  Description: Reduces the PFM file to the cells in the rectangle
//               bounded by (x_begin, x_end, y_begin, y_end), where
//               the _end cells are not included.
////////////////////////////////////////////////////////////////////
void PfmFile::
apply_crop(int x_begin, int x_end, int y_begin, int y_end) {
  nassertv(x_begin >= 0 && x_begin <= x_end && x_end <= _x_size);
  nassertv(y_begin >= 0 && y_begin <= y_end && y_end <= _y_size);

  int new_x_size = x_end - x_begin;
  int new_y_size = y_end - y_begin;
  Table new_table;
  int new_size = new_x_size * new_y_size;
  new_table.insert(new_table.end(), new_size, LPoint3::zero());

  for (int yi = 0; yi < new_y_size; ++yi) {
    memcpy(&new_table[yi * new_x_size],
           &_table[(yi + y_begin) * _x_size + x_begin],
           new_x_size * sizeof(LPoint3));
  }

  _table.swap(new_table);
  _x_size = new_x_size;
  _y_size = new_y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::compute_planar_bounds
//       Access: Published
//  Description: This version of this method exists for temporary
//               backward compatibility only.
////////////////////////////////////////////////////////////////////
PT(BoundingHexahedron) PfmFile::
compute_planar_bounds(PN_stdfloat point_dist, PN_stdfloat sample_radius) const {
  return compute_planar_bounds(LPoint2(0.5, 0.5), point_dist, sample_radius, false);
}


////////////////////////////////////////////////////////////////////
//     Function: PfmFile::compute_planar_bounds
//       Access: Published
//  Description: Computes the minmax bounding volume of the points in
//               3-D space, assuming the points represent a
//               mostly-planar surface.
//
//               This algorithm works by sampling the (square)
//               sample_radius pixels at the four point_dist corners
//               around the center (cx - pd, cx + pd) and so on, to
//               approximate the plane of the surface.  Then all of
//               the points are projected into that plane and the
//               bounding volume of the entire mesh within that plane
//               is determined.  If points_only is true, the bounding
//               volume of only those four points is determined.
//
//               center, point_dist and sample_radius are in UV space,
//               i.e. in the range 0..1.
////////////////////////////////////////////////////////////////////
PT(BoundingHexahedron) PfmFile::
compute_planar_bounds(const LPoint2 &center, PN_stdfloat point_dist, PN_stdfloat sample_radius, bool points_only) const {
  LPoint3 p0, p1, p2, p3;
  compute_sample_point(p0, center[0] + point_dist, center[1] - point_dist, sample_radius);
  compute_sample_point(p1, center[0] + point_dist, center[1] + point_dist, sample_radius);
  compute_sample_point(p2, center[0] - point_dist, center[1] + point_dist, sample_radius);
  compute_sample_point(p3, center[0] - point_dist, center[1] - point_dist, sample_radius);

  LPoint3 normal;

  normal[0] = p0[1] * p1[2] - p0[2] * p1[1];
  normal[1] = p0[2] * p1[0] - p0[0] * p1[2];
  normal[2] = p0[0] * p1[1] - p0[1] * p1[0];

  normal[0] += p1[1] * p2[2] - p1[2] * p2[1];
  normal[1] += p1[2] * p2[0] - p1[0] * p2[2];
  normal[2] += p1[0] * p2[1] - p1[1] * p2[0];

  normal[0] += p2[1] * p3[2] - p2[2] * p3[1];
  normal[1] += p2[2] * p3[0] - p2[0] * p3[2];
  normal[2] += p2[0] * p3[1] - p2[1] * p3[0];

  normal[0] += p3[1] * p0[2] - p3[2] * p0[1];
  normal[1] += p3[2] * p0[0] - p3[0] * p0[2];
  normal[2] += p3[0] * p0[1] - p3[1] * p0[0];

  normal.normalize();

  LVector3 up = (p1 - p0) + (p2 - p3);
  LPoint3 pcenter = ((p0 + p1 + p2 + p3) * 0.25);

  // Compute the transform necessary to rotate all of the points into
  // the Y = 0 plane.
  LMatrix4 rotate;
  look_at(rotate, normal, up);

  LMatrix4 rinv;
  rinv.invert_from(rotate);

  LPoint3 trans = pcenter * rinv;
  rinv.set_row(3, -trans);
  rotate.invert_from(rinv);

  // Now determine the minmax.
  PN_stdfloat min_x, min_y, min_z, max_x, max_y, max_z;
  bool got_point = false;
  if (points_only) {
    LPoint3 points[4] = {
      p0 * rinv,
      p1 * rinv,
      p2 * rinv,
      p3 * rinv,
    };
    for (int i = 0; i < 4; ++i) {
      const LPoint3 &point = points[i];
      if (!got_point) {
        min_x = point[0];
        min_y = point[1];
        min_z = point[2];
        max_x = point[0];
        max_y = point[1];
        max_z = point[2];
        got_point = true;
      } else {
        min_x = min(min_x, point[0]);
        min_y = min(min_y, point[1]);
        min_z = min(min_z, point[2]);
        max_x = max(max_x, point[0]);
        max_y = max(max_y, point[1]);
        max_z = max(max_z, point[2]);
      }
    }

  } else {
    Table::const_iterator ti;
    for (ti = _table.begin(); ti != _table.end(); ++ti) {
      if (_has_no_data_value && (*ti) == _no_data_value) {
        continue;
      }
      LPoint3 point = (*ti) * rinv;
      if (!got_point) {
        min_x = point[0];
        min_y = point[1];
        min_z = point[2];
        max_x = point[0];
        max_y = point[1];
        max_z = point[2];
        got_point = true;
      } else {
        min_x = min(min_x, point[0]);
        min_y = min(min_y, point[1]);
        min_z = min(min_z, point[2]);
        max_x = max(max_x, point[0]);
        max_y = max(max_y, point[1]);
        max_z = max(max_z, point[2]);
      }
    }
  }

  PT(BoundingHexahedron) bounds;
  
  // We create a BoundingHexahedron with the points in a particular
  // well-defined order, based on the current coordinate system.
  CoordinateSystem cs = get_default_coordinate_system();
  switch (cs) {
  case CS_yup_right:
    bounds = new BoundingHexahedron
      (LPoint3(min_x, min_y, min_z), LPoint3(max_x, min_y, min_z),
       LPoint3(min_x, max_y, min_z), LPoint3(max_x, max_y, min_z),
       LPoint3(min_x, min_y, max_z), LPoint3(max_x, min_y, max_z),
       LPoint3(min_x, max_y, max_z), LPoint3(max_x, max_y, max_z));
    break;

  case CS_zup_right:
    bounds = new BoundingHexahedron
      (LPoint3(min_x, min_y, min_z), LPoint3(max_x, min_y, min_z),
       LPoint3(min_x, min_y, max_z), LPoint3(max_x, min_y, max_z),
       LPoint3(min_x, max_y, min_z), LPoint3(max_x, max_y, min_z),
       LPoint3(min_x, max_y, max_z), LPoint3(max_x, max_y, max_z));
    break;

  case CS_yup_left:
    bounds = new BoundingHexahedron
      (LPoint3(max_x, min_y, max_z), LPoint3(min_x, min_y, max_z),
       LPoint3(max_x, max_y, max_z), LPoint3(min_x, max_y, max_z),
       LPoint3(max_x, min_y, min_z), LPoint3(min_x, min_y, min_z),
       LPoint3(max_x, max_y, min_z), LPoint3(min_x, max_y, min_z));
    break;

  case CS_zup_left:
    bounds = new BoundingHexahedron
      (LPoint3(max_x, max_y, min_z), LPoint3(min_x, max_y, min_z),
       LPoint3(max_x, max_y, max_z), LPoint3(min_x, max_y, max_z),
       LPoint3(max_x, min_y, min_z), LPoint3(min_x, min_y, min_z),
       LPoint3(max_x, min_y, max_z), LPoint3(min_x, min_y, max_z));
    break;
  }

  // Rotate the bounding volume back into the original space of the
  // screen.
  bounds->xform(rotate);

  return bounds;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::compute_sample_point
//       Access: Published
//  Description: Computes the average of all the point within
//               sample_radius (manhattan distance) and the indicated
//               point.
//
//               The point coordinates are given in UV space, in the
//               range 0..1.
////////////////////////////////////////////////////////////////////
void PfmFile::
compute_sample_point(LPoint3 &result,
                     PN_stdfloat x, PN_stdfloat y, PN_stdfloat sample_radius) const {
  x *= _x_size;
  y *= _y_size;
  PN_stdfloat xr = sample_radius * _x_size;
  PN_stdfloat yr = sample_radius * _y_size;
  box_filter_region(result, x - xr, y - yr, x + xr, y + yr);
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

  int num_points = 0;
  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      const LPoint3 &point = get_point(xi, yi);
      if (_has_no_data_value && point == _no_data_value) {
        continue;
      }

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
      ++num_points;
    }
  }
  
  PT(Geom) geom = new Geom(vdata);
  PT(GeomPoints) points = new GeomPoints(Geom::UH_static);
  points->add_next_vertices(num_points);
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

  bool reverse_normals = inverted;
  bool reverse_faces = inverted;
  if (!is_right_handed(get_default_coordinate_system())) {
    reverse_faces = !reverse_faces;
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

  PT(GeomVertexArrayFormat) array_format;

  if (_vis_2d) {
    // No normals needed if we're just generating a 2-d mesh.
    array_format = new GeomVertexArrayFormat
      (InternalName::get_vertex(), 3, Geom::NT_stdfloat, Geom::C_point,
       InternalName::get_texcoord(), 2, Geom::NT_stdfloat, Geom::C_texcoord);

  } else {
    if (_vis_inverse) {
      // We need a 3-d texture coordinate if we're inverting the vis
      // and it's 3-d.  But we still don't need normals in that case.
      array_format = new GeomVertexArrayFormat
        (InternalName::get_vertex(), 3, Geom::NT_stdfloat, Geom::C_point,
         InternalName::get_texcoord(), 3, Geom::NT_stdfloat, Geom::C_texcoord);
    } else {
      // Otherwise, we only need a 2-d texture coordinate, and we do
      // want normals.
      array_format = new GeomVertexArrayFormat
        (InternalName::get_vertex(), 3, Geom::NT_stdfloat, Geom::C_point,
         InternalName::get_normal(), 3, Geom::NT_stdfloat, Geom::C_vector,
         InternalName::get_texcoord(), 2, Geom::NT_stdfloat, Geom::C_texcoord);
    }
  }

  if (_flat_texcoord_name != (InternalName *)NULL) {
    // We need an additional texcoord column for the flat texcoords.
    array_format->add_column(_flat_texcoord_name, 2, 
                             Geom::NT_stdfloat, Geom::C_texcoord);
  }

  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format(array_format);

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
      GeomVertexWriter texcoord2(vdata, _flat_texcoord_name);

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
            if (reverse_normals) {
              n = -n;
            }
            normal.add_data3(n);
          }

          if (_flat_texcoord_name != (InternalName *)NULL) {
            texcoord2.add_data2(uv);
          }
        }
      }
  
      PT(Geom) geom = new Geom(vdata);
      PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);

      tris->reserve_num_vertices(max_indices);

      for (int yi = y_begin; yi < y_end - 1; ++yi) {
        for (int xi = x_begin; xi < x_end - 1; ++xi) {

          if (_has_no_data_value) {
            if (get_point(xi, yi) == _no_data_value ||
                get_point(xi, yi + 1) == _no_data_value ||
                get_point(xi + 1, yi + 1) == _no_data_value ||
                get_point(xi + 1, yi) == _no_data_value) {
              continue;
            }
          }

          int xi0 = xi - x_begin;
          int yi0 = yi - y_begin;

          int vi0 = ((xi0) + (yi0) * x_size);
          int vi1 = ((xi0) + (yi0 + 1) * x_size);
          int vi2 = ((xi0 + 1) + (yi0 + 1) * x_size);
          int vi3 = ((xi0 + 1) + (yi0) * x_size);
          
          if (reverse_faces) {
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
                  PN_stdfloat x0, PN_stdfloat y0, PN_stdfloat x1, PN_stdfloat y1) const {
  result = LPoint3::zero();
  PN_stdfloat coverage = 0.0;

  if (x1 < x0 || y1 < y0) {
    return;
  }
  nassertv(y0 >= 0.0 && y1 >= 0.0);

  int y = (int)y0;
  // Get the first (partial) row
  box_filter_line(result, coverage, x0, y, x1, (PN_stdfloat)(y+1)-y0);

  int y_last = (int)y1;
  if (y < y_last) {
    y++;
    while (y < y_last) {
      // Get each consecutive (complete) row
      box_filter_line(result, coverage, x0, y, x1, 1.0);
      y++;
    }

    // Get the final (partial) row
    PN_stdfloat y_contrib = y1 - (PN_stdfloat)y_last;
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
box_filter_line(LPoint3 &result, PN_stdfloat &coverage,
                PN_stdfloat x0, int y, PN_stdfloat x1, PN_stdfloat y_contrib) const {
  int x = (int)x0;
  // Get the first (partial) xel
  box_filter_point(result, coverage, x, y, (PN_stdfloat)(x+1)-x0, y_contrib);

  int x_last = (int)x1;
  if (x < x_last) {
    x++;
    while (x < x_last) {
      // Get each consecutive (complete) xel
      box_filter_point(result, coverage, x, y, 1.0, y_contrib);
      x++;
    }

    // Get the final (partial) xel
    PN_stdfloat x_contrib = x1 - (PN_stdfloat)x_last;
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
box_filter_point(LPoint3 &result, PN_stdfloat &coverage,
                 int x, int y, PN_stdfloat x_contrib, PN_stdfloat y_contrib) const {
  const LPoint3 &point = get_point(x, y);
  if (_has_no_data_value && point == _no_data_value) {
    return;
  }

  PN_stdfloat contrib = x_contrib * y_contrib;
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
