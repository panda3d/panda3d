/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfmFile.cxx
 * @author drose
 * @date 2010-12-23
 */

#include "config_pnmimage.h"
#include "pfmFile.h"
#include "virtualFileSystem.h"
#include "pandaFileStream.h"
#include "littleEndian.h"
#include "bigEndian.h"
#include "cmath.h"
#include "pnmImage.h"
#include "pnmReader.h"
#include "pnmWriter.h"
#include "string_utils.h"
#include "look_at.h"

using std::istream;
using std::max;
using std::min;
using std::ostream;

/**
 *
 */
PfmFile::
PfmFile() {
  _has_no_data_value = false;
  _has_no_data_threshold = false;
  _no_data_value = LPoint4f::zero();
  _has_point = has_point_noop;
  clear();
}

/**
 *
 */
PfmFile::
PfmFile(const PfmFile &copy) :
  PNMImageHeader(copy),
  _table(copy._table),
  _scale(copy._scale),
  _has_no_data_value(copy._has_no_data_value),
  _has_no_data_threshold(copy._has_no_data_threshold),
  _no_data_value(copy._no_data_value),
  _has_point(copy._has_point)
{
}

/**
 *
 */
void PfmFile::
operator = (const PfmFile &copy) {
  PNMImageHeader::operator = (copy);
  _table = copy._table;
  _scale = copy._scale;
  _has_no_data_value = copy._has_no_data_value;
  _has_no_data_threshold = copy._has_no_data_threshold;
  _no_data_value = copy._no_data_value;
  _has_point = copy._has_point;
}

/**
 * Eliminates all data in the file.
 */
void PfmFile::
clear() {
  _x_size = 0;
  _y_size = 0;
  _scale = 1.0;
  _num_channels = 0;
  _table.clear();
  clear_no_data_value();
}

/**
 * Resets to an empty table with a specific size.  The case of num_channels ==
 * 0 is allowed only in the case that x_size and y_size are also == 0; and
 * this makes an empty (and invalid) PfmFile.
 */
void PfmFile::
clear(int x_size, int y_size, int num_channels) {
  nassertv(x_size >= 0 && y_size >= 0);
  nassertv((num_channels > 0 && num_channels <= 4) ||
           (x_size == 0 && y_size == 0 && num_channels == 0));

  _x_size = x_size;
  _y_size = y_size;
  _scale = 1.0;
  _num_channels = num_channels;

  _table.clear();
  int size = _x_size * _y_size * _num_channels;

  // We allocate a little bit bigger to allow safe overflow: you can call
  // get_point3() or get_point4() on the last point of a 1- or 3-channel
  // image.
  _table.insert(_table.end(), size + 4, (PN_float32)0.0);

  clear_no_data_value();
}

/**
 * Reads the PFM data from the indicated file, returning true on success,
 * false on failure.
 *
 * This can also handle reading a standard image file supported by PNMImage;
 * it will be quietly converted to a floating-point type.
 */
bool PfmFile::
read(const Filename &fullpath) {
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename filename = Filename::binary_filename(fullpath);
  PT(VirtualFile) file = vfs->get_file(filename);
  if (file == nullptr) {
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

/**
 * Reads the PFM data from the indicated stream, returning true on success,
 * false on failure.
 *
 * This can also handle reading a standard image file supported by PNMImage;
 * it will be quietly converted to a floating-point type.
 */
bool PfmFile::
read(istream &in, const Filename &fullpath) {
  PNMReader *reader = make_reader(&in, false, fullpath);
  if (reader == nullptr) {
    clear();
    return false;
  }
  return read(reader);
}

/**
 * Reads the PFM data using the indicated PNMReader.
 *
 * The PNMReader is always deleted upon completion, whether successful or not.
 */
bool PfmFile::
read(PNMReader *reader) {
  clear();

  if (reader == nullptr) {
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

/**
 * Writes the PFM data to the indicated file, returning true on success, false
 * on failure.
 *
 * If the type implied by the filename extension supports floating-point, the
 * data will be written directly; otherwise, the floating-point data will be
 * quietly converted to the appropriate integer type.
 */
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

  if (pnmimage_cat.is_debug()) {
    pnmimage_cat.debug()
      << "Writing PFM file " << filename << "\n";
  }

  return write(out, fullpath);
}

/**
 * Writes the PFM data to the indicated stream, returning true on success,
 * false on failure.
 */
bool PfmFile::
write(ostream &out, const Filename &fullpath) {
  if (!is_valid()) {
    return false;
  }

  PNMWriter *writer = make_writer(&out, false, fullpath);
  if (writer == nullptr) {
    return false;
  }

  return write(writer);
}

/**
 * Writes the PFM data using the indicated PNMWriter.
 *
 * The PNMWriter is always deleted upon completion, whether successful or not.
 */
bool PfmFile::
write(PNMWriter *writer) {
  if (writer == nullptr) {
    return false;
  }

  if (!is_valid()) {
    delete writer;
    return false;
  }

  writer->copy_header_from(*this);

  if (!writer->supports_floating_point()) {
    // Hmm, it's an integer file type.  Convert it from the floating-point
    // data we have.
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

/**
 * Fills the PfmFile with the data from the indicated PNMImage, converted to
 * floating-point values.
 */
bool PfmFile::
load(const PNMImage &pnmimage) {
  if (!pnmimage.is_valid()) {
    clear();
    return false;
  }

  int num_channels = pnmimage.get_num_channels();

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

  case 2:
    {
      for (int yi = 0; yi < pnmimage.get_y_size(); ++yi) {
        for (int xi = 0; xi < pnmimage.get_x_size(); ++xi) {
          PN_float32 *point = &_table[(yi * _x_size + xi) * _num_channels];
          point[0] = pnmimage.get_gray(xi, yi);
          point[1] = pnmimage.get_alpha(xi, yi);
        }
      }
    }
    break;

  case 3:
    {
      for (int yi = 0; yi < pnmimage.get_y_size(); ++yi) {
        for (int xi = 0; xi < pnmimage.get_x_size(); ++xi) {
          PN_float32 *point = &_table[(yi * _x_size + xi) * _num_channels];
          LRGBColorf xel = pnmimage.get_xel(xi, yi);
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
          LColorf xel = pnmimage.get_xel_a(xi, yi);
          point[0] = xel[0];
          point[1] = xel[1];
          point[2] = xel[2];
          point[3] = xel[3];
        }
      }
    }
    break;

  default:
    nassert_raise("unexpected channel count");
    return false;
  }
  return true;
}


/**
 * Copies the data to the indicated PNMImage, converting to RGB values.
 */
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

  case 2:
    {
      for (int yi = 0; yi < get_y_size(); ++yi) {
        for (int xi = 0; xi < get_x_size(); ++xi) {
          const LPoint2f &point = get_point2(xi, yi);
          pnmimage.set_gray(xi, yi, point[0]);
          pnmimage.set_alpha(xi, yi, point[1]);
        }
      }
    }
    break;

  case 3:
    {
      for (int yi = 0; yi < get_y_size(); ++yi) {
        for (int xi = 0; xi < get_x_size(); ++xi) {
          const LPoint3f &point = get_point3(xi, yi);
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
          pnmimage.set_xel_a(xi, yi, point[0], point[1], point[2], point[3]);
        }
      }
    }
    break;

  default:
    nassert_raise("unexpected channel count");
    return false;
  }
  return true;
}

/**
 * Stores 1 or 0 values into the indicated PNMImage, according to has_point()
 * for each pixel.  Each valid point gets a 1 value; each nonexistent point
 * gets a 0 value.
 */
bool PfmFile::
store_mask(PNMImage &pnmimage) const {
  if (!is_valid()) {
    pnmimage.clear();
    return false;
  }

  pnmimage.clear(get_x_size(), get_y_size(), 1, 255);
  for (int yi = 0; yi < get_y_size(); ++yi) {
    for (int xi = 0; xi < get_x_size(); ++xi) {
      pnmimage.set_gray(xi, yi, has_point(xi, yi));
    }
  }
  return true;
}

/**
 * Stores 1 or 0 values into the indicated PNMImage, according to has_point()
 * for each pixel.  Each valid point gets a 1 value; each nonexistent point
 * gets a 0 value.
 *
 * This flavor of store_mask also checks whether the valid points are within
 * the specified min/max range.  Any valid points without the condition
 * min_point[c] <= value[c] <= max_point[c], for any c, are stored with a 0 in
 * the mask.
 */
bool PfmFile::
store_mask(PNMImage &pnmimage, const LVecBase4f &min_point, const LVecBase4f &max_point) const {
  if (!is_valid()) {
    pnmimage.clear();
    return false;
  }

  pnmimage.clear(get_x_size(), get_y_size(), 1, 255);
  for (int yi = 0; yi < get_y_size(); ++yi) {
    for (int xi = 0; xi < get_x_size(); ++xi) {
      if (!has_point(xi, yi)) {
        pnmimage.set_gray(xi, yi, 0);
      } else {
        const float *value = &_table[(yi * _x_size + xi) * _num_channels];
        bool in_range = true;
        for (int ci = 0; ci < _num_channels; ++ci) {
          if (value[ci] < min_point[ci] || value[ci] > max_point[ci]) {
            in_range = false;
            break;
          }
        }
        pnmimage.set_gray(xi, yi, (float)in_range);
      }
    }
  }
  return true;
}

/**
 * Fills the table with all of the same value.
 */
void PfmFile::
fill(const LPoint4f &value) {
  switch (_num_channels) {
  case 1:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          _table[(yi * _x_size + xi)] = value[0];
        }
      }
    }
    break;

  case 2:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          (*(LPoint2f *)&_table[(yi * _x_size + xi) * _num_channels]).set(value[0], value[1]);
        }
      }
    }
    break;

  case 3:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          (*(LPoint3f *)&_table[(yi * _x_size + xi) * _num_channels]).set(value[0], value[1], value[2]);
        }
      }
    }
    break;

  case 4:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          *(LPoint4f *)&_table[(yi * _x_size + xi) * _num_channels] = value;
        }
      }
    }
    break;
  }
}

/**
 * Fills the table with all NaN.
 */
void PfmFile::
fill_nan() {
  PN_float32 nan = make_nan((PN_float32)0.0);
  LPoint4f nan4(nan, nan, nan, nan);
  fill(nan4);
}

/**
 * Fills the table with the current no_data value, so that the table is empty.
 */
void PfmFile::
fill_no_data_value() {
  fill(_no_data_value);
}

/**
 * Fills the indicated channel with all of the same value, leaving the other
 * channels unchanged.
 */
void PfmFile::
fill_channel(int channel, PN_float32 value) {
  nassertv(channel >= 0 && channel < _num_channels);

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      _table[(yi * _x_size + xi) * _num_channels + channel] = value;
    }
  }
}

/**
 * Fills the indicated channel with NaN, leaving the other channels unchanged.
 */
void PfmFile::
fill_channel_nan(int channel) {
  PN_float32 nan = make_nan((PN_float32)0.0);
  fill_channel(channel, nan);
}

/**
 * Fills the indicated channel with all of the same value, but only where the
 * table already has a data point.  Leaves empty points unchanged.
 */
void PfmFile::
fill_channel_masked(int channel, PN_float32 value) {
  nassertv(channel >= 0 && channel < _num_channels);

  if (!_has_no_data_value) {
    fill_channel(channel, value);
  } else {
    for (int yi = 0; yi < _y_size; ++yi) {
      for (int xi = 0; xi < _x_size; ++xi) {
        if (has_point(xi, yi)) {
          _table[(yi * _x_size + xi) * _num_channels + channel] = value;
        }
      }
    }
  }
}

/**
 * Fills the indicated channel with NaN, but only where the table already has
 * a data point.  Leaves empty points unchanged.
 */
void PfmFile::
fill_channel_masked_nan(int channel) {
  PN_float32 nan = make_nan((PN_float32)0.0);
  fill_channel_masked(channel, nan);
}

/**
 * Computes the unweighted average point of all points within the box centered
 * at (x, y) with the indicated Manhattan-distance radius.  Missing points are
 * assigned the value of their nearest neighbor.  Returns true if successful,
 * or false if the point value cannot be determined.
 */
bool PfmFile::
calc_average_point(LPoint3f &result, PN_float32 x, PN_float32 y, PN_float32 radius) const {
  result = LPoint3f::zero();

  int min_x = int(ceil(x - radius));
  int min_y = int(ceil(y - radius));
  int max_x = int(floor(x + radius));
  int max_y = int(floor(y + radius));

  // We first construct a mini-grid of x_size by y_size integer values to
  // index into the main table.  This indirection allows us to fill in the
  // holes in the mini-grid with the nearest known values before we compute
  // the average.
  int x_size = max_x - min_x + 1;
  int y_size = max_y - min_y + 1;
  int size = x_size * y_size;
  if (size == 0) {
    return false;
  }

  pvector<MiniGridCell> mini_grid;
  mini_grid.insert(mini_grid.end(), size, MiniGridCell());

  // Now collect the known data points and apply them to the mini-grid.
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

  // Now the mini-grid is completely filled, so we can compute the average.
  for (int gi = 0; gi < size; ++gi) {
    int sxi = mini_grid[gi]._sxi;
    int syi = mini_grid[gi]._syi;
    const LPoint3f &p = get_point(sxi, syi);
    result += p;
  }

  result /= PN_float32(size);
  return true;
}

/**
 * Computes the weighted average of the four nearest points to the floating-
 * point index (x, y).  Returns true if the point has any contributors, false
 * if the point is unknown.
 */
bool PfmFile::
calc_bilinear_point(LPoint3f &result, PN_float32 x, PN_float32 y) const {
  result = LPoint3f::zero();

  x = (x * _x_size - 0.5);
  y = (y * _y_size - 0.5);

  int min_x = int(floor(x));
  int min_y = int(floor(y));

  PN_float32 frac_x = x - min_x;
  PN_float32 frac_y = y - min_y;

  LPoint3f p00(LPoint3f::zero()), p01(LPoint3f::zero()), p10(LPoint3f::zero()), p11(LPoint3f::zero());
  PN_float32 w00 = 0.0, w01 = 0.0, w10 = 0.0, w11 = 0.0;

  if (has_point(min_x, min_y)) {
    w00 = (1.0 - frac_y) * (1.0 - frac_x);
    p00 = get_point(min_x, min_y);
  }
  if (has_point(min_x + 1, min_y)) {
    w10 = (1.0 - frac_y) * frac_x;
    p10 = get_point(min_x + 1, min_y);
  }
  if (has_point(min_x, min_y + 1)) {
    w01 = frac_y * (1.0 - frac_x);
    p01 = get_point(min_x, min_y + 1);
  }
  if (has_point(min_x + 1, min_y + 1)) {
    w11 = frac_y * frac_x;
    p11 = get_point(min_x + 1, min_y + 1);
  }

  PN_float32 net_w = w00 + w01 + w10 + w11;
  if (net_w == 0.0) {
    return false;
  }

  result = (p00 * w00 + p01 * w01 + p10 * w10 + p11 * w11) / net_w;
  return true;
}

/**
 * Calculates the minimum and maximum x, y, and z depth component values,
 * representing the bounding box of depth values, and places them in the
 * indicated vectors.  Returns true if successful, false if the mesh contains
 * no points.
 */
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

/**
 * Computes the minimum range of x and y across the PFM file that include all
 * points.  If there are no points with no_data_value in the grid--that is,
 * all points are included--then this will return (0, get_x_size(), 0,
 * get_y_size()).
 */
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

/**
 * Returns true if all of the points on row y, in the range [x_begin, x_end),
 * are the no_data value, or false if any one of these points has a value.
 */
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

/**
 * Returns true if all of the points on column x, from [y_begin, y_end), are
 * the no_data value, or false if any one of these points has a value.
 */
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

/**
 * Sets the no_data_nan flag.  When num_channels is nonzero, then a NaN value
 * in any of the first num_channels channels indicates no data for that point.
 * If num_channels is zero, then all points are valid.
 *
 * This is a special case of set_no_data_value().
 */
void PfmFile::
set_no_data_nan(int num_channels) {
  if (num_channels > 0) {
    num_channels = min(num_channels, _num_channels);
    _has_no_data_value = true;
    _has_no_data_threshold = false;
    _no_data_value = LPoint4f::zero();
    PN_float32 nan = make_nan((PN_float32)0.0);
    for (int i = 0; i < num_channels; ++i) {
      _no_data_value[i] = nan;
    }
    switch (num_channels) {
    case 1:
      _has_point = has_point_nan_1;
      break;
    case 2:
      _has_point = has_point_nan_2;
      break;
    case 3:
      _has_point = has_point_nan_3;
      break;
    case 4:
      _has_point = has_point_nan_4;
      break;
    default:
      nassert_raise("unexpected channel count");
      break;
    }
  } else {
    clear_no_data_value();
  }
}

/**
 * Sets the special value that means "no data" when it appears in the pfm
 * file.
 */
void PfmFile::
set_no_data_value(const LPoint4f &no_data_value) {
  nassertv(is_valid());

  _has_no_data_value = true;
  _has_no_data_threshold = false;
  _no_data_value = no_data_value;
  switch (_num_channels) {
  case 1:
    _has_point = has_point_1;
    break;
  case 2:
    _has_point = has_point_2;
    break;
  case 3:
    _has_point = has_point_3;
    break;
  case 4:
    _has_point = has_point_4;
    break;
  default:
    nassert_raise("unexpected channel count");
    break;
  }
}

/**
 * Sets the special threshold value.  Points that are below this value in all
 * components are considered "no value".
 */
void PfmFile::
set_no_data_threshold(const LPoint4f &no_data_value) {
  nassertv(is_valid());

  _has_no_data_value = true;
  _has_no_data_threshold = true;
  _no_data_value = no_data_value;
  switch (_num_channels) {
  case 1:
    _has_point = has_point_threshold_1;
    break;
  case 2:
    _has_point = has_point_threshold_2;
    break;
  case 3:
    _has_point = has_point_threshold_3;
    break;
  case 4:
    _has_point = has_point_threshold_4;
    break;
  default:
    nassert_raise("unexpected channel count");
    break;
  }
}

/**
 * Applies a simple filter to resample the pfm file in-place to the indicated
 * size.  Don't confuse this with applying a scale to all of the points via
 * xform().
 */
void PfmFile::
resize(int new_x_size, int new_y_size) {
  if (_x_size == 0 || _y_size == 0 || new_x_size == 0 || new_y_size == 0) {
    clear(new_x_size, new_y_size, _num_channels);
    return;
  }

  if (new_x_size == _x_size && new_y_size == _y_size) {
    return;
  }

  PfmFile result;
  result.clear(new_x_size, new_y_size, _num_channels);
  if (_has_no_data_value) {
    result.fill(_no_data_value);
  }

  if (pfm_resize_quick && new_x_size <= _x_size && new_y_size <= _y_size) {
    // If we're downscaling, we can use quick_filter, which is faster.
    result.quick_filter_from(*this);

  } else {
    // Otherwise, we should use box_filter() or gaussian_filter, which are
    // more general.
    if (pfm_resize_gaussian) {
      result.gaussian_filter_from(pfm_resize_radius, *this);
    } else {
      result.box_filter_from(pfm_resize_radius, *this);
    }
  }

  _table.swap(result._table);
  _x_size = new_x_size;
  _y_size = new_y_size;
}

/**
 * Resizes from the given image, with a fixed radius of 0.5. This is a very
 * specialized and simple algorithm that doesn't handle dropping below the
 * Nyquist rate very well, but is quite a bit faster than the more general
 * box_filter(), above.
 */
void PfmFile::
quick_filter_from(const PfmFile &from) {
  if (_x_size == 0 || _y_size == 0) {
    return;
  }

  Table new_data;
  new_data.reserve(_table.size());

  PN_float32 from_x0, from_x1, from_y0, from_y1;

  int orig_x_size = from.get_x_size();
  int orig_y_size = from.get_y_size();

  PN_float32 x_scale = 1.0;
  PN_float32 y_scale = 1.0;

  if (_x_size > 1) {
    x_scale = (PN_float32)orig_x_size / (PN_float32)_x_size;
  }
  if (_y_size > 1) {
    y_scale = (PN_float32)orig_y_size / (PN_float32)_y_size;
  }

  switch (_num_channels) {
  case 1:
    {
      from_y0 = 0.0;
      for (int to_y = 0; to_y < _y_size; ++to_y) {
        from_y1 = (to_y + 1.0) * y_scale;
        from_y1 = min(from_y1, (PN_float32)orig_y_size);

        from_x0 = 0.0;
        for (int to_x = 0; to_x < _x_size; ++to_x) {
          from_x1 = (to_x + 1.0) * x_scale;
          from_x1 = min(from_x1, (PN_float32)orig_x_size);

          // Now the box from (from_x0, from_y0) - (from_x1, from_y1) but not
          // including (from_x1, from_y1) maps to the pixel (to_x, to_y).
          PN_float32 result;
          from.box_filter_region(result, from_x0, from_y0, from_x1, from_y1);
          new_data.push_back(result);

          from_x0 = from_x1;
        }
        from_y0 = from_y1;
      }
    }
    break;

  case 2:
    {
      from_y0 = 0.0;
      for (int to_y = 0; to_y < _y_size; ++to_y) {
        from_y1 = (to_y + 1.0) * y_scale;
        from_y1 = min(from_y1, (PN_float32)orig_y_size);

        from_x0 = 0.0;
        for (int to_x = 0; to_x < _x_size; ++to_x) {
          from_x1 = (to_x + 1.0) * x_scale;
          from_x1 = min(from_x1, (PN_float32)orig_x_size);

          // Now the box from (from_x0, from_y0) - (from_x1, from_y1) but not
          // including (from_x1, from_y1) maps to the pixel (to_x, to_y).
          LPoint2f result;
          from.box_filter_region(result, from_x0, from_y0, from_x1, from_y1);
          new_data.push_back(result[0]);
          new_data.push_back(result[1]);

          from_x0 = from_x1;
        }
        from_y0 = from_y1;
      }
    }
    break;

  case 3:
    {
      from_y0 = 0.0;
      for (int to_y = 0; to_y < _y_size; ++to_y) {
        from_y1 = (to_y + 1.0) * y_scale;
        from_y1 = min(from_y1, (PN_float32)orig_y_size);

        from_x0 = 0.0;
        for (int to_x = 0; to_x < _x_size; ++to_x) {
          from_x1 = (to_x + 1.0) * x_scale;
          from_x1 = min(from_x1, (PN_float32)orig_x_size);

          // Now the box from (from_x0, from_y0) - (from_x1, from_y1) but not
          // including (from_x1, from_y1) maps to the pixel (to_x, to_y).
          LPoint3f result;
          from.box_filter_region(result, from_x0, from_y0, from_x1, from_y1);
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
      for (int to_y = 0; to_y < _y_size; ++to_y) {
        from_y1 = (to_y + 1.0) * y_scale;
        from_y1 = min(from_y1, (PN_float32)orig_y_size);

        from_x0 = 0.0;
        for (int to_x = 0; to_x < _x_size; ++to_x) {
          from_x1 = (to_x + 1.0) * x_scale;
          from_x1 = min(from_x1, (PN_float32)orig_x_size);

          // Now the box from (from_x0, from_y0) - (from_x1, from_y1) but not
          // including (from_x1, from_y1) maps to the pixel (to_x, to_y).
          LPoint4f result;
          from.box_filter_region(result, from_x0, from_y0, from_x1, from_y1);
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
    nassert_raise("unexpected channel count");
    return;
  }

  new_data.push_back(0.0);
  new_data.push_back(0.0);
  new_data.push_back(0.0);
  new_data.push_back(0.0);

  nassertv(new_data.size() == _table.size());
  _table.swap(new_data);
}

/**
 * Performs an in-place reversal of the row (y) data.
 */
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

/**
 * Reverses, transposes, and/or rotates the table in-place according to the
 * specified parameters.  If flip_x is true, the x axis is reversed; if flip_y
 * is true, the y axis is reversed.  Then, if transpose is true, the x and y
 * axes are exchanged.  These parameters can be used to select any combination
 * of 90-degree or 180-degree rotations and flips.
 */
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

/**
 * Applies the indicated transform matrix to all points in-place.
 */
void PfmFile::
xform(const LMatrix4f &transform) {
  nassertv(is_valid());

  int num_channels = get_num_channels();
  switch (num_channels) {
  case 1:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          if (!has_point(xi, yi)) {
            continue;
          }
          PN_float32 pi = get_point1(xi, yi);
          LPoint3f po = transform.xform_point(LPoint3f(pi, 0.0, 0.0));
          set_point1(xi, yi, po[0]);
        }
      }
    }
    break;

  case 2:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          if (!has_point(xi, yi)) {
            continue;
          }
          LPoint2f pi = get_point2(xi, yi);
          LPoint3f po = transform.xform_point(LPoint3f(pi[0], pi[1], 0.0));
          set_point2(xi, yi, LPoint2f(po[0], po[1]));
        }
      }
    }
    break;

  case 3:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          if (!has_point(xi, yi)) {
            continue;
          }
          LPoint3f &p = modify_point3(xi, yi);
          transform.xform_point_general_in_place(p);
        }
      }
    }
    break;

  case 4:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          if (!has_point(xi, yi)) {
            continue;
          }
          LPoint4f &p = modify_point4(xi, yi);
          transform.xform_in_place(p);
        }
      }
    }
    break;
  }
}

/**
 * Applies the distortion indicated in the supplied dist map to the current
 * map.  The dist map is understood to be a mapping of points in the range
 * 0..1 in the first two dimensions.
 *
 * The operation can be expressed symbolically as:
 *
 * this(u, v) = this(dist(u, v))
 *
 * If scale_factor is not 1, it should be a value > 1, and it specifies the
 * factor to upscale the working table while processing, to reduce artifacts
 * from integer truncation.
 *
 * By convention, the y axis is inverted in the distortion map relative to the
 * coordinates here.  A y value of 0 in the distortion map corresponds with a
 * v value of 1 in this file.
 */
void PfmFile::
forward_distort(const PfmFile &dist, PN_float32 scale_factor) {
  int working_x_size = (int)cceil(_x_size * scale_factor);
  int working_y_size = (int)cceil(_y_size * scale_factor);

  working_x_size = max(working_x_size, dist.get_x_size());
  working_y_size = max(working_y_size, dist.get_y_size());

  const PfmFile *source_p = this;
  PfmFile scaled_source;
  if ((*this).get_x_size() != working_x_size || (*this).get_y_size() != working_y_size) {
    // Rescale the source file as needed.
    scaled_source = (*this);
    scaled_source.resize(working_x_size, working_y_size);
    source_p = &scaled_source;
  }

  const PfmFile *dist_p = &dist;
  PfmFile scaled_dist;
  if (dist.get_x_size() != working_x_size || dist.get_y_size() != working_y_size) {
    // Rescale the dist file as needed.
    scaled_dist = dist;
    scaled_dist.resize(working_x_size, working_y_size);
    dist_p = &scaled_dist;
  }

  PfmFile result;
  result.clear(working_x_size, working_y_size, _num_channels);

  if (_has_no_data_value) {
    result.set_no_data_value(_no_data_value);
    result.fill(_no_data_value);
  }

  for (int yi = 0; yi < working_y_size; ++yi) {
    for (int xi = 0; xi < working_x_size; ++xi) {
      if (!dist_p->has_point(xi, yi)) {
        continue;
      }
      LPoint2f uv = dist_p->get_point2(xi, yi);
      LPoint3f p;
      if (!source_p->calc_bilinear_point(p, uv[0], 1.0 - uv[1])) {
        continue;
      }
      nassertv(!p.is_nan());
      result.set_point(xi, working_y_size - 1 - yi, p);
    }
  }

  // Resize to the target size for completion.
  result.resize(_x_size, _y_size);

  nassertv(result._table.size() == _table.size());
  _table.swap(result._table);
}

/**
 * Applies the distortion indicated in the supplied dist map to the current
 * map.  The dist map is understood to be a mapping of points in the range
 * 0..1 in the first two dimensions.
 *
 * The operation can be expressed symbolically as:
 *
 * this(u, v) = dist(this(u, v))
 *
 * If scale_factor is not 1, it should be a value > 1, and it specifies the
 * factor to upscale the working table while processing, to reduce artifacts
 * from integer truncation.
 *
 * By convention, the y axis in inverted in the distortion map relative to the
 * coordinates here.  A y value of 0 in the distortion map corresponds with a
 * v value of 1 in this file.
 */
void PfmFile::
reverse_distort(const PfmFile &dist, PN_float32 scale_factor) {
  int working_x_size = (int)cceil(_x_size * scale_factor);
  int working_y_size = (int)cceil(_y_size * scale_factor);

  working_x_size = max(working_x_size, dist.get_x_size());
  working_y_size = max(working_y_size, dist.get_y_size());

  const PfmFile *source_p = this;
  PfmFile scaled_source;
  if ((*this).get_x_size() != working_x_size || (*this).get_y_size() != working_y_size) {
    // Rescale the source file as needed.
    scaled_source = (*this);
    scaled_source.resize(working_x_size, working_y_size);
    source_p = &scaled_source;
  }

  const PfmFile *dist_p = &dist;
  PfmFile scaled_dist;
  if (dist.get_x_size() != working_x_size || dist.get_y_size() != working_y_size) {
    // Rescale the dist file as needed.
    scaled_dist = dist;
    scaled_dist.resize(working_x_size, working_y_size);
    dist_p = &scaled_dist;
  }

  PfmFile result;
  result.clear(working_x_size, working_y_size, _num_channels);

  if (_has_no_data_value) {
    result.set_no_data_value(_no_data_value);
    result.fill(_no_data_value);
  }

  for (int yi = 0; yi < working_y_size; ++yi) {
    for (int xi = 0; xi < working_x_size; ++xi) {
      if (!source_p->has_point(xi, yi)) {
        continue;
      }
      LPoint2f uv = source_p->get_point2(xi, yi);
      LPoint3f p;
      if (!dist_p->calc_bilinear_point(p, uv[0], 1.0 - uv[1])) {
        continue;
      }
      result.set_point(xi, yi, LPoint3f(p[0], 1.0 - p[1], p[2]));
    }
  }

  // Resize to the target size for completion.
  result.resize(_x_size, _y_size);

  nassertv(result._table.size() == _table.size());
  _table.swap(result._table);
}

/**
 * Assumes that lut is an X by 1, 1-component PfmFile whose X axis maps points
 * to target points.  For each point in this pfm file, computes: p(u,
 * v)[channel] = lut(p(u, v)[channel] * x_scale, 0)[0]
 */
void PfmFile::
apply_1d_lut(int channel, const PfmFile &lut, PN_float32 x_scale) {
  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      if (!has_point(xi, yi)) {
        continue;
      }

      PN_float32 v = get_channel(xi, yi, channel);
      LPoint3f p;
      if (!lut.calc_bilinear_point(p, v * x_scale, 0.5)) {
        continue;
      }
      set_channel(xi, yi, channel, p[0]);
    }
  }
}

/**
 * Wherever there is missing data in this PfmFile (that is, wherever
 * has_point() returns false), copy data from the other PfmFile, which must be
 * exactly the same dimensions as this one.
 */
void PfmFile::
merge(const PfmFile &other) {
  nassertv(is_valid() && other.is_valid());
  nassertv(other._x_size == _x_size && other._y_size == _y_size && other._num_channels == _num_channels);

  if (!_has_no_data_value) {
    // Trivial no-op.
    return;
  }

  size_t point_size = _num_channels * sizeof(PN_float32);
  for (int y = 0; y < _y_size; ++y) {
    for (int x = 0; x < _x_size; ++x) {
      if (!has_point(x, y) && other.has_point(x, y)) {
        memcpy(&_table[(y * _x_size + x) * _num_channels],
               &other._table[(y * _x_size + x) * _num_channels],
               point_size);
      }
    }
  }
}

/**
 * Wherever there is missing data in the other PfmFile, set this the
 * corresponding point in this PfmFile to missing as well, so that this
 * PfmFile has only points where both files have points.
 *
 * The point is set to "missing" by setting it the no_data_value.
 */
void PfmFile::
apply_mask(const PfmFile &other) {
  nassertv(is_valid() && other.is_valid());
  nassertv(other._x_size == _x_size && other._y_size == _y_size);

  if (!other._has_no_data_value || !_has_no_data_value) {
    // Trivial no-op.
    return;
  }

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      if (!other.has_point(xi, yi)) {
        set_point4(xi, yi, _no_data_value);
      }
    }
  }
}

/**
 * Copies just the specified channel values from the indicated PfmFile (which
 * could be same as this PfmFile) into the specified channel of this one.
 */
void PfmFile::
copy_channel(int to_channel, const PfmFile &other, int from_channel) {
  nassertv(is_valid() && other.is_valid());
  nassertv(other._x_size == _x_size && other._y_size == _y_size);
  nassertv(to_channel >= 0 && to_channel < get_num_channels() &&
           from_channel >= 0 && from_channel < other.get_num_channels());

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      set_channel(xi, yi, to_channel, other.get_channel(xi, yi, from_channel));
    }
  }
}

/**
 * Copies just the specified channel values from the indicated PfmFile, but
 * only where the other file has a data point.
 */
void PfmFile::
copy_channel_masked(int to_channel, const PfmFile &other, int from_channel) {
  nassertv(is_valid() && other.is_valid());
  nassertv(other._x_size == _x_size && other._y_size == _y_size);
  nassertv(to_channel >= 0 && to_channel < get_num_channels() &&
           from_channel >= 0 && from_channel < other.get_num_channels());

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      if (other.has_point(xi, yi)) {
        set_channel(xi, yi, to_channel, other.get_channel(xi, yi, from_channel));
      }
    }
  }
}

/**
 * Reduces the PFM file to the cells in the rectangle bounded by (x_begin,
 * x_end, y_begin, y_end), where the _end cells are not included.
 */
void PfmFile::
apply_crop(int x_begin, int x_end, int y_begin, int y_end) {
  nassertv(x_begin >= 0 && x_begin <= x_end && x_end <= _x_size);
  nassertv(y_begin >= 0 && y_begin <= y_end && y_end <= _y_size);

  int new_x_size = x_end - x_begin;
  int new_y_size = y_end - y_begin;
  Table new_table;
  size_t new_size = (size_t)new_x_size * (size_t)new_y_size * (size_t)_num_channels;

  // We allocate a little bit bigger to allow safe overflow: you can call
  // get_point3() or get_point4() on the last point of a 1- or 3-channel
  // image.
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

/**
 * Replaces this PfmFile with a new PfmFile of size x_size x y_size x 3,
 * containing the x y 0 values in the range 0 .. 1 according to the x y index.
 */
void PfmFile::
clear_to_texcoords(int x_size, int y_size) {
  clear(x_size, y_size, 3);

  LPoint2f uv_scale(1.0, 1.0);
  if (_x_size > 0) {
    uv_scale[0] = 1.0f / PN_float32(_x_size);
  }
  if (_y_size > 0) {
    uv_scale[1] = 1.0f / PN_float32(_y_size);
  }

  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      LPoint3f uv((PN_float32(xi) + 0.5) * uv_scale[0],
                  (PN_float32(yi) + 0.5) * uv_scale[1],
                  0.0f);
      set_point(xi, yi, uv);
    }
  }
}

/**
 * Applies delta * t to the point values within radius (xr, yr) distance of
 * (xc, yc).  The t value is scaled from 1.0 at the center to 0.0 at radius
 * (xr, yr), and this scale follows the specified exponent.  Returns the
 * number of points affected.
 */
int PfmFile::
pull_spot(const LPoint4f &delta, float xc, float yc,
          float xr, float yr, float exponent) {
  int minx = max((int)cceil(xc - xr), 0);
  int maxx = min((int)cfloor(xc + xr), _x_size - 1);
  int miny = max((int)cceil(yc - yr), 0);
  int maxy = min((int)cfloor(yc + yr), _y_size - 1);

  int count = 0;
  for (int yi = miny; yi <= maxy; ++yi) {
    for (int xi = minx; xi <= maxx; ++xi) {
      float xd = ((float)xi - xc) / xr;
      float yd = ((float)yi - yc) / yr;
      float r2 = xd * xd + yd * yd;
      if (r2 >= 1.0f) {
        continue;
      }
      PN_float32 t = (PN_float32)cpow(1.0f - csqrt(r2), exponent);

      PN_float32 *f = &_table[(yi * _x_size + xi) * _num_channels];
      for (int ci = 0; ci < _num_channels; ++ci) {
        f[ci] += delta[ci] * t;
      }
      ++count;
    }
  }

  return count;
}

/**
 * Calculates the minimum and maximum vertices of all points within the table.
 * Assumes the table contains 3-D points.
 *
 * The return value is true if any points in the table, or false if none are.
 */
bool PfmFile::
calc_tight_bounds(LPoint3f &min_point, LPoint3f &max_point) const {
  min_point.set(0.0f, 0.0f, 0.0f);
  max_point.set(0.0f, 0.0f, 0.0f);

  bool found_any = false;
  for (int yi = 0; yi < _y_size; ++yi) {
    for (int xi = 0; xi < _x_size; ++xi) {
      if (!has_point(xi, yi)) {
        continue;
      }

      const LPoint3f &point = get_point(xi, yi);
      if (!found_any) {
        min_point = point;
        max_point = point;
        found_any = true;
      } else {
        min_point.set(min(min_point[0], point[0]),
                      min(min_point[1], point[1]),
                      min(min_point[2], point[2]));
        max_point.set(max(max_point[0], point[0]),
                      max(max_point[1], point[1]),
                      max(max_point[2], point[2]));
      }
    }
  }

  return found_any;
}

/**
 * Computes the minmax bounding volume of the points in 3-D space, assuming
 * the points represent a mostly-planar surface.
 *
 * This algorithm works by sampling the (square) sample_radius pixels at the
 * four point_dist corners around the center (cx - pd, cx + pd) and so on, to
 * approximate the plane of the surface.  Then all of the points are projected
 * into that plane and the bounding volume of the entire mesh within that
 * plane is determined.  If points_only is true, the bounding volume of only
 * those four points is determined.
 *
 * center, point_dist and sample_radius are in UV space, i.e.  in the range
 * 0..1.
 */
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

  // Compute the transform necessary to rotate all of the points into the Y =
  // 0 plane.
  LMatrix4f rotate;
  look_at(rotate, normal, up);

  LMatrix4f rinv;
  rinv.invert_from(rotate);

  LPoint3f trans = pcenter * rinv;
  rinv.set_row(3, -trans);
  rotate.invert_from(rinv);

  // Now determine the minmax.
  PN_float32 min_x, min_y, min_z, max_x, max_y, max_z;
  if (points_only) {
    const LPoint3f points[4] = {
      p0 * rinv,
      p1 * rinv,
      p2 * rinv,
      p3 * rinv,
    };
    const LPoint3f &point = points[0];
    min_x = point[0];
    min_y = point[1];
    min_z = point[2];
    max_x = point[0];
    max_y = point[1];
    max_z = point[2];

    for (int i = 1; i < 4; ++i) {
      const LPoint3f &point = points[i];
      min_x = min(min_x, point[0]);
      min_y = min(min_y, point[1]);
      min_z = min(min_z, point[2]);
      max_x = max(max_x, point[0]);
      max_y = max(max_y, point[1]);
      max_z = max(max_z, point[2]);
    }
  } else {
    bool got_point = false;
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
    if (!got_point) {
      min_x = 0.0f;
      min_y = 0.0f;
      min_z = 0.0f;
      max_x = 0.0f;
      max_y = 0.0f;
      max_z = 0.0f;
    }
  }

  PT(BoundingHexahedron) bounds;

  // We create a BoundingHexahedron with the points in a particular well-
  // defined order, based on the current coordinate system.
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

  default:
    nassert_raise("invalid coordinate system");
    return nullptr;
  }

  // Rotate the bounding volume back into the original space of the screen.
  bounds->xform(LCAST(PN_stdfloat, rotate));

  return bounds;
}

/**
 * Computes the average of all the point within sample_radius (manhattan
 * distance) and the indicated point.
 *
 * The point coordinates are given in UV space, in the range 0..1.
 */
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

  case 2:
    {
      LPoint2f result2;
      box_filter_region(result2, x - xr, y - yr, x + xr, y + yr);
      result.set(result2[0], result2[1], 0.0);
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
    nassert_raise("unexpected channel count");
    break;
  }
}

/**
 * Copies a rectangular area of another image into a rectangular area of this
 * image.  Both images must already have been initialized.  The upper-left
 * corner of the region in both images is specified, and the size of the area;
 * if the size is omitted, it defaults to the entire other image, or the
 * largest piece that will fit.
 */
void PfmFile::
copy_sub_image(const PfmFile &copy, int xto, int yto,
               int xfrom, int yfrom, int x_size, int y_size) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  int x, y;
  switch (_num_channels) {
  case 1:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            set_point1(x, y, copy.get_point1(x - xmin + xfrom, y - ymin + yfrom));
          }
        }
      }
    }
    break;

  case 2:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            set_point2(x, y, copy.get_point2(x - xmin + xfrom, y - ymin + yfrom));
          }
        }
      }
    }
    break;

  case 3:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            set_point(x, y, copy.get_point(x - xmin + xfrom, y - ymin + yfrom));
          }
        }
      }
    }
    break;

  case 4:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            set_point4(x, y, copy.get_point4(x - xmin + xfrom, y - ymin + yfrom));
          }
        }
      }
    }
    break;
  }
}

/**
 * Behaves like copy_sub_image(), except the copy pixels are added to the
 * pixels of the destination, after scaling by the specified pixel_scale.
 */
void PfmFile::
add_sub_image(const PfmFile &copy, int xto, int yto,
              int xfrom, int yfrom, int x_size, int y_size,
              float pixel_scale) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  int x, y;
  switch (_num_channels) {
  case 1:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            set_point1(x, y, get_point1(x, y) + copy.get_point1(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale);
          }
        }
      }
    }
    break;

  case 2:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            set_point2(x, y, get_point2(x, y) + copy.get_point2(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale);
          }
        }
      }
    }
    break;

  case 3:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            set_point3(x, y, get_point3(x, y) + copy.get_point3(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale);
          }
        }
      }
    }
    break;

  case 4:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            set_point4(x, y, get_point4(x, y) + copy.get_point4(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale);
          }
        }
      }
    }
    break;
  }
}

/**
 * Behaves like copy_sub_image(), except the copy pixels are multiplied to the
 * pixels of the destination, after scaling by the specified pixel_scale.
 */
void PfmFile::
mult_sub_image(const PfmFile &copy, int xto, int yto,
               int xfrom, int yfrom, int x_size, int y_size,
               float pixel_scale) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  int x, y;
  switch (_num_channels) {
  case 1:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            set_point1(x, y, get_point1(x, y) * (copy.get_point1(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale));
          }
        }
      }
    }
    break;

  case 2:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            modify_point2(x, y).componentwise_mult(copy.get_point2(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale);
          }
        }
      }
    }
    break;

  case 3:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            modify_point3(x, y).componentwise_mult(copy.get_point3(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale);
          }
        }
      }
    }
    break;

  case 4:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            modify_point4(x, y).componentwise_mult(copy.get_point4(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale);
          }
        }
      }
    }
    break;
  }
}

/**
 * Behaves like copy_sub_image(), except the copy pixels are divided into the
 * pixels of the destination, after scaling by the specified pixel_scale.
 * dest(x, y) = dest(x, y) / (copy(x, y) * pixel_scale).
 */
void PfmFile::
divide_sub_image(const PfmFile &copy, int xto, int yto,
                 int xfrom, int yfrom, int x_size, int y_size,
                 float pixel_scale) {
  int xmin, ymin, xmax, ymax;
  setup_sub_image(copy, xto, yto, xfrom, yfrom, x_size, y_size,
                  xmin, ymin, xmax, ymax);

  int x, y;
  switch (_num_channels) {
  case 1:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            float val = get_point1(x, y) / copy.get_point1(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale;
            if (cnan(val)) {
              val = 0.0f;
            }
            set_point1(x, y, val);
          }
        }
      }
    }
    break;

  case 2:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            LPoint2f p = copy.get_point2(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale;
            LPoint2f &t = modify_point2(x, y);
            t[0] /= p[0];
            t[1] /= p[1];
          }
        }
      }
    }
    break;

  case 3:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            LPoint3f p = copy.get_point3(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale;
            LPoint3f &t = modify_point3(x, y);
            t[0] /= p[0];
            t[1] /= p[1];
            t[2] /= p[2];
          }
        }
      }
    }
    break;

  case 4:
    {
      for (y = ymin; y < ymax; y++) {
        for (x = xmin; x < xmax; x++) {
          if (has_point(x, y) && copy.has_point(x - xmin + xfrom, y - ymin + yfrom)) {
            LPoint4f p = copy.get_point4(x - xmin + xfrom, y - ymin + yfrom) * pixel_scale;
            LPoint4f &t = modify_point4(x, y);
            t[0] /= p[0];
            t[1] /= p[1];
            t[2] /= p[2];
            t[3] /= p[3];
          }
        }
      }
    }
    break;
  }
}

/**
 * Multiplies every point value in the image by a constant floating-point
 * multiplier value.
 */
void PfmFile::
operator *= (float multiplier) {
  nassertv(is_valid());

  switch (_num_channels) {
  case 1:
    {
      for (int y = 0; y < _y_size; ++y) {
        for (int x = 0; x < _x_size; ++x) {
          if (has_point(x, y)) {
            PN_float32 p = get_point1(x, y);
            p *= multiplier;
            set_point1(x, y, p);
          }
        }
      }
    }
    break;

  case 2:
    {
      for (int y = 0; y < _y_size; ++y) {
        for (int x = 0; x < _x_size; ++x) {
          if (has_point(x, y)) {
            LPoint2f &p = modify_point2(x, y);
            p *= multiplier;
          }
        }
      }
    }
    break;

  case 3:
    {
      for (int y = 0; y < _y_size; ++y) {
        for (int x = 0; x < _x_size; ++x) {
          if (has_point(x, y)) {
            LPoint3f &p = modify_point3(x, y);
            p *= multiplier;
          }
        }
      }
    }
    break;

  case 4:
    {
      for (int y = 0; y < _y_size; ++y) {
        for (int x = 0; x < _x_size; ++x) {
          if (has_point(x, y)) {
            LPoint4f &p = modify_point4(x, y);
            p *= multiplier;
          }
        }
      }
    }
    break;
  }
}

/**
 * index_image is a WxH 1-channel image, while pixel_values is an Nx1
 * image with any number of channels.  Typically pixel_values will be
 * a 256x1 image.
 *
 * Fills the PfmFile with a new image the same width and height as
 * index_image, with the same number of channels as pixel_values.
 *
 * Each pixel of the new image is computed with the formula:
 *
 * new_image(x, y) = pixel_values(index_image(x, y)[channel], 0)
 *
 * At present, no interpolation is performed; the nearest value in
 * pixel_values is discovered.  This may change in the future.
 */
void PfmFile::
indirect_1d_lookup(const PfmFile &index_image, int channel,
                   const PfmFile &pixel_values) {
  clear(index_image.get_x_size(), index_image.get_y_size(),
        pixel_values.get_num_channels());

  for (int yi = 0; yi < get_y_size(); ++yi) {
    switch (get_num_channels()) {
    case 1:
      for (int xi = 0; xi < get_x_size(); ++xi) {
        int v = int(index_image.get_channel(xi, yi, channel) * (pixel_values.get_x_size() - 1) + 0.5);
        nassertv(v >= 0 && v < pixel_values.get_x_size());
        set_point1(xi, yi, pixel_values.get_point1(v, 0));
      }
      break;

    case 2:
      for (int xi = 0; xi < get_x_size(); ++xi) {
        int v = int(index_image.get_channel(xi, yi, channel) * (pixel_values.get_x_size() - 1) + 0.5);
        nassertv(v >= 0 && v < pixel_values.get_x_size());
        set_point2(xi, yi, pixel_values.get_point2(v, 0));
      }
      break;

    case 3:
      for (int xi = 0; xi < get_x_size(); ++xi) {
        int v = int(index_image.get_channel(xi, yi, channel) * (pixel_values.get_x_size() - 1) + 0.5);
        nassertv(v >= 0 && v < pixel_values.get_x_size());
        set_point3(xi, yi, pixel_values.get_point3(v, 0));
      }
      break;

    case 4:
      for (int xi = 0; xi < get_x_size(); ++xi) {
        int v = int(index_image.get_channel(xi, yi, channel) * (pixel_values.get_x_size() - 1) + 0.5);
        nassertv(v >= 0 && v < pixel_values.get_x_size());
        set_point4(xi, yi, pixel_values.get_point4(v, 0));
      }
      break;
    }
  }
}

/**
 * Adjusts each channel of the image by raising the corresponding component
 * value to the indicated exponent, such that L' = L ^ exponent.
 */
void PfmFile::
apply_exponent(float c0_exponent, float c1_exponent, float c2_exponent,
               float c3_exponent) {
  switch (_num_channels) {
  case 1:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          float *val = &_table[(yi * _x_size + xi)];
          val[0] = cpow(val[0], c0_exponent);
        }
      }
    }
    break;

  case 2:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          float *val = &_table[(yi * _x_size + xi) * _num_channels];
          val[0] = cpow(val[0], c0_exponent);
          val[1] = cpow(val[1], c1_exponent);
        }
      }
    }
    break;

  case 3:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          float *val = &_table[(yi * _x_size + xi) * _num_channels];
          val[0] = cpow(val[0], c0_exponent);
          val[1] = cpow(val[1], c1_exponent);
          val[2] = cpow(val[2], c2_exponent);
        }
      }
    }
    break;

  case 4:
    {
      for (int yi = 0; yi < _y_size; ++yi) {
        for (int xi = 0; xi < _x_size; ++xi) {
          float *val = &_table[(yi * _x_size + xi) * _num_channels];
          val[0] = cpow(val[0], c0_exponent);
          val[1] = cpow(val[1], c1_exponent);
          val[2] = cpow(val[2], c2_exponent);
          val[3] = cpow(val[3], c3_exponent);
        }
      }
    }
    break;
  }
}

/**
 *
 */
void PfmFile::
output(ostream &out) const {
  out << "floating-point image: " << _x_size << " by " << _y_size << " pixels, "
      << _num_channels << " channels.";
}

/**
 * Averages all the points in the rectangle from x0 .. y0 to x1 .. y1 into
 * result.  The region may be defined by floating-point boundaries; the result
 * will be weighted by the degree of coverage of each included point.
 */
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

/**
 * Averages all the points in the rectangle from x0 .. y0 to x1 .. y1 into
 * result.  The region may be defined by floating-point boundaries; the result
 * will be weighted by the degree of coverage of each included point.
 */
void PfmFile::
box_filter_region(LPoint2f &result,
                  PN_float32 x0, PN_float32 y0, PN_float32 x1, PN_float32 y1) const {
  result = LPoint2f::zero();
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

/**
 * Averages all the points in the rectangle from x0 .. y0 to x1 .. y1 into
 * result.  The region may be defined by floating-point boundaries; the result
 * will be weighted by the degree of coverage of each included point.
 */
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

/**
 * Averages all the points in the rectangle from x0 .. y0 to x1 .. y1 into
 * result.  The region may be defined by floating-point boundaries; the result
 * will be weighted by the degree of coverage of each included point.
 */
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

/**
 *
 */
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

/**
 *
 */
void PfmFile::
box_filter_line(LPoint2f &result, PN_float32 &coverage,
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

/**
 *
 */
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

/**
 *
 */
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

/**
 *
 */
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

/**
 *
 */
void PfmFile::
box_filter_point(LPoint2f &result, PN_float32 &coverage,
                 int x, int y, PN_float32 x_contrib, PN_float32 y_contrib) const {
  if (!has_point(x, y)) {
    return;
  }
  const LPoint2f &point = get_point2(x, y);

  PN_float32 contrib = x_contrib * y_contrib;
  result += point * contrib;
  coverage += contrib;
}

/**
 *
 */
void PfmFile::
box_filter_point(LPoint3f &result, PN_float32 &coverage,
                 int x, int y, PN_float32 x_contrib, PN_float32 y_contrib) const {
  if (!has_point(x, y)) {
    return;
  }
  const LPoint3f &point = get_point3(x, y);

  PN_float32 contrib = x_contrib * y_contrib;
  result += point * contrib;
  coverage += contrib;
}

/**
 *
 */
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

/**
 * A support function for calc_average_point(), this recursively fills in the
 * holes in the mini_grid data with the index to the nearest value.
 */
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

/**
 * The implementation of has_point() for files without a no_data_value.
 */
bool PfmFile::
has_point_noop(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    return true;
  }
  return false;
}

/**
 * The implementation of has_point() for 1-component files with a
 * no_data_value.
 */
bool PfmFile::
has_point_1(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    return self->_table[(y * self->_x_size + x)] != self->_no_data_value[0];
  }
  return false;
}

/**
 * The implementation of has_point() for 2-component files with a
 * no_data_value.
 */
bool PfmFile::
has_point_2(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    return *(LPoint2f *)&self->_table[(y * self->_x_size + x) * 2] != *(LPoint2f *)&self->_no_data_value;
  }
  return false;
}

/**
 * The implementation of has_point() for 3-component files with a
 * no_data_value.
 */
bool PfmFile::
has_point_3(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    return *(LPoint3f *)&self->_table[(y * self->_x_size + x) * 3] != *(LPoint3f *)&self->_no_data_value;
  }
  return false;
}

/**
 * The implementation of has_point() for 4-component files with a
 * no_data_value.
 */
bool PfmFile::
has_point_4(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    return *(LPoint4f *)&self->_table[(y * self->_x_size + x) * 4] != *(LPoint4f *)&self->_no_data_value;
  }
  return false;
}

/**
 * The implementation of has_point_threshold() for 1-component files with a
 * no_data_value.
 */
bool PfmFile::
has_point_threshold_1(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    const float *table = &self->_table[(y * self->_x_size + x)];
    return table[0] >= self->_no_data_value[0];
  }
  return false;
}

/**
 * The implementation of has_point_threshold() for 2-component files with a
 * no_data_value.
 */
bool PfmFile::
has_point_threshold_2(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    const float *table = &self->_table[(y * self->_x_size + x) * 2];
    return (table[0] >= self->_no_data_value[0] ||
            table[1] >= self->_no_data_value[1]);
  }
  return false;
}

/**
 * The implementation of has_point_threshold() for 3-component files with a
 * no_data_value.
 */
bool PfmFile::
has_point_threshold_3(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    const float *table = &self->_table[(y * self->_x_size + x) * 3];
    return (table[0] >= self->_no_data_value[0] ||
            table[1] >= self->_no_data_value[1] ||
            table[2] >= self->_no_data_value[2]);
  }
  return false;
}

/**
 * The implementation of has_point_threshold() for 4-component files with a
 * no_data_value.
 */
bool PfmFile::
has_point_threshold_4(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    const float *table = &self->_table[(y * self->_x_size + x) * 4];
    return (table[0] >= self->_no_data_value[0] ||
            table[1] >= self->_no_data_value[1] ||
            table[2] >= self->_no_data_value[2] ||
            table[3] >= self->_no_data_value[3]);
  }
  return false;
}

/**
 * The implementation of has_point() for 4-component files with
 * set_no_data_chan4() in effect.  This means that the data is valid iff the
 * fourth channel >= 0.
 */
bool PfmFile::
has_point_chan4(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    return self->_table[(y * self->_x_size + x) * 4 + 3] >= 0.0;
  }
  return false;
}

/**
 * The implementation of has_point() for files with set_no_data_nan() in
 * effect.  This means that the data is valid iff no components involve NaN.
 */
bool PfmFile::
has_point_nan_1(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    return !cnan(self->_table[(y * self->_x_size + x) * self->_num_channels]);
  }
  return false;
}

/**
 * The implementation of has_point() for files with set_no_data_nan() in
 * effect.  This means that the data is valid iff no components involve NaN.
 */
bool PfmFile::
has_point_nan_2(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    return !((LVecBase2f *)&self->_table[(y * self->_x_size + x) * self->_num_channels])->is_nan();
  }
  return false;
}

/**
 * The implementation of has_point() for files with set_no_data_nan() in
 * effect.  This means that the data is valid iff no components involve NaN.
 */
bool PfmFile::
has_point_nan_3(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    return !((LVecBase3f *)&self->_table[(y * self->_x_size + x) * self->_num_channels])->is_nan();
  }
  return false;
}

/**
 * The implementation of has_point() for files with set_no_data_nan() in
 * effect.  This means that the data is valid iff no components involve NaN.
 */
bool PfmFile::
has_point_nan_4(const PfmFile *self, int x, int y) {
  if ((x >= 0 && x < self->_x_size) &&
      (y >= 0 && y < self->_y_size)) {
    return !((LVecBase4f *)&self->_table[(y * self->_x_size + x) * self->_num_channels])->is_nan();
  }
  return false;
}
