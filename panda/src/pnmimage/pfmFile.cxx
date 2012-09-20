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

#include "config_pnmimage.h"
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
#include "pnmReader.h"
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
  _no_data_value = LPoint4f::zero();
  _has_point = has_point_noop;
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
  PNMImageHeader(copy),
  _table(copy._table),
  _scale(copy._scale),
  _has_no_data_value(copy._has_no_data_value),
  _no_data_value(copy._no_data_value),
  _has_point(copy._has_point),
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
  PNMImageHeader::operator = (copy);
  _table = copy._table;
  _scale = copy._scale;
  _has_no_data_value = copy._has_no_data_value;
  _no_data_value = copy._no_data_value;
  _has_point = copy._has_point;
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
  clear_no_data_value();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::clear
//       Access: Published
//  Description: Resets to an empty table with a specific size.
////////////////////////////////////////////////////////////////////
void PfmFile::
clear(int x_size, int y_size, int num_channels) {
  nassertv(x_size >= 0 && y_size >= 0);
  _x_size = x_size;
  _y_size = y_size;
  _scale = 1.0;
  _num_channels = num_channels;

  _table.clear();
  int size = _x_size * _y_size * _num_channels;

  // We allocate a little bit bigger to allow safe overflow: you can
  // call get_point3() or get_point4() on the last point of a 1- or
  // 3-channel image.
  _table.insert(_table.end(), size + 4, (PN_float32)0.0);

  clear_no_data_value();
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
    pnmimage_cat.error()
      << "Could not find " << fullpath << "\n";
    return false;
  }

  if (pnmimage_cat.is_debug()) {
    pnmimage_cat.debug()
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
read(istream &in, const Filename &fullpath) {
  PNMReader *reader = make_reader(&in, false, fullpath);
  if (reader == (PNMReader *)NULL) {
    clear();
    return false;
  }
  return read(reader);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::read
//       Access: Published
//  Description: Reads the PFM data using the indicated PNMReader.
//
//               The PNMReader is always deleted upon completion,
//               whether successful or not.
////////////////////////////////////////////////////////////////////
bool PfmFile::
read(PNMReader *reader) {
  clear();

  if (reader == NULL) {
    return false;
  }

  if (!reader->is_valid()) {
    delete reader;
    return false;
  }

  if (!reader->is_floating_point()) {
    // Not a floating-point file.  Quietly convert it.
    PNMImage pnm;
    if (!pnm.read(reader)) {
      return false;
    }

    return load(pnm);
  }

  bool success = reader->read_pfm(*this);
  delete reader;
  return success;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::write
//       Access: Published
//  Description: Writes the PFM data to the indicated file, returning
//               true on success, false on failure.
//
//               This can also handle writing a standard image file
//               supported by PNMImage, if the filename extension is
//               some image type's extension other than "pfm"; it
//               will be quietly converted to the appropriate integer
//               type.
////////////////////////////////////////////////////////////////////
bool PfmFile::
write(const Filename &fullpath) {
  if (!is_valid()) {
    pnmimage_cat.error()
      << "PFM file is invalid.\n";
    return false;
  }

  Filename filename = Filename::binary_filename(fullpath);
  pofstream out;
  if (!filename.open_write(out)) {
    pnmimage_cat.error()
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
  
  if (pnmimage_cat.is_debug()) {
    pnmimage_cat.debug()
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
  if (!is_valid()) {
    return false;
  }

  PNMWriter *writer = make_writer(fullpath);
  if (writer == (PNMWriter *)NULL) {
    return false;
  }

  return write(writer);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::write
//       Access: Published
//  Description: Writes the PFM data using the indicated PNMWriter.
//
//               The PNMWriter is always deleted upon completion,
//               whether successful or not.
////////////////////////////////////////////////////////////////////
bool PfmFile::
write(PNMWriter *writer) {
  if (writer == NULL) {
    return false;
  }

  if (!is_valid()) {
    delete writer;
    return false;
  }

  writer->copy_header_from(*this);

  if (!writer->supports_floating_point()) {
    // Hmm, it's an integer file type.  Convert it from the
    // floating-point data we have.
    PNMImage pnmimage;
    if (!store(pnmimage)) {
      delete writer;
      return false;
    }
    writer->copy_header_from(pnmimage);
    bool success = (writer->write_data(pnmimage.get_array(), pnmimage.get_alpha_array()) != 0);
    delete writer;
    return success;
  }

  bool success = writer->write_pfm(*this);
  delete writer;
  return success;
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
  switch (pnmimage.get_num_channels()) {
  case 4:
    num_channels = 4;
    break;

  case 3:
    num_channels = 3;
    break;

  case 2:
  case 1:
    num_channels = 1;
    break;

  default:
    num_channels = 3;
  }

  clear(pnmimage.get_x_size(), pnmimage.get_y_size(), num_channels);
  switch (num_channels) {
  case 1:
    {
      for (int yi = 0; yi < pnmimage.get_y_size(); ++yi) {
        for (int xi = 0; xi < pnmimage.get_x_size(); ++xi) {
          _table[yi * _x_size + xi] = pnmimage.get_gray(xi, yi);
        }
      }
    }
    break;

  case 3:
    {
      for (int yi = 0; yi < pnmimage.get_y_size(); ++yi) {
        for (int xi = 0; xi < pnmimage.get_x_size(); ++xi) {
          PN_float32 *point = &_table[(yi * _x_size + xi) * _num_channels];
          LRGBColord xel = pnmimage.get_xel(xi, yi);
          point[0] = xel[0];
          point[1] = xel[1];
          point[2] = xel[2];
        }
      }
    }
    break;

  case 4:
    {
      for (int yi = 0; yi < pnmimage.get_y_size(); ++yi) {
        for (int xi = 0; xi < pnmimage.get_x_size(); ++xi) {
          PN_float32 *point = &_table[(yi * _x_size + xi) * _num_channels];
          LRGBColord xel = pnmimage.get_xel(xi, yi);
          point[0] = xel[0];
          point[1] = xel[1];
          point[2] = xel[2];
          point[3] = pnmimage.get_alpha(xi, yi);
        }
      }
    }
    break;

  default:
    nassertr(false, false);
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
  switch (num_channels) {
  case 1:
    {
      for (int yi = 0; yi < get_y_size(); ++yi) {
        for (int xi = 0; xi < get_x_size(); ++xi) {
          pnmimage.set_gray(xi, yi, _table[yi * _x_size + xi]);
        }
      }
    }
    break;

  case 3:
    {
      for (int yi = 0; yi < get_y_size(); ++yi) {
        for (int xi = 0; xi < get_x_size(); ++xi) {
          const LPoint3f &point = get_point(xi, yi);
          pnmimage.set_xel(xi, yi, point[0], point[1], point[2]);
        }
      }
    }
    break;

  case 4:
    {
      for (int yi = 0; yi < get_y_size(); ++yi) {
        for (int xi = 0; xi < get_x_size(); ++xi) {
          const LPoint4f &point = get_point4(xi, yi);
          pnmimage.set_xel(xi, yi, point[0], point[1], point[2]);
          pnmimage.set_alpha(xi, yi, point[3]);
        }
      }
    }
    break;

  default:
    nassertr(false, false);
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
calc_average_point(LPoint3f &result, PN_float32 x, PN_float32 y, PN_float32 radius) const {
  result = LPoint3f::zero();

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
      if (!has_point(xi, yi)) {
        continue;
      }

      const LPoint3f &p = get_point(xi, yi);
      int gi = (yi - min_y) * y_size + (xi - min_x);
      nassertr(gi >= 0 && gi < size, false);
      mini_grid[gi]._sxi = xi;
      mini_grid[gi]._syi = yi;
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
        int sxi = mini_grid[gi]._sxi;
        int syi = mini_grid[gi]._syi;
        fill_mini_grid(&mini_grid[0], x_size, y_size, xi + 1, yi, 1, sxi, syi);
        fill_mini_grid(&mini_grid[0], x_size, y_size, xi - 1, yi, 1, sxi, syi);
        fill_mini_grid(&mini_grid[0], x_size, y_size, xi, yi + 1, 1, sxi, syi);
        fill_mini_grid(&mini_grid[0], x_size, y_size, xi, yi - 1, 1, sxi, syi);
      }
    }
  }

  // Now the mini-grid is completely filled, so we can compute the
  // average.
  for (int gi = 0; gi < size; ++gi) {
    int sxi = mini_grid[gi]._sxi;
    int syi = mini_grid[gi]._syi;
    const LPoint3f &p = get_point(sxi, syi);
    result += p;
  }

  result /= PN_float32(size);
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
calc_min_max(LVecBase3f &min_depth, LVecBase3f &max_depth) const {
  bool any_points = false;

  min_depth = LVecBase3f::zero();
  max_depth = LVecBase3f::zero();

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      if (!has_point(xi, yi)) {
        continue;
      }
      
      const LPoint3f &p = get_point(xi, yi);
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
    if (has_point(x, y)) {
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
    if (has_point(x, y)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::set_no_data_value
//       Access: Published
//  Description: Sets the special value that means "no data" when it
//               appears in the pfm file.
////////////////////////////////////////////////////////////////////
void PfmFile::
set_no_data_value(const LPoint4f &no_data_value) {
  _has_no_data_value = true;
  _no_data_value = no_data_value;
  switch (_num_channels) {
  case 1:
    _has_point = has_point_1;
    break;
  case 3:
    _has_point = has_point_3;
    break;
  case 4:
    _has_point = has_point_4;
    break;
  default:
    nassertv(false);
  }
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

  int new_size = new_x_size * new_y_size * _num_channels;

  // We allocate a little bit bigger to allow safe overflow.
  Table new_data;
  new_data.reserve(new_size + 4);

  PN_float32 from_x0, from_x1, from_y0, from_y1;

  PN_float32 x_scale = 1.0;
  PN_float32 y_scale = 1.0;

  if (new_x_size > 1) {
    x_scale = (PN_float32)(_x_size - 1) / (PN_float32)(new_x_size - 1);
  }
  if (new_y_size > 1) {
    y_scale = (PN_float32)(_y_size - 1) / (PN_float32)(new_y_size - 1);
  }

  switch (_num_channels) {
  case 1:
    {
      from_y0 = 0.0;
      for (int to_y = 0; to_y < new_y_size; ++to_y) {
        from_y1 = (to_y + 0.5) * y_scale;
        from_y1 = min(from_y1, (PN_float32) _y_size);
        
        from_x0 = 0.0;
        for (int to_x = 0; to_x < new_x_size; ++to_x) {
          from_x1 = (to_x + 0.5) * x_scale;
          from_x1 = min(from_x1, (PN_float32) _x_size);
          
          // Now the box from (from_x0, from_y0) - (from_x1, from_y1)
          // but not including (from_x1, from_y1) maps to the pixel (to_x, to_y).
          PN_float32 result;
          box_filter_region(result, from_x0, from_y0, from_x1, from_y1);
          new_data.push_back(result);
          
          from_x0 = from_x1;
        }
        from_y0 = from_y1;
      }
    }
    break;

  case 3:
    {
      from_y0 = 0.0;
      for (int to_y = 0; to_y < new_y_size; ++to_y) {
        from_y1 = (to_y + 0.5) * y_scale;
        from_y1 = min(from_y1, (PN_float32) _y_size);
        
        from_x0 = 0.0;
        for (int to_x = 0; to_x < new_x_size; ++to_x) {
          from_x1 = (to_x + 0.5) * x_scale;
          from_x1 = min(from_x1, (PN_float32) _x_size);
          
          // Now the box from (from_x0, from_y0) - (from_x1, from_y1)
          // but not including (from_x1, from_y1) maps to the pixel (to_x, to_y).
          LPoint3f result;
          box_filter_region(result, from_x0, from_y0, from_x1, from_y1);
          new_data.push_back(result[0]);
          new_data.push_back(result[1]);
          new_data.push_back(result[2]);
          
          from_x0 = from_x1;
        }
        from_y0 = from_y1;
      }
    }
    break;

  case 4:
    {
      from_y0 = 0.0;
      for (int to_y = 0; to_y < new_y_size; ++to_y) {
        from_y1 = (to_y + 0.5) * y_scale;
        from_y1 = min(from_y1, (PN_float32) _y_size);
        
        from_x0 = 0.0;
        for (int to_x = 0; to_x < new_x_size; ++to_x) {
          from_x1 = (to_x + 0.5) * x_scale;
          from_x1 = min(from_x1, (PN_float32) _x_size);
          
          // Now the box from (from_x0, from_y0) - (from_x1, from_y1)
          // but not including (from_x1, from_y1) maps to the pixel (to_x, to_y).
          LPoint4f result;
          box_filter_region(result, from_x0, from_y0, from_x1, from_y1);
          new_data.push_back(result[0]);
          new_data.push_back(result[1]);
          new_data.push_back(result[2]);
          new_data.push_back(result[3]);
          
          from_x0 = from_x1;
        }
        from_y0 = from_y1;
      }
    }
    break;
    
  default:
    nassertv(false);
  }

  new_data.push_back(0.0);
  new_data.push_back(0.0);
  new_data.push_back(0.0);
  new_data.push_back(0.0);

  nassertv(new_data.size() == new_size + 4);
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
  int row_size = _x_size * _num_channels;
  for (int yi = 0; yi < _y_size; ++yi) {
    int source_yi = _y_size - 1 - yi;
    int start = source_yi * row_size;
    reversed.insert(reversed.end(), 
                    _table.begin() + start, _table.begin() + start + row_size);
  }

  nassertv(reversed.size() <= _table.size());
  // Also add in the extra buffer at the end.
  reversed.insert(reversed.end(), _table.size() - reversed.size(), (PN_float32)0.0);

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
      int source_xi = !flip_x ? xi : _x_size - 1 - xi;
      for (int yi = 0; yi < _y_size; ++yi) {
        int source_yi = !flip_y ? yi : _y_size - 1 - yi;
        const PN_float32 *p = &_table[(source_yi * _x_size + source_xi) * _num_channels];
        for (int ci = 0; ci < _num_channels; ++ci) {
          flipped.push_back(p[ci]);
        }
      }
    }

    int t = _x_size;
    _x_size = _y_size;
    _y_size = t;

  } else {
    // Non-transposed.  X is X, Y is Y.
    for (int yi = 0; yi < _y_size; ++yi) {
      int source_yi = !flip_y ? yi : _y_size - 1 - yi;
      for (int xi = 0; xi < _x_size; ++xi) {
        int source_xi = !flip_x ? xi : _x_size - 1 - xi;
        const PN_float32 *p = &_table[(source_yi * _x_size + source_xi) * _num_channels];
        for (int ci = 0; ci < _num_channels; ++ci) {
          flipped.push_back(p[ci]);
        }
      }
    }
  }

  nassertv(flipped.size() <= _table.size());
  // Also add in the extra buffer at the end.
  flipped.insert(flipped.end(), _table.size() - flipped.size(), (PN_float32)0.0);

  _table.swap(flipped);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::xform
//       Access: Published
//  Description: Applies the indicated transform matrix to all points
//               in-place.
////////////////////////////////////////////////////////////////////
void PfmFile::
xform(const LMatrix4f &transform) {
  nassertv(is_valid());

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      if (!has_point(xi, yi)) {
        continue;
      }
      LPoint3f &p = modify_point(xi, yi);
      transform.xform_point_general_in_place(p);
    }
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

  static LMatrix4f to_uv(0.5f, 0.0f, 0.0f, 0.0f,
                         0.0f, 0.5f, 0.0f, 0.0f, 
                         0.0f, 0.0f, 0.5f, 0.0f, 
                         0.5f, 0.5f, 0.5f, 1.0f);
  
  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      if (!has_point(xi, yi)) {
        continue;
      }
      LPoint3f &p = modify_point(xi, yi);

      LPoint3 film;
      lens->project(LCAST(PN_stdfloat, p), film);
      p = to_uv.xform_point(LCAST(PN_float32, film));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::extrude
//       Access: Published
//  Description: Converts each (u, v, depth) point of the Pfm file to
//               an (x, y, z) point, by reversing project().  If the
//               original file is only a 1-d file, assumes that it is
//               a depth map with implicit (u, v) coordinates.
////////////////////////////////////////////////////////////////////
void PfmFile::
extrude(const Lens *lens) {
  nassertv(is_valid());

  static LMatrix4 from_uv(2.0, 0.0, 0.0, 0.0,
                          0.0, 2.0, 0.0, 0.0,
                          0.0, 0.0, 2.0, 0.0,
                          -1.0, -1.0, -1.0, 1.0);

  PfmFile result;
  result.clear(_x_size, _y_size, 3);
  result.set_zero_special(true);

  LPoint2 uv_scale(1.0, 1.0);
  if (_x_size > 1) {
    uv_scale[0] = 1.0 / PN_stdfloat(_x_size - 1);
  }
  if (_y_size > 1) {
    uv_scale[1] = 1.0 / PN_stdfloat(_y_size - 1);
  }

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      if (!has_point(xi, yi)) {
        continue;
      }
      LPoint3 p;
      if (_num_channels == 1) {
        p.set((PN_stdfloat)xi * uv_scale[0],
              (PN_stdfloat)yi * uv_scale[1],
              (PN_stdfloat)get_point1(xi, yi));
      } else {
        p = LCAST(PN_stdfloat, get_point(xi, yi));
      }

      if (lens->is_linear()) {
        lens->get_projection_mat_inv().xform_point_general_in_place(p);
        result.set_point(xi, yi, p);

      } else {
        from_uv.xform_point_in_place(p);
        LPoint3 near_point, far_point;
        if (!lens->extrude(p, near_point, far_point)) {
          continue;
        }
        
        LPoint3 film = near_point + (far_point - near_point) * p[2];
        result.set_point(xi, yi, film);
      }
    }
  }

  (*this) = result;
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

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      if (!has_point(xi, yi)) {
        set_point(xi, yi, other.get_point(xi, yi));
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::copy_channel
//       Access: Published
//  Description: Copies just the specified channel values from the
//               indicated PfmFile (which could be same as this
//               PfmFile) into the specified channel of this one.
////////////////////////////////////////////////////////////////////
void PfmFile::
copy_channel(int to_channel, const PfmFile &other, int from_channel) {
  nassertv(is_valid() && other.is_valid());
  nassertv(other._x_size == _x_size && other._y_size == _y_size);
  nassertv(to_channel >= 0 && to_channel < get_num_channels() &&
           from_channel >= 0 && from_channel < other.get_num_channels());

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      set_component(xi, yi, to_channel, other.get_component(xi, yi, from_channel));
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
  int new_size = new_x_size * new_y_size * _num_channels;

  // We allocate a little bit bigger to allow safe overflow: you can
  // call get_point3() or get_point4() on the last point of a 1- or
  // 3-channel image.
  new_table.insert(new_table.end(), new_size + 4, (PN_float32)0.0);

  for (int yi = 0; yi < new_y_size; ++yi) {
    memcpy(&new_table[(yi * new_x_size) * _num_channels],
           &_table[((yi + y_begin) * _x_size + x_begin) * _num_channels],
           new_x_size * sizeof(PN_float32) * _num_channels);
  }

  nassertv(new_table.size() == new_size + 4);
  _table.swap(new_table);
  _x_size = new_x_size;
  _y_size = new_y_size;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::clear_to_texcoords
//       Access: Published
//  Description: Replaces this PfmFile with a new PfmFile of size
//               x_size x y_size x 3, containing the x y 0 values in
//               the range 0 .. 1 according to the x y index.
////////////////////////////////////////////////////////////////////
void PfmFile::
clear_to_texcoords(int x_size, int y_size) {
  clear(x_size, y_size, 3);

  LPoint2f uv_scale(1.0, 1.0);
  if (_x_size > 1) {
    uv_scale[0] = 1.0f / PN_float32(_x_size - 1);
  }
  if (_y_size > 1) {
    uv_scale[1] = 1.0f / PN_float32(_y_size - 1);
  }

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      LPoint3f uv(PN_float32(xi) * uv_scale[0],
                  PN_float32(yi) * uv_scale[1], 0.0f);
      set_point(xi, yi, uv);
    }
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
compute_planar_bounds(const LPoint2f &center, PN_float32 point_dist, PN_float32 sample_radius, bool points_only) const {
  LPoint3f p0, p1, p2, p3;
  compute_sample_point(p0, center[0] + point_dist, center[1] - point_dist, sample_radius);
  compute_sample_point(p1, center[0] + point_dist, center[1] + point_dist, sample_radius);
  compute_sample_point(p2, center[0] - point_dist, center[1] + point_dist, sample_radius);
  compute_sample_point(p3, center[0] - point_dist, center[1] - point_dist, sample_radius);

  LPoint3f normal;

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

  LVector3f up = (p1 - p0) + (p2 - p3);
  LPoint3f pcenter = ((p0 + p1 + p2 + p3) * 0.25);

  // Compute the transform necessary to rotate all of the points into
  // the Y = 0 plane.
  LMatrix4f rotate;
  look_at(rotate, normal, up);

  LMatrix4f rinv;
  rinv.invert_from(rotate);

  LPoint3f trans = pcenter * rinv;
  rinv.set_row(3, -trans);
  rotate.invert_from(rinv);

  // Now determine the minmax.
  PN_float32 min_x, min_y, min_z, max_x, max_y, max_z;
  bool got_point = false;
  if (points_only) {
    LPoint3f points[4] = {
      p0 * rinv,
      p1 * rinv,
      p2 * rinv,
      p3 * rinv,
    };
    for (int i = 0; i < 4; ++i) {
      const LPoint3f &point = points[i];
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
    for (int yi = 0; yi < _y_size; ++yi) {
      for (int xi = 0; xi < _x_size; ++xi) {
        if (!has_point(xi, yi)) {
          continue;
        }

        LPoint3f point = get_point(xi, yi) * rinv;
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
  bounds->xform(LCAST(PN_stdfloat, rotate));

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
compute_sample_point(LPoint3f &result,
                     PN_float32 x, PN_float32 y, PN_float32 sample_radius) const {
  x *= _x_size;
  y *= _y_size;
  PN_float32 xr = sample_radius * _x_size;
  PN_float32 yr = sample_radius * _y_size;

  switch (_num_channels) {
  case 1:
    {
      PN_float32 result1;
      box_filter_region(result1, x - xr, y - yr, x + xr, y + yr);
      result.set(result1, 0.0, 0.0);
    }
    break;

  case 3:
    box_filter_region(result, x - xr, y - yr, x + xr, y + yr);
    break;

  case 4:
    {
      LPoint4f result4;
      box_filter_region(result4, x - xr, y - yr, x + xr, y + yr);
      result.set(result4[0], result4[1], result4[2]);
    }
    break;

  default:
    nassertv(false);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::clear_vis_columns
//       Access: Published
//  Description: Removes all of the previously-added vis columns in
//               preparation for building a new list.  See
//               add_vis_column().
////////////////////////////////////////////////////////////////////
void PfmFile::
clear_vis_columns() {
  _vis_columns.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::add_vis_column
//       Access: Published
//  Description: Adds a new vis column specification to the list of
//               vertex data columns that will be generated at the
//               next call to generate_vis_points() or
//               generate_vis_mesh().  This advanced interface
//               supercedes the higher-level set_vis_inverse(),
//               set_flat_texcoord_name(), and set_vis_2d().
//
//               If you use this advanced interface, you must specify
//               explicitly the complete list of data columns to be
//               created in the resulting GeomVertexData, by calling
//               add_vis_column() each time.  For each column, you
//               specify the source of the column in the PFMFile, the
//               target column and name in the GeomVertexData, and an
//               optional transform matrix and/or lens to transform
//               and project the point before generating it.
////////////////////////////////////////////////////////////////////
void PfmFile::
add_vis_column(ColumnType source, ColumnType target,
               InternalName *name, const TransformState *transform,
               const Lens *lens) {
  add_vis_column(_vis_columns, source, target, name, transform, lens);
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

  LPoint2f uv_scale(1.0, 1.0);
  if (_x_size > 1) {
    uv_scale[0] = 1.0f / PN_float32(_x_size - 1);
  }
  if (_y_size > 1) {
    uv_scale[1] = 1.0f / PN_float32(_y_size - 1);
  }

  int num_points = 0;
  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      if (!has_point(xi, yi)) {
        continue;
      }

      const LPoint3f &point = get_point(xi, yi);
      LPoint2f uv(PN_float32(xi) * uv_scale[0],
                  PN_float32(yi) * uv_scale[1]);
      if (_vis_inverse) {
        vertex.add_data2f(uv);
        texcoord.add_data3f(point);
      } else if (_vis_2d) {
        vertex.add_data2f(point[0], point[1]);
        texcoord.add_data2f(uv);
      } else {
        vertex.add_data3f(point);
        texcoord.add_data2f(uv);
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

  if (_num_channels == 1 && _vis_columns.empty()) {
    // If we're generating a default mesh from a one-channel pfm file,
    // expand it to a three-channel pfm file to make the visualization
    // useful.
    PfmFile expanded;
    expanded.clear_to_texcoords(_x_size, _y_size);
    expanded.copy_channel(2, *this, 0);
    return expanded.generate_vis_mesh(face);
  }
  
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
//     Function: PfmFile::output
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
void PfmFile::
output(ostream &out) const {
  out << "floating-point image: " << _x_size << " by " << _y_size << " pixels, "
      << _num_channels << " channels.";
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
  if (pnmimage_cat.is_debug()) {
    pnmimage_cat.debug()
      << "Generating mesh with " << num_x_cells << " x " << num_y_cells
      << " pieces.\n";
  }

  VisColumns vis_columns = _vis_columns;
  if (vis_columns.empty()) {
    build_auto_vis_columns(vis_columns, true);
  }

  CPT(GeomVertexFormat) format = make_array_format(vis_columns);

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

      // Fill in all of the vertices.
      for (VisColumns::const_iterator vci = vis_columns.begin();
           vci != vis_columns.end();
           ++vci) {
        const VisColumn &column = *vci;
        GeomVertexWriter vwriter(vdata, column._name);
        vwriter.set_row(0);

        for (int yi = y_begin; yi < y_end; ++yi) {
          for (int xi = x_begin; xi < x_end; ++xi) {
            column.add_data(*this, vwriter, xi, yi, reverse_normals);
          }
        }
      }
  
      PT(Geom) geom = new Geom(vdata);
      PT(GeomTriangles) tris = new GeomTriangles(Geom::UH_static);

      tris->reserve_num_vertices(max_indices);

      for (int yi = y_begin; yi < y_end - 1; ++yi) {
        for (int xi = x_begin; xi < x_end - 1; ++xi) {

          if (_has_no_data_value) {
            if (!has_point(xi, yi) ||
                !has_point(xi, yi + 1) ||
                !has_point(xi + 1, yi + 1) ||
                !has_point(xi + 1, yi)) {
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
box_filter_region(PN_float32 &result,
                  PN_float32 x0, PN_float32 y0, PN_float32 x1, PN_float32 y1) const {
  result = 0.0;
  PN_float32 coverage = 0.0;

  if (x1 < x0 || y1 < y0) {
    return;
  }
  nassertv(y0 >= 0.0 && y1 >= 0.0);

  int y = (int)y0;
  // Get the first (partial) row
  box_filter_line(result, coverage, x0, y, x1, (PN_float32)(y+1)-y0);

  int y_last = (int)y1;
  if (y < y_last) {
    y++;
    while (y < y_last) {
      // Get each consecutive (complete) row
      box_filter_line(result, coverage, x0, y, x1, 1.0);
      y++;
    }

    // Get the final (partial) row
    PN_float32 y_contrib = y1 - (PN_float32)y_last;
    if (y_contrib > 0.0001) {
      box_filter_line(result, coverage, x0, y, x1, y_contrib);
    }
  }

  if (coverage != 0.0) {
    result /= coverage;
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
box_filter_region(LPoint3f &result,
                  PN_float32 x0, PN_float32 y0, PN_float32 x1, PN_float32 y1) const {
  result = LPoint3f::zero();
  PN_float32 coverage = 0.0;

  if (x1 < x0 || y1 < y0) {
    return;
  }
  nassertv(y0 >= 0.0 && y1 >= 0.0);

  int y = (int)y0;
  // Get the first (partial) row
  box_filter_line(result, coverage, x0, y, x1, (PN_float32)(y+1)-y0);

  int y_last = (int)y1;
  if (y < y_last) {
    y++;
    while (y < y_last) {
      // Get each consecutive (complete) row
      box_filter_line(result, coverage, x0, y, x1, 1.0);
      y++;
    }

    // Get the final (partial) row
    PN_float32 y_contrib = y1 - (PN_float32)y_last;
    if (y_contrib > 0.0001) {
      box_filter_line(result, coverage, x0, y, x1, y_contrib);
    }
  }

  if (coverage != 0.0) {
    result /= coverage;
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
box_filter_region(LPoint4f &result,
                  PN_float32 x0, PN_float32 y0, PN_float32 x1, PN_float32 y1) const {
  result = LPoint4f::zero();
  PN_float32 coverage = 0.0;

  if (x1 < x0 || y1 < y0) {
    return;
  }
  nassertv(y0 >= 0.0 && y1 >= 0.0);

  int y = (int)y0;
  // Get the first (partial) row
  box_filter_line(result, coverage, x0, y, x1, (PN_float32)(y+1)-y0);

  int y_last = (int)y1;
  if (y < y_last) {
    y++;
    while (y < y_last) {
      // Get each consecutive (complete) row
      box_filter_line(result, coverage, x0, y, x1, 1.0);
      y++;
    }

    // Get the final (partial) row
    PN_float32 y_contrib = y1 - (PN_float32)y_last;
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
box_filter_line(PN_float32 &result, PN_float32 &coverage,
                PN_float32 x0, int y, PN_float32 x1, PN_float32 y_contrib) const {
  int x = (int)x0;
  // Get the first (partial) xel
  box_filter_point(result, coverage, x, y, (PN_float32)(x+1)-x0, y_contrib);

  int x_last = (int)x1;
  if (x < x_last) {
    x++;
    while (x < x_last) {
      // Get each consecutive (complete) xel
      box_filter_point(result, coverage, x, y, 1.0, y_contrib);
      x++;
    }

    // Get the final (partial) xel
    PN_float32 x_contrib = x1 - (PN_float32)x_last;
    if (x_contrib > 0.0001) {
      box_filter_point(result, coverage, x, y, x_contrib, y_contrib);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::box_filter_line
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void PfmFile::
box_filter_line(LPoint3f &result, PN_float32 &coverage,
                PN_float32 x0, int y, PN_float32 x1, PN_float32 y_contrib) const {
  int x = (int)x0;
  // Get the first (partial) xel
  box_filter_point(result, coverage, x, y, (PN_float32)(x+1)-x0, y_contrib);

  int x_last = (int)x1;
  if (x < x_last) {
    x++;
    while (x < x_last) {
      // Get each consecutive (complete) xel
      box_filter_point(result, coverage, x, y, 1.0, y_contrib);
      x++;
    }

    // Get the final (partial) xel
    PN_float32 x_contrib = x1 - (PN_float32)x_last;
    if (x_contrib > 0.0001) {
      box_filter_point(result, coverage, x, y, x_contrib, y_contrib);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::box_filter_line
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void PfmFile::
box_filter_line(LPoint4f &result, PN_float32 &coverage,
                PN_float32 x0, int y, PN_float32 x1, PN_float32 y_contrib) const {
  int x = (int)x0;
  // Get the first (partial) xel
  box_filter_point(result, coverage, x, y, (PN_float32)(x+1)-x0, y_contrib);

  int x_last = (int)x1;
  if (x < x_last) {
    x++;
    while (x < x_last) {
      // Get each consecutive (complete) xel
      box_filter_point(result, coverage, x, y, 1.0, y_contrib);
      x++;
    }

    // Get the final (partial) xel
    PN_float32 x_contrib = x1 - (PN_float32)x_last;
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
box_filter_point(PN_float32 &result, PN_float32 &coverage,
                 int x, int y, PN_float32 x_contrib, PN_float32 y_contrib) const {
  if (!has_point(x, y)) {
    return;
  }
  PN_float32 point = get_point1(x, y);

  PN_float32 contrib = x_contrib * y_contrib;
  result += point * contrib;
  coverage += contrib;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::box_filter_point
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void PfmFile::
box_filter_point(LPoint3f &result, PN_float32 &coverage,
                 int x, int y, PN_float32 x_contrib, PN_float32 y_contrib) const {
  if (!has_point(x, y)) {
    return;
  }
  const LPoint3f &point = get_point(x, y);

  PN_float32 contrib = x_contrib * y_contrib;
  result += point * contrib;
  coverage += contrib;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::box_filter_point
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
void PfmFile::
box_filter_point(LPoint4f &result, PN_float32 &coverage,
                 int x, int y, PN_float32 x_contrib, PN_float32 y_contrib) const {
  if (!has_point(x, y)) {
    return;
  }
  const LPoint4f &point = get_point4(x, y);

  PN_float32 contrib = x_contrib * y_contrib;
  result += point * contrib;
  coverage += contrib;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::add_vis_column
//       Access: Private, Static
//  Description: The private implementation of the public
//               add_vis_column(), this adds the column to the
//               indicated specific vector.
////////////////////////////////////////////////////////////////////
void PfmFile::
add_vis_column(VisColumns &vis_columns, ColumnType source, ColumnType target,
               InternalName *name, const TransformState *transform,
               const Lens *lens) {
  VisColumn column;
  column._source = source;
  column._target = target;
  column._name = name;
  column._transform = transform;
  if (transform == NULL) {
    column._transform = TransformState::make_identity();
  }
  column._lens = lens;
  vis_columns.push_back(column);
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::build_auto_vis_columns
//       Access: Private
//  Description: This function is called internally to construct the
//               list of vis_columns automatically from the high-level
//               interfaces such as set_vis_inverse(),
//               set_flat_texcoord_name(), and set_vis_2d().  It's not
//               called if the list has been build explicitly.
////////////////////////////////////////////////////////////////////
void PfmFile::
build_auto_vis_columns(VisColumns &vis_columns, bool for_points) const {
  vis_columns.clear();

  if (_vis_2d) {
    // No normals needed if we're just generating a 2-d mesh.
    if (_vis_inverse) {
      add_vis_column(vis_columns, CT_texcoord2, CT_vertex2, InternalName::get_vertex());
      add_vis_column(vis_columns, CT_vertex2, CT_texcoord2, InternalName::get_texcoord());
    } else {
      add_vis_column(vis_columns, CT_vertex2, CT_vertex2, InternalName::get_vertex());
      add_vis_column(vis_columns, CT_texcoord2, CT_texcoord2, InternalName::get_texcoord());
    }

  } else {
    if (_vis_inverse) {
      // We need a 3-d texture coordinate if we're inverting the vis
      // and it's 3-d.  But we still don't need normals in that case.
      add_vis_column(vis_columns, CT_texcoord3, CT_vertex3, InternalName::get_vertex());
      add_vis_column(vis_columns, CT_vertex3, CT_texcoord3, InternalName::get_texcoord());
    } else {
      // Otherwise, we only need a 2-d texture coordinate, and we do
      // want normals.
      add_vis_column(vis_columns, CT_vertex3, CT_vertex3, InternalName::get_vertex());
      add_vis_column(vis_columns, CT_normal3, CT_normal3, InternalName::get_normal());
      add_vis_column(vis_columns, CT_texcoord2, CT_texcoord2, InternalName::get_texcoord());
    }
  }

  if (_flat_texcoord_name != (InternalName *)NULL) {
    // We need an additional texcoord column for the flat texcoords.
    add_vis_column(vis_columns, CT_texcoord2, CT_texcoord2, _flat_texcoord_name);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::make_array_format
//       Access: Private
//  Description: Constructs a GeomVertexFormat that corresponds to the
//               vis_columns list.
////////////////////////////////////////////////////////////////////
CPT(GeomVertexFormat) PfmFile::
make_array_format(const VisColumns &vis_columns) const {
  PT(GeomVertexArrayFormat) array_format = new GeomVertexArrayFormat;

  for (VisColumns::const_iterator vci = vis_columns.begin();
       vci != vis_columns.end();
       ++vci) {
    const VisColumn &column = *vci;
    InternalName *name = column._name;

    int num_components = 0;
    GeomEnums::NumericType numeric_type = GeomEnums::NT_float32;
    GeomEnums::Contents contents = GeomEnums::C_point;
    switch (column._target) {
    case CT_texcoord2:
      num_components = 2;
      numeric_type = GeomEnums::NT_float32;
      contents = GeomEnums::C_texcoord;
      break;

    case CT_texcoord3:
      num_components = 3;
      numeric_type = GeomEnums::NT_float32;
      contents = GeomEnums::C_texcoord;
      break;

    case CT_vertex1:
    case CT_vertex2:
    case CT_vertex3:
      num_components = 3;
      numeric_type = GeomEnums::NT_float32;
      contents = GeomEnums::C_point;
      break;

    case CT_normal3:
      num_components = 3;
      numeric_type = GeomEnums::NT_float32;
      contents = GeomEnums::C_vector;
      break;
    }
    nassertr(num_components != 0, NULL);
    
    array_format->add_column(name, num_components, numeric_type, contents);
  }

  return GeomVertexFormat::register_format(array_format);
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
               int xi, int yi, int dist, int sxi, int syi) const {
  if (xi < 0 || xi >= x_size || yi < 0 || yi >= y_size) {
    // Out of bounds.
    return;
  }

  int gi = yi * x_size + xi;
  if (mini_grid[gi]._dist == -1 || mini_grid[gi]._dist > dist) {
    // Here's an undefined value that we need to populate.
    mini_grid[gi]._dist = dist;
    mini_grid[gi]._sxi = sxi;
    mini_grid[gi]._syi = syi;
    fill_mini_grid(mini_grid, x_size, y_size, xi + 1, yi, dist + 1, sxi, syi);
    fill_mini_grid(mini_grid, x_size, y_size, xi - 1, yi, dist + 1, sxi, syi);
    fill_mini_grid(mini_grid, x_size, y_size, xi, yi + 1, dist + 1, sxi, syi);
    fill_mini_grid(mini_grid, x_size, y_size, xi, yi - 1, dist + 1, sxi, syi);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::has_point_noop
//       Access: Private, Static
//  Description: The implementation of has_point() for 
//               files without a no_data_value.
////////////////////////////////////////////////////////////////////
bool PfmFile::
has_point_noop(const PfmFile *self, int x, int y) {
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::has_point_1
//       Access: Private, Static
//  Description: The implementation of has_point() for 1-component
//               files with a no_data_value.
////////////////////////////////////////////////////////////////////
bool PfmFile::
has_point_1(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) && 
      (y >= 0 && y < self->_y_size)) {
    return self->_table[(y * self->_x_size + x)] != self->_no_data_value[0];
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::has_point_3
//       Access: Private, Static
//  Description: The implementation of has_point() for 3-component
//               files with a no_data_value.
////////////////////////////////////////////////////////////////////
bool PfmFile::
has_point_3(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) && 
      (y >= 0 && y < self->_y_size)) {
    return *(LPoint3f *)&self->_table[(y * self->_x_size + x) * 3] != *(LPoint3f *)&self->_no_data_value;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::has_point_4
//       Access: Private, Static
//  Description: The implementation of has_point() for 4-component
//               files with a no_data_value.
////////////////////////////////////////////////////////////////////
bool PfmFile::
has_point_4(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) && 
      (y >= 0 && y < self->_y_size)) {
    return *(LPoint4f *)&self->_table[(y * self->_x_size + x) * 4] != *(LPoint4f *)&self->_no_data_value;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::has_point_chan4
//       Access: Private, Static
//  Description: The implementation of has_point() for 4-component
//               files with set_no_data_chan4() in effect.  This means
//               that the data is valid iff the fourth channel >= 0.
////////////////////////////////////////////////////////////////////
bool PfmFile::
has_point_chan4(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) && 
      (y >= 0 && y < self->_y_size)) {
    return self->_table[(y * self->_x_size + x) * 4 + 3] >= 0.0;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::VisColumn::add_data
//       Access: Public
//  Description: Adds the data for this column to the appropriate
//               column of the GeomVertexWriter.
////////////////////////////////////////////////////////////////////
void PfmFile::VisColumn::
add_data(const PfmFile &file, GeomVertexWriter &vwriter, int xi, int yi, bool reverse_normals) const {
  switch (_source) {
  case CT_texcoord2:
    { 
      LPoint2f uv(PN_float32(xi) / PN_float32(file._x_size - 1),
                  PN_float32(yi) / PN_float32(file._y_size - 1));
      transform_point(uv);
      vwriter.set_data2f(uv);
    }
    break;

  case CT_texcoord3:
    {
      LPoint3f uv(PN_float32(xi) / PN_float32(file._x_size - 1),
                  PN_float32(yi) / PN_float32(file._y_size - 1), 
                  0.0f);
      transform_point(uv);
      vwriter.set_data3f(uv);
    }
    break;

  case CT_vertex1:
    {
      PN_float32 p = file.get_point1(xi, yi);
      LPoint2f point(p, 0.0);
      transform_point(point);
      vwriter.set_data2f(point);
    }
    break;

  case CT_vertex2:
    {
      LPoint2f point = file.get_point2(xi, yi);
      transform_point(point);
      vwriter.set_data2f(point);
    }
    break;

  case CT_vertex3:
    {
      LPoint3f point = file.get_point(xi, yi);
      transform_point(point);
      vwriter.set_data3f(point);
    }
    break;

  case CT_normal3:
    {
      // Calculate the normal based on two neighboring vertices.
      LPoint3f v[3];
      v[0] = file.get_point(xi, yi);
      if (xi + 1 < file._x_size) {
        v[1] = file.get_point(xi + 1, yi);
      } else {
        v[1] = v[0];
        v[0] = file.get_point(xi - 1, yi);
      }
                
      if (yi + 1 < file._y_size) {
        v[2] = file.get_point(xi, yi + 1);
      } else {
        v[2] = v[0];
        v[0] = file.get_point(xi, yi - 1);
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
      nassertv(!n.is_nan());
      if (reverse_normals) {
        n = -n;
      }
      transform_vector(n);
      vwriter.set_data3f(n);
    }
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::VisColumn::transform_point
//       Access: Public
//  Description: Transforms the indicated point as specified by the
//               VisColumn.
////////////////////////////////////////////////////////////////////
void PfmFile::VisColumn::
transform_point(LPoint2f &point) const {
  if (!_transform->is_identity()) {
    LCAST(PN_float32, _transform->get_mat3()).xform_point_in_place(point);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::VisColumn::transform_point
//       Access: Public
//  Description: Transforms the indicated point as specified by the
//               VisColumn.
////////////////////////////////////////////////////////////////////
void PfmFile::VisColumn::
transform_point(LPoint3f &point) const {
  if (!_transform->is_identity()) {
    LCAST(PN_float32, _transform->get_mat()).xform_point_in_place(point);
  }
  if (_lens != (Lens *)NULL) {
    static LMatrix4f to_uv(0.5, 0.0, 0.0, 0.0,
                           0.0, 0.5, 0.0, 0.0, 
                           0.0, 0.0, 1.0, 0.0, 
                           0.5, 0.5, 0.0, 1.0);
    LPoint3 film;
    _lens->project(LCAST(PN_stdfloat, point), film);
    point = to_uv.xform_point(LCAST(PN_float32, film));
  }
}

////////////////////////////////////////////////////////////////////
//     Function: PfmFile::VisColumn::transform_vector
//       Access: Public
//  Description: Transforms the indicated vector as specified by the
//               VisColumn.
////////////////////////////////////////////////////////////////////
void PfmFile::VisColumn::
transform_vector(LVector3f &vec) const {
  if (!_transform->is_identity()) {
    LCAST(PN_float32, _transform->get_mat()).xform_vec_in_place(vec);
  }
}
