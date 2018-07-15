/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file stBasicTerrain.cxx
 * @author drose
 * @date 2010-10-12
 */

#include "stBasicTerrain.h"
#include "geomVertexWriter.h"
#include "pnmImage.h"
#include "indent.h"

using std::istream;
using std::ostream;
using std::string;

TypeHandle STBasicTerrain::_type_handle;

// VERTEX_ATTRIB_END is defined as a macro that must be evaluated within the
// SpeedTree namespace.
namespace SpeedTree {
  static const SVertexAttribDesc st_attrib_end = VERTEX_ATTRIB_END();
}

/* Hmm, maybe we want to use this lower-level structure directly
   instead of the GeomVertexWriter.

namespace SpeedTree {
  static const SVertexAttribDesc std_vertex_format[] = {
    { VERTEX_ATTRIB_SEMANTIC_POS, VERTEX_ATTRIB_TYPE_FLOAT, 3 },
    { VERTEX_ATTRIB_SEMANTIC_TEXCOORD0, VERTEX_ATTRIB_TYPE_FLOAT, 3 },
    VERTEX_ATTRIB_END( )
  };
  static const int std_vertex_format_length =
    sizeof(std_vertex_format) / sizeof(std_vertex_format[0]);
};
*/

/**
 *
 */
STBasicTerrain::
STBasicTerrain() {
  clear();
}

/**
 * Not sure whether any derived classes will implement the copy constructor,
 * but it's defined here at the base level just in case.
 */
STBasicTerrain::
STBasicTerrain(const STBasicTerrain &copy) :
  STTerrain(copy),
  _size(copy._size),
  _height_scale(copy._height_scale)
{
}

/**
 *
 */
STBasicTerrain::
~STBasicTerrain() {
}

/**
 * Resets the terrain to its initial, unloaded state.
 */
void STBasicTerrain::
clear() {
  STTerrain::clear();

  _height_map = "";
  _size = 1.0f;
  _height_scale = 1.0f;

  CPT(GeomVertexFormat) format = GeomVertexFormat::register_format
    (new GeomVertexArrayFormat(InternalName::get_vertex(), 3,
                               GeomEnums::NT_stdfloat, GeomEnums::C_point,
                               InternalName::get_texcoord(), 3,
                               GeomEnums::NT_stdfloat, GeomEnums::C_texcoord));
  set_vertex_format(format);
}

/**
 * Sets up the terrain by reading a terrain.txt file as defined by SpeedTree.
 * This file names the various map files that define the terrain, as well as
 * defining parameters size as its size and color.
 *
 * If a relative filename is supplied, the model-path is searched.  If a
 * directory is named, "terrain.txt" is implicitly appended.
 */
bool STBasicTerrain::
setup_terrain(const Filename &terrain_filename) {
  _is_valid = false;
  set_name(terrain_filename.get_basename());

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  Filename fullpath = Filename::text_filename(terrain_filename);
  vfs->resolve_filename(fullpath, get_model_path());

  if (!vfs->exists(fullpath)) {
    speedtree_cat.warning()
      << "Couldn't find " << terrain_filename << "\n";
    return false;
  }

  if (vfs->is_directory(fullpath)) {
    fullpath = Filename(fullpath, "terrain.txt");
  }

  istream *in = vfs->open_read_file(fullpath, true);
  if (in == nullptr) {
    speedtree_cat.warning()
      << "Couldn't open " << terrain_filename << "\n";
    return false;
  }

  bool success = setup_terrain(*in, fullpath);
  vfs->close_read_file(in);

  return success;
}

/**
 * Sets up the terrain by reading a terrain.txt file as defined by SpeedTree.
 * This variant on this method accepts an istream for an already-opened
 * terrain.txt file.  The filename is provided for reference, to assist
 * relative file operations.  It should name the terrain.txt file that has
 * been opened.
 */
bool STBasicTerrain::
setup_terrain(istream &in, const Filename &pathname) {
  clear();

  Filename dirname = pathname.get_dirname();

  string keyword;
  in >> keyword;
  while (in && !in.eof()) {
    if (keyword == "area") {
      // "area" defines the size of the terrain in square kilometers.  We
      // apply speedtree_area_scale to convert that to local units.
      PN_stdfloat area;
      in >> area;
      _size = csqrt(area) * speedtree_area_scale;

    } else if (keyword == "height_scale") {
      in >> _height_scale;

    } else if (keyword == "normalmap_b_scale") {
      PN_stdfloat normalmap_b_scale;
      in >> normalmap_b_scale;

    } else if (keyword == "heightmap") {
      read_quoted_filename(_height_map, in, dirname);

    } else if (keyword == "texture") {
      SplatLayer splat;
      read_quoted_filename(splat._filename, in, dirname);
      in >> splat._tiling;
      splat._color.set(1.0f, 1.0f, 1.0f, 1.0f);
      _splat_layers.push_back(splat);

    } else if (keyword == "color") {
      // color means the overall color of the previous texture.
      PN_stdfloat r, g, b;
      in >> r >> g >> b;
      if (!_splat_layers.empty()) {
        _splat_layers.back()._color.set(r, g, b, 1.0f);
      }

    } else if (keyword == "ambient" || keyword == "diffuse" || keyword == "specular" || keyword == "emissive") {
      PN_stdfloat r, g, b;
      in >> r >> g >> b;

    } else if (keyword == "shininess") {
      PN_stdfloat s;
      in >> s;

    } else {
      speedtree_cat.error()
        << "Invalid token " << keyword << " in " << pathname << "\n";
      return false;
    }

    in >> keyword;
  }

  // Consume any whitespace at the end of the file.
  in >> std::ws;

  if (!in.eof()) {
    // If we didn't read all the way to end-of-file, there was an error.
    in.clear();
    string text;
    in >> text;
    speedtree_cat.error()
      << "Unexpected text in " << pathname << " at \"" << text << "\"\n";
    return false;
  }

  // The first two textures are the normal map and splat map, respectively.
  if (!_splat_layers.empty()) {
    _normal_map = _splat_layers[0]._filename;
    _splat_layers.erase(_splat_layers.begin());
  }
  if (!_splat_layers.empty()) {
    _splat_map = _splat_layers[0]._filename;
    _splat_layers.erase(_splat_layers.begin());
  }

  // Now try to load the actual height map data.
  load_data();

  return _is_valid;
}

/**
 * This will be called at some point after initialization.  It should be
 * overridden by a derived class to load up the terrain data from its source
 * and fill in the data members of this class appropriately, especially
 * _is_valid.  After this call, if _is_valid is true, then get_height() etc.
 * will be called to query the terrain's data.
 */
void STBasicTerrain::
load_data() {
  _is_valid = false;

  if (!read_height_map()) {
    return;
  }

  _is_valid = true;
}

/**
 * After load_data() has been called, this should return the computed height
 * value at point (x, y) of the terrain, where x and y are unbounded and may
 * refer to any 2-d point in space.
 */
PN_stdfloat STBasicTerrain::
get_height(PN_stdfloat x, PN_stdfloat y) const {
  return _height_data.calc_bilinear_interpolation(x / _size, y / _size);
}

/**
 * After load_data() has been called, this should return the approximate
 * average height value over a circle of the specified radius, centered at
 * point (x, y) of the terrain.
 */
PN_stdfloat STBasicTerrain::
get_smooth_height(PN_stdfloat x, PN_stdfloat y, PN_stdfloat radius) const {
  return _height_data.calc_smooth(x / _size, y / _size, radius / _size);
}

/**
 * After load_data() has been called, this should return the directionless
 * slope at point (x, y) of the terrain, where 0.0 is flat and 1.0 is
 * vertical.  This is used for determining the legal points to place trees and
 * grass.
 */
PN_stdfloat STBasicTerrain::
get_slope(PN_stdfloat x, PN_stdfloat y) const {
  return _slope_data.calc_bilinear_interpolation(x / _size, y / _size);
}

/**
 * After load_data() has been called, this will be called occasionally to
 * populate the vertices for a terrain cell.
 *
 * It will be passed a GeomVertexData whose format will match
 * get_vertex_format(), and already allocated with num_xy * num_xy rows.  This
 * method should fill the rows of the data with the appropriate vertex data
 * for the terrain, over the grid described by the corners (start_x, start_y)
 * up to and including (start_x + size_x, start_y + size_xy)--a square of the
 * terrain with num_xy vertices on a size, arranged in row-major order.
 */
void STBasicTerrain::
fill_vertices(GeomVertexData *data,
              PN_stdfloat start_x, PN_stdfloat start_y,
              PN_stdfloat size_xy, int num_xy) const {
  nassertv(data->get_format() == _vertex_format);
  GeomVertexWriter vertex(data, InternalName::get_vertex());
  GeomVertexWriter texcoord(data, InternalName::get_texcoord());

  PN_stdfloat vertex_scale = 1.0 / (PN_stdfloat)(num_xy - 1);
  PN_stdfloat texcoord_scale = 1.0 / _size;
  for (int xi = 0; xi < num_xy; ++xi) {
    PN_stdfloat xt = xi * vertex_scale;
    PN_stdfloat x = start_x + xt * size_xy;
    for (int yi = 0; yi < num_xy; ++yi) {
      PN_stdfloat yt = yi * vertex_scale;
      PN_stdfloat y = start_y + yt * size_xy;

      PN_stdfloat z = get_height(x, y);

      vertex.set_data3(x, y, z);
      texcoord.set_data3(x * texcoord_scale, -y * texcoord_scale, 1.0f);
    }
  }
}

/**
 *
 */
void STBasicTerrain::
output(ostream &out) const {
  Namable::output(out);
}

/**
 *
 */
void STBasicTerrain::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << *this << "\n";
}

/**
 * Reads the height map image stored in _height_map, and stores it in
 * _height_data.  Returns true on success, false on failure.
 */
bool STBasicTerrain::
read_height_map() {
  PNMImage image(_height_map);
  if (!image.is_valid()) {
    return false;
  }

  _height_data.reset(image.get_x_size(), image.get_y_size());
  _min_height = FLT_MAX;
  _max_height = FLT_MIN;

  PN_stdfloat scalar = _size * _height_scale / image.get_num_channels();
  int pi = 0;
  for (int yi = image.get_y_size() - 1; yi >= 0; --yi) {
    for (int xi = 0; xi < image.get_x_size(); ++xi) {
      LColord rgba = image.get_xel_a(xi, yi);
      PN_stdfloat v = rgba[0] + rgba[1] + rgba[2] + rgba[3];
      v *= scalar;
      _height_data._data[pi] = v;
      ++pi;
      _min_height = std::min(_min_height, v);
      _max_height = std::max(_max_height, v);
    }
  }

  compute_slope(0.5f);

  return true;
}

/**
 * Once _height_data has been filled in, compute the corresponding values for
 * _slope_data.
 */
void STBasicTerrain::
compute_slope(PN_stdfloat smoothing) {
  nassertv(!_height_data._data.empty());

  int width = _height_data._width;
  int height = _height_data._height;
  _slope_data.reset(width, height);

  PN_stdfloat u_spacing = _size / (PN_stdfloat)width;
  PN_stdfloat v_spacing = _size / (PN_stdfloat)height;

  for (int i = 0; i < width; ++i) {
    int left = (i + width - 1) % width;
    int right = (i + 1) % width;

    for (int j = 0; j < height; ++j) {
      int top = (j + height - 1) % height;
      int bottom = (j + 1) % height;

      PN_stdfloat slope = 0.0f;
      PN_stdfloat this_height = _height_data._data[i + j * width];
      slope += catan2(cabs(this_height - _height_data._data[right + j * width]), u_spacing);
      slope += catan2(cabs(this_height - _height_data._data[left + j * width]), u_spacing);
      slope += catan2(cabs(this_height - _height_data._data[i + top * width]), v_spacing);
      slope += catan2(cabs(this_height - _height_data._data[i + bottom * width]), v_spacing);

      slope *= (0.5f / MathNumbers::pi_f);

      if (slope > 1.0f) {
        slope = 1.0f;
      }
      _slope_data._data[i + j * width] = slope;
    }
  }

  if (smoothing > 0.0f) {
    // Create a temporary array for smoothing data.
    InterpolationData<PN_stdfloat> smoothing_data;
    smoothing_data.reset(width, height);
    PN_stdfloat *smoothed = &smoothing_data._data[0];

    int steps = int(smoothing);
    PN_stdfloat last_interpolation = smoothing - steps;
    ++steps;
    for (int si = 0; si < steps; ++si) {

      // compute smoothed normals
      for (int i = 0; i < width; ++i) {
        int left = (i + width - 1) % width;
        int right = (i + 1) % width;

        for (int j = 0; j < height; ++j) {
          int top = (j + height - 1) % height;
          int bottom = (j + 1) % height;

          smoothed[i + j * width] = (_slope_data._data[right + j * width] +
                                     _slope_data._data[left + j * width] +
                                     _slope_data._data[i + top * width] +
                                     _slope_data._data[i + bottom * width] +
                                     _slope_data._data[right + top * width] +
                                     _slope_data._data[right + bottom * width] +
                                     _slope_data._data[left + top * width] +
                                     _slope_data._data[left + bottom * width]);
          smoothed[i + j * width] *= 0.125f;
        }
      }

      // interpolate or set
      if (si == steps - 1) {
        // last step, interpolate
        for (int i = 0; i < width; ++i) {
          for (int j = 0; j < height; ++j) {
            _slope_data._data[i + j * width] = interpolate(_slope_data._data[i + j * width], smoothed[i + j * width], last_interpolation);
          }
        }

      } else {
        // full smoothing step, copy everything
        _slope_data = smoothing_data;
      }
    }
  }
}

/**
 * Reads a quoted filename from the input stream, which is understood to be
 * relative to the indicated directory.
 */
void STBasicTerrain::
read_quoted_filename(Filename &result, istream &in, const Filename &dirname) {
  string filename;
  in >> filename;

  // The terrain.txt file should, in theory, support spaces, but the SpeedTree
  // reference application doesn't, so we don't bother either.
  if (filename.size() >= 2 && filename[0] == '"' && filename[filename.size() - 1] == '"') {
    filename = filename.substr(1, filename.size() - 2);
  }

  result = Filename::from_os_specific(filename);
  if (result.is_local()) {
    result = Filename(dirname, result);
  }
}
