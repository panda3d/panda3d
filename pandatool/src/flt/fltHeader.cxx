/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltHeader.cxx
 * @author drose
 * @date 2000-08-24
 */

#include "fltHeader.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltUnsupportedRecord.h"
#include "config_flt.h"
#include "zStream.h"
#include "nearly_zero.h"
#include "virtualFileSystem.h"

#include <assert.h>
#include <math.h>

TypeHandle FltHeader::_type_handle;

/**
 * The FltHeader constructor accepts a PathReplace pointer; it uses this
 * object to automatically convert all external filename and texture
 * references.  (This is necessary because the FltHeader has to look in the
 * same directory as the texture to find the .attr file, so it must pre-
 * convert at least the texture references.)
 *
 * Most of the other file converters do not have this requirement, so they do
 * not need to pre-convert any pathname references.
 */
FltHeader::
FltHeader(PathReplace *path_replace) : FltBeadID(this) {
  if (path_replace == nullptr) {
    _path_replace = new PathReplace;
    _path_replace->_path_store = PS_absolute;
  } else {
    _path_replace = path_replace;
  }

  _format_revision_level = 1570;
  _edit_revision_level = 1570;
  _next_group_id = 1;
  _next_lod_id = 1;
  _next_object_id = 1;
  _next_face_id = 1;
  _unit_multiplier = 1;
  _vertex_units = U_feet;
  _texwhite_new = false;
  _flags = 0;
  _projection_type = PT_flat_earth;
  _next_dof_id = 1;
  _vertex_storage_type = VTS_double;
  _database_origin = DO_open_flight;
  _sw_x = 0.0;
  _sw_y = 0.0;
  _delta_x = 0.0;
  _delta_y = 0.0;
  _next_sound_id = 1;
  _next_path_id = 1;
  _next_clip_id = 1;
  _next_text_id = 1;
  _next_bsp_id = 1;
  _next_switch_id = 1;
  _sw_lat = 0.0;
  _sw_long = 0.0;
  _ne_lat = 0.0;
  _ne_long = 0.0;
  _origin_lat = 0.0;
  _origin_long = 0.0;
  _lambert_upper_lat = 0.0;
  _lambert_lower_lat = 0.0;
  _next_light_id = 1;
  _next_road_id = 1;
  _next_cat_id = 1;

  // New with 15.2
  _earth_model = EM_wgs84;

  // New with 15.6
  _next_adaptive_id = 0;
  _next_curve_id = 0;

  // New with 15.7
  _delta_z = 0.0;
  _radius = 0.0;
  _next_mesh_id = 0;

  _vertex_lookups_stale = false;
  _current_vertex_offset = 0;
  _next_material_index = 1;
  _next_pattern_index = 1;
  _got_color_palette = false;
  _got_14_material_palette = false;
  _got_eyepoint_trackplane_palette = false;

  _auto_attr_update = AU_if_missing;
}

/**
 * Walks the hierarchy at this record and below and copies the
 * _converted_filename record into the _orig_filename record, so the flt file
 * will be written out with the converted filename instead of what was
 * originally read in.
 */
void FltHeader::
apply_converted_filenames() {
  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    FltTexture *texture = (*ti).second;
    texture->apply_converted_filenames();
  }

  FltBeadID::apply_converted_filenames();
}

/**
 * Replaces the PathReplace object (which specifies how to mangle paths from
 * the source to the destination file) with a new one.
 */
void FltHeader::
set_path_replace(PathReplace *path_replace) {
  _path_replace = path_replace;
}

/**
 * Returns a pointer to the PathReplace object associated with this converter.
 * If the converter is non-const, this returns a non-const pointer, which can
 * be adjusted.
 */
PathReplace *FltHeader::
get_path_replace() {
  return _path_replace;
}

/**
 * Returns a pointer to the PathReplace object associated with this converter.
 * If the converter is non-const, this returns a non-const pointer, which can
 * be adjusted.
 */
const PathReplace *FltHeader::
get_path_replace() const {
  return _path_replace;
}

/**
 * Uses the PathReplace object to convert the named filename as read from the
 * flt record to its actual name.
 */
Filename FltHeader::
convert_path(const Filename &orig_filename, const DSearchPath &additional_path) {
  DSearchPath file_path;
  if (!_flt_filename.empty()) {
    file_path.append_directory(_flt_filename.get_dirname());
  }
  file_path.append_path(additional_path);
  return _path_replace->convert_path(orig_filename, file_path);
}

/**
 * Sets the filename--especially the directory part--in which the flt file is
 * considered to reside.  This is also implicitly set by read_flt().
 */
void FltHeader::
set_flt_filename(const Filename &flt_filename) {
  _flt_filename = flt_filename;
}

/**
 * Returns the directory in which the flt file is considered to reside.
 */
const Filename &FltHeader::
get_flt_filename() const {
  return _flt_filename;
}

/**
 * Opens the indicated filename for reading and attempts to read the complete
 * Flt file.  Returns FE_ok on success, otherwise on failure.
 */
FltError FltHeader::
read_flt(Filename filename) {
  filename.set_binary();
  _flt_filename = filename;

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  std::istream *in = vfs->open_read_file(filename, true);
  if (in == nullptr) {
    assert(!flt_error_abort);
    return FE_could_not_open;
  }
  FltError result = read_flt(*in);
  vfs->close_read_file(in);
  return result;
}

/**
 * Attempts to read a complete Flt file from the already-opened stream.
 * Returns FE_ok on success, otherwise on failure.
 */
FltError FltHeader::
read_flt(std::istream &in) {
  FltRecordReader reader(in);
  FltError result = reader.advance();
  if (result == FE_end_of_file) {
    assert(!flt_error_abort);
    return FE_empty_file;
  } else if (result != FE_ok) {
    return result;
  }

  result = read_record_and_children(reader);
  if (result != FE_ok) {
    return result;
  }

  if (!reader.eof()) {
    assert(!flt_error_abort);
    return FE_extra_data;
  }

  return FE_ok;
}


/**
 * Opens the indicated filename for writing and attempts to write the complete
 * Flt file.  Returns FE_ok on success, otherwise on failure.
 */
FltError FltHeader::
write_flt(Filename filename) {
  filename.set_binary();

  std::ofstream out;
  if (!filename.open_write(out)) {
    assert(!flt_error_abort);
    return FE_could_not_open;
  }

#ifdef HAVE_ZLIB
  if (filename.get_extension() == "pz") {
    // The filename ends in .pz, which means to automatically compress the flt
    // file that we write.
    OCompressStream compressor(&out, false);
    return write_flt(compressor);
  }
#endif  // HAVE_ZLIB

  return write_flt(out);
}

/**
 * Attempts to write a complete Flt file to the already-opened stream.
 * Returns FE_ok on success, otherwise on failure.
 */
FltError FltHeader::
write_flt(std::ostream &out) {
  FltRecordWriter writer(out);
  FltError result = write_record_and_children(writer);

  if (out.fail()) {
    assert(!flt_error_abort);
    return FE_write_error;
  }
  return result;
}

/**
 * Controls whether texture .attr files are written automatically when
 * write_flt() is called.  There are three possibilities:
 *
 * AU_none: the .attr files are not written automatically; they must be
 * written explicitly via a call to FltTexture::write_attr_data() if you want
 * them to be written.
 *
 * AU_if_missing: the .attr files are written only if they do not already
 * exist.  This will not update any .attr files, even if the data is changed.
 *
 * AU_always: the .attr files are always rewritten, even if they already exist
 * and even if the data has not changed.
 *
 * The default is AU_if_missing.
 */
void FltHeader::
set_auto_attr_update(FltHeader::AttrUpdate attr) {
  _auto_attr_update = attr;
}

/**
 * Returns the current setting of the auto_attr_update flag.  See
 * sett_auto_attr_update().
 */
FltHeader::AttrUpdate FltHeader::
get_auto_attr_update() const {
  return _auto_attr_update;
}

/**
 * Returns the version number of the flt file as reported in the header, times
 * 100.  Divide by 100 to get the floating-point version number.
 */
int FltHeader::
get_flt_version() const {
  if (_format_revision_level < 1420) {
    return _format_revision_level * 100;
  } else {
    return _format_revision_level;
  }
}

/**
 * Changes the version number of the flt file that will be reported in the
 * header.  Pass in the floating-point version number times 100.
 */
void FltHeader::
set_flt_version(int version) {
  if (version < 14.2) {
    _format_revision_level = version / 100;
  } else {
    _format_revision_level = version;
  }
}

/**
 * Returns the earliest flt version number that this codebase supports (times
 * 100).  Earlier versions will probably not work.
 */
int FltHeader::
min_flt_version() {
  return 1400;
}

/**
 * Returns the latest flt version number that this codebase is known to
 * support (times 100).  Later versions might work, but then again they may
 * not.
 */
int FltHeader::
max_flt_version() {
  return 1570;
}

/**
 * Verifies that the version number read from the header is an understood
 * version number, and prints a warning to the user if this is not so--the
 * read may or may not succeed.  Returns true if the version number is
 * acceptable (and no warning is printed), or false if it is questionable (and
 * a warning is printed).
 */
bool FltHeader::
check_version() const {
  int version = get_flt_version();

  if (version < min_flt_version()) {
    nout << "Warning!  The version number of this file appears to be "
         << version / 100.0 << ", which is older than " << min_flt_version() / 100.0
         << ", the oldest OpenFlight version understood by this program.  "
      "It is unlikely that this program will be able to read the file "
      "correctly.\n";
    return false;
  }

  if (version > max_flt_version()) {
    nout << "Warning!  The version number of this file appears to be "
         << version / 100.0 << ", which is newer than " << max_flt_version() / 100.0
         << ", the newest OpenFlight version understood by this program.  "
      "Chances are good that the program will still be able to read it "
      "correctly, but any features in the file that are specific to "
      "the latest version of OpenFlight will not be understood.\n";
    return false;
  }

  return true;
}

/**
 * Returns the units indicated by the flt header, or DU_invalid if the units
 * in the header are not understood.
 */
DistanceUnit FltHeader::
get_units() const {
  switch (_vertex_units) {
  case FltHeader::U_meters:
    return DU_meters;

  case FltHeader::U_kilometers:
    return DU_kilometers;

  case FltHeader::U_feet:
    return DU_feet;

  case FltHeader::U_inches:
    return DU_inches;

  case FltHeader::U_nautical_miles:
    return DU_nautical_miles;
  }

  // Unknown units.
  return DU_invalid;
}

/**
 * Returns true if a instance subtree with the given index has been defined.
 */
bool FltHeader::
has_instance(int instance_index) const {
  return (_instances.count(instance_index) != 0);
}

/**
 * Returns the instance subtree associated with the given index, or NULL if
 * there is no such instance.
 */
FltInstanceDefinition *FltHeader::
get_instance(int instance_index) const {
  Instances::const_iterator mi;
  mi = _instances.find(instance_index);
  if (mi != _instances.end()) {
    return (*mi).second;
  }
  return nullptr;
}

/**
 * Removes all instance subtrees from the instance pool.
 */
void FltHeader::
clear_instances() {
  _instances.clear();
}

/**
 * Defines a new instance subtree.  This subtree is not itself part of the
 * hierarchy; it marks geometry that may be instanced to various beads
 * elsewhere in the hierarchy by creating a corresponding FltInstanceRef bead.
 */
void FltHeader::
add_instance(FltInstanceDefinition *instance) {
  _instances[instance->_instance_index] = instance;
}

/**
 * Removes a particular instance subtree from the pool, if it exists.
 */
void FltHeader::
remove_instance(int instance_index) {
  _instances.erase(instance_index);
}

/**
 * Returns the number of vertices in the vertex palette.
 */
int FltHeader::
get_num_vertices() const {
  return _vertices.size();
}

/**
 * Returns the nth vertex of the vertex palette.
 */
FltVertex *FltHeader::
get_vertex(int n) const {
  nassertr(n >= 0 && n < (int)_vertices.size(), nullptr);
  return _vertices[n];
}

/**
 * Removes all vertices from the vertex palette.
 */
void FltHeader::
clear_vertices() {
  _vertices.clear();
  _unique_vertices.clear();
  _vertices_by_offset.clear();
  _offsets_by_vertex.clear();
  _vertex_lookups_stale = false;
}

/**
 * Adds a new vertex to the end of the vertex palette.  If this particular
 * vertex was already present in the palette, does nothing.
 */
void FltHeader::
add_vertex(FltVertex *vertex) {
  bool inserted = _unique_vertices.insert(vertex).second;
  if (inserted) {
    _vertices.push_back(vertex);
  }
  _vertex_lookups_stale = true;
  nassertv(_unique_vertices.size() == _vertices.size());
}

/**
 * Returns the particular vertex pointer associated with the given byte offset
 * into the vertex palette.  If there is no such vertex in the palette, this
 * generates an error message and returns NULL.
 */
FltVertex *FltHeader::
get_vertex_by_offset(int offset) {
  if (_vertex_lookups_stale) {
    update_vertex_lookups();
  }

  VerticesByOffset::const_iterator vi;
  vi = _vertices_by_offset.find(offset);
  if (vi == _vertices_by_offset.end()) {
    nout << "No vertex with offset " << offset << "\n";
    return nullptr;
  }
  return (*vi).second;
}

/**
 * Returns the byte offset into the vertex palette associated with the given
 * vertex pointer.  If there is no such vertex in the palette, this generates
 * an error message and returns 0.
 */
int FltHeader::
get_offset_by_vertex(FltVertex *vertex) {
  if (_vertex_lookups_stale) {
    update_vertex_lookups();
  }

  OffsetsByVertex::const_iterator vi;
  vi = _offsets_by_vertex.find(vertex);
  if (vi == _offsets_by_vertex.end()) {
    nout << "Vertex does not appear in palette.\n";
    return 0;
  }
  return (*vi).second;
}

/**
 * Returns the total number of different colors in the color palette.  This
 * includes all different colors, and represents the complete range of
 * alloable color indices.  This is different from the actual number of color
 * entries as read directly from the color palette, since each color entry
 * defines a number of different intensity levels--the value returned by
 * get_num_colors() is equal to get_num_color_entries() *
 * get_num_color_shades().
 */
int FltHeader::
get_num_colors() const {
  return _colors.size() * get_num_color_shades();
}

/**
 * Returns the four-component color corresponding to the given color index.
 * Each component will be in the range [0, 1].
 */
LColor FltHeader::
get_color(int color_index) const {
  nassertr(color_index >= 0 && color_index < get_num_colors(),
           LColor(0.0, 0.0, 0.0, 0.0));
  int num_color_shades = get_num_color_shades();

  int index = (color_index / num_color_shades);
  int level = (color_index % num_color_shades);
  nassertr(index >= 0 && index < (int)_colors.size(),
           LColor(0.0, 0.0, 0.0, 0.0));

  LColor color = _colors[index].get_color();
  return color * ((double)level / (double)(num_color_shades - 1));
}

/**
 * Returns the three-component color corresponding to the given color index,
 * ignoring the alpha component.  Each component will be in the range [0, 1].
 */
LRGBColor FltHeader::
get_rgb(int color_index) const {
  nassertr(color_index >= 0 && color_index < get_num_colors(),
           LRGBColor(0.0, 0.0, 0.0));
  int num_color_shades = get_num_color_shades();

  int index = (color_index / num_color_shades);
  int level = (color_index % num_color_shades);
  nassertr(index >= 0 && index < (int)_colors.size(),
           LRGBColor(0.0, 0.0, 0.0));

  LRGBColor color = _colors[index].get_rgb();
  return color * ((double)level / (double)(num_color_shades - 1));
}

/**
 * Returns true if the given color is named, false otherwise.
 */
bool FltHeader::
has_color_name(int color_index) const {
  return (_color_names.count(color_index) != 0);
}

/**
 * Returns the name associated with the given color, if any.
 */
std::string FltHeader::
get_color_name(int color_index) const {
  ColorNames::const_iterator ni;
  ni = _color_names.find(color_index);
  if (ni != _color_names.end()) {
    return (*ni).second;
  }
  return std::string();
}

/**
 * Returns the color index of the nearest color in the palette that matches
 * the given four-component color, including alpha.
 */
int FltHeader::
get_closest_color(const LColor &color0) const {
  // Since the colortable stores the brightest colors, with num_color_shades
  // scaled versions of each color implicitly available, we really only care
  // about the relative brightnesses of the various components.  Normalize the
  // color in terms of the largest of these.
  LColor color = color0;

  double scale = 1.0;

  if (color[0] == 0.0 && color[1] == 0.0 && color[2] == 0.0 && color[3] == 0.0) {
    // Oh, this is invisible black.
    scale = 0.0;
    color.set(1.0, 1.0, 1.0, 1.0);

  } else {
    if (color[0] >= color[1] && color[0] >= color[2] && color[0] >= color[3]) {
      // color[0] is largest.
      scale = color[0];

    } else if (color[1] >= color[2] && color[1] >= color[3]) {
      // color[1] is largest.
      scale = color[1];

    } else if (color[2] >= color[3]) {
      // color[2] is largest.
      scale = color[2];

    } else {
      // color[3] is largest.
      scale = color[3];
    }
    color /= scale;
  }

  // Now search for the best match.
  PN_stdfloat best_dist = 5.0;  // Greater than 4.
  int best_i = -1;

  int num_color_entries = get_num_color_entries();
  for (int i = 0; i < num_color_entries; i++) {
    LColor consider = _colors[i].get_color();
    PN_stdfloat dist2 = dot(consider - color, consider - color);
    nassertr(dist2 < 5.0, 0);

    if (dist2 < best_dist) {
      best_dist = dist2;
      best_i = i;
    }
  }
  nassertr(best_i >= 0, 0);

  int num_color_shades = get_num_color_shades();
  int shade_index = (int)floor((num_color_shades-1) * scale + 0.5);

  return (best_i * num_color_shades) + shade_index;
}

/**
 * Returns the color index of the nearest color in the palette that matches
 * the given three-component color, ignoring alpha.
 */
int FltHeader::
get_closest_rgb(const LRGBColor &color0) const {
  // Since the colortable stores the brightest colors, with num_color_shades
  // scaled versions of each color implicitly available, we really only care
  // about the relative brightnesses of the various components.  Normalize the
  // color in terms of the largest of these.

  LRGBColor color = color0;
  double scale = 1.0;

  if (color[0] == 0.0 && color[1] == 0.0 && color[2] == 0.0) {
    // Oh, this is black.
    scale = 0.0;
    color.set(1.0, 1.0, 1.0);

  } else {
    if (color[0] >= color[1] && color[0] >= color[2]) {
      // color[0] is largest.
      scale = color[0];

    } else if (color[1] >= color[2]) {
      // color[1] is largest.
      scale = color[1];

    } else {
      // color[2] is largest.
      scale = color[2];
    }
    color /= scale;
  }

  // Now search for the best match.
  PN_stdfloat best_dist = 5.0;  // Greater than 4.
  int best_i = -1;

  int num_color_entries = get_num_color_entries();
  for (int i = 0; i < num_color_entries; i++) {
    LRGBColor consider = _colors[i].get_rgb();
    PN_stdfloat dist2 = dot(consider - color, consider - color);
    nassertr(dist2 < 5.0, 0);

    if (dist2 < best_dist) {
      best_dist = dist2;
      best_i = i;
    }
  }
  nassertr(best_i >= 0, 0);

  int num_color_shades = get_num_color_shades();
  int shade_index = (int)floor((num_color_shades-1) * scale + 0.5);

  return (best_i * num_color_shades) + shade_index;
}

/**
 * Returns the number of actual entries in the color palette.  This is based
 * on the version of the flt file, and is usually either 512 or 1024.
 */
int FltHeader::
get_num_color_entries() const {
  return _colors.size();
}

/**
 * Returns the number of shades of brightness of each entry in the color
 * palette.  This is a fixed property of MultiGen files: each entry in the
 * palette actually represents a range of this many colors.
 */
int FltHeader::
get_num_color_shades() const {
  return 128;
}

/**
 * Decodes a MultiGen color, as stored on a face or vertex, into an actual
 * four-component LColor.  Normally you need not call this directly; there are
 * color accessors defined on faces and vertices that do this.
 */
LColor FltHeader::
get_color(int color_index, bool use_packed_color,
          const FltPackedColor &packed_color,
          int transparency) {
  if (!use_packed_color) {
    return get_color(color_index);
  }

  LColor color;
  color[0] = packed_color._r / 255.0;
  color[1] = packed_color._g / 255.0;
  color[2] = packed_color._b / 255.0;
  // MultiGen doesn't yet use the A component of RGBA. color[3] =
  // packed_color._a  255.0;
  color[3] = 1.0 - (transparency / 65535.0);
  return color;
}

/**
 * Decodes a MultiGen color, as stored on a face or vertex, into an actual
 * three-component LRGBColor.  Normally you need not call this directly; there
 * are color accessors defined on faces and vertices that do this.
 */
LRGBColor FltHeader::
get_rgb(int color_index, bool use_packed_color,
        const FltPackedColor &packed_color) {
  if (!use_packed_color) {
    return get_rgb(color_index);
  }

  LRGBColor color;
  color[0] = packed_color._r / 255.0;
  color[1] = packed_color._g / 255.0;
  color[2] = packed_color._b / 255.0;
  return color;
}

/**
 * Returns true if a material with the given index has been defined.
 */
bool FltHeader::
has_material(int material_index) const {
  return (_materials.count(material_index) != 0);
}

/**
 * Returns the material associated with the given index, or NULL if there is
 * no such material.
 */
FltMaterial *FltHeader::
get_material(int material_index) const {
  Materials::const_iterator mi;
  mi = _materials.find(material_index);
  if (mi != _materials.end()) {
    return (*mi).second;
  }
  return nullptr;
}

/**
 * Removes all materials from the palette.
 */
void FltHeader::
clear_materials() {
  _materials.clear();
}

/**
 * Defines a new material.  The material is added in the position indicated by
 * the material's index number.  If there is already a material defined for
 * that index number, it is replaced.
 */
void FltHeader::
add_material(FltMaterial *material) {
  if (material->_material_index < 0) {
    // We need to make up a new material index for the material.
    material->_material_index = _next_material_index;
    _next_material_index++;

  } else {
    // Make sure our next generated material index will be different from any
    // existing material indices.
    _next_material_index = std::max(_next_material_index, material->_material_index + 1);
  }

  _materials[material->_material_index] = material;
}

/**
 * Removes a particular material from the material palette, if it exists.
 */
void FltHeader::
remove_material(int material_index) {
  _materials.erase(material_index);
}

/**
 * Returns true if a texture with the given index has been defined.
 */
bool FltHeader::
has_texture(int texture_index) const {
  return (_textures.count(texture_index) != 0);
}

/**
 * Returns the texture associated with the given index, or NULL if there is no
 * such texture.
 */
FltTexture *FltHeader::
get_texture(int texture_index) const {
  Textures::const_iterator mi;
  mi = _textures.find(texture_index);
  if (mi != _textures.end()) {
    return (*mi).second;
  }
  return nullptr;
}

/**
 * Removes all textures from the palette.
 */
void FltHeader::
clear_textures() {
  _textures.clear();
}

/**
 * Defines a new texture.  The texture is added in the position indicated by
 * the texture's index number.  If there is already a texture defined for that
 * index number, it is replaced.
 */
void FltHeader::
add_texture(FltTexture *texture) {
  if (texture->_pattern_index < 0) {
    // We need to make up a new pattern index for the texture.
    texture->_pattern_index = _next_pattern_index;
    _next_pattern_index++;

  } else {
    // Make sure our next generated pattern index will be different from any
    // existing texture indices.
    _next_pattern_index = std::max(_next_pattern_index, texture->_pattern_index + 1);
  }

  _textures[texture->_pattern_index] = texture;
}

/**
 * Removes a particular texture from the texture palette, if it exists.
 */
void FltHeader::
remove_texture(int texture_index) {
  _textures.erase(texture_index);
}

/**
 * Returns true if a light source with the given index has been defined.
 */
bool FltHeader::
has_light_source(int light_index) const {
  return (_light_sources.count(light_index) != 0);
}

/**
 * Returns the light source associated with the given index, or NULL if there
 * is no such light source.
 */
FltLightSourceDefinition *FltHeader::
get_light_source(int light_index) const {
  LightSources::const_iterator li;
  li = _light_sources.find(light_index);
  if (li != _light_sources.end()) {
    return (*li).second;
  }
  return nullptr;
}

/**
 * Removes all light sources from the palette.
 */
void FltHeader::
clear_light_sources() {
  _light_sources.clear();
}

/**
 * Defines a new light source.  The light source is added in the position
 * indicated by its light index number.  If there is already a light source
 * defined for that index number, it is replaced.
 */
void FltHeader::
add_light_source(FltLightSourceDefinition *light_source) {
  _light_sources[light_source->_light_index] = light_source;
}

/**
 * Removes a particular light source from the light source palette, if it
 * exists.
 */
void FltHeader::
remove_light_source(int light_index) {
  _light_sources.erase(light_index);
}

/**
 * Returns true if we have read an eyepoint/trackplane palette, and at least
 * some of the eyepoints and trackplanes are therefore expected to be
 * meaningful.
 */
bool FltHeader::
got_eyepoint_trackplane_palette() const {
  return _got_eyepoint_trackplane_palette;
}

/**
 * Sets the state of the eyepoint/trackplane palette flag.  When this is
 * false, the palette is believed to be meaningless, and will not be written;
 * when it is true, the palette is believed to contain at least some
 * meaningful data, and will be written.
 */
void FltHeader::
set_eyepoint_trackplane_palette(bool flag) {
  _got_eyepoint_trackplane_palette = flag;
}

/**
 * Returns the number of eyepoints in the eyepoint/trackplane palette.  This
 * is presently fixed at 10, according to the MultiGen specs.
 */
int FltHeader::
get_num_eyepoints() const {
  return 10;
}

/**
 * Returns the nth eyepoint in the eyepoint/trackplane palette.
 */
FltEyepoint *FltHeader::
get_eyepoint(int n) {
  nassertr(n >= 0 && n < get_num_eyepoints(), nullptr);
  return &_eyepoints[n];
}

/**
 * Returns the number of trackplanes in the eyepoint/trackplane palette.  This
 * is presently fixed at 10, according to the MultiGen specs.
 */
int FltHeader::
get_num_trackplanes() const {
  return 10;
}

/**
 * Returns the nth trackplane in the eyepoint/trackplane palette.
 */
FltTrackplane *FltHeader::
get_trackplane(int n) {
  nassertr(n >= 0 && n < get_num_trackplanes(), nullptr);
  return &_trackplanes[n];
}

/**
 * Recomputes the offsets_by_vertex and vertices_by_offset tables.  This
 * reflects the flt file as it will be written out, but not necessarily as it
 * was read in.
 *
 * The return value is the total length of the vertex palette, including the
 * header record.
 */
int FltHeader::
update_vertex_lookups() {
  // We start with the length of the vertex palette record itself.
  int offset = 8;

  Vertices::const_iterator vi;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    FltVertex *vertex = (*vi);

    _offsets_by_vertex[vertex] = offset;
    _vertices_by_offset[offset] = vertex;
    offset += vertex->get_record_length();
  }

  _vertex_lookups_stale = false;

  return offset;
}

/**
 * Fills in the information in this bead based on the information given in the
 * indicated datagram, whose opcode has already been read.  Returns true on
 * success, false if the datagram is invalid.
 */
bool FltHeader::
extract_record(FltRecordReader &reader) {
  if (!FltBeadID::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_header, false);
  DatagramIterator &iterator = reader.get_iterator();

  _format_revision_level = iterator.get_be_int32();
  _edit_revision_level = iterator.get_be_int32();
  _last_revision = iterator.get_fixed_string(32);
  _next_group_id = iterator.get_be_int16();
  _next_lod_id = iterator.get_be_int16();
  _next_object_id = iterator.get_be_int16();
  _next_face_id = iterator.get_be_int16();
  _unit_multiplier = iterator.get_be_int16();
  _vertex_units = (Units)iterator.get_int8();
  _texwhite_new = (iterator.get_int8() != 0);
  _flags = iterator.get_be_uint32();
  iterator.skip_bytes(24);
  _projection_type = (ProjectionType)iterator.get_be_int32();
  iterator.skip_bytes(28);
  _next_dof_id = iterator.get_be_int16();
  _vertex_storage_type = (VertexStorageType)iterator.get_be_int16();
  _database_origin = (DatabaseOrigin)iterator.get_be_int32();
  _sw_x = iterator.get_be_float64();
  _sw_y = iterator.get_be_float64();
  _delta_x = iterator.get_be_float64();
  _delta_y = iterator.get_be_float64();
  _next_sound_id = iterator.get_be_int16();
  _next_path_id = iterator.get_be_int16();
  iterator.skip_bytes(8);
  _next_clip_id = iterator.get_be_int16();
  _next_text_id = iterator.get_be_int16();
  _next_bsp_id = iterator.get_be_int16();
  _next_switch_id = iterator.get_be_int16();
  iterator.skip_bytes(4);
  _sw_lat = iterator.get_be_float64();
  _sw_long = iterator.get_be_float64();
  _ne_lat = iterator.get_be_float64();
  _ne_long = iterator.get_be_float64();
  _origin_lat = iterator.get_be_float64();
  _origin_long = iterator.get_be_float64();
  _lambert_upper_lat = iterator.get_be_float64();
  _lambert_lower_lat = iterator.get_be_float64();
  _next_light_id = iterator.get_be_int16();
  iterator.skip_bytes(2);
  if (get_flt_version() >= 1420 && iterator.get_remaining_size() > 0) {
    _next_road_id = iterator.get_be_int16();
    _next_cat_id = iterator.get_be_int16();

    if (get_flt_version() >= 1520 && iterator.get_remaining_size() > 0) {
      iterator.skip_bytes(2 + 2 + 2 + 2);
      _earth_model = (EarthModel)iterator.get_be_int32();

      // Undocumented padding.
      iterator.skip_bytes(4);

      if (get_flt_version() >= 1560 && iterator.get_remaining_size() > 0) {
        _next_adaptive_id = iterator.get_be_int16();
        _next_curve_id = iterator.get_be_int16();
        iterator.skip_bytes(4);

        if (get_flt_version() >= 1570 && iterator.get_remaining_size() > 0) {
          _delta_z = iterator.get_be_float64();
          _radius = iterator.get_be_float64();
          _next_mesh_id = iterator.get_be_int16();
          iterator.skip_bytes(2);

          // Undocumented padding.
          iterator.skip_bytes(4);
        }
      }
    }
  }

  check_remaining_size(iterator);
  return true;
}

/**
 * Checks whether the given bead, which follows this bead sequentially in the
 * file, is an ancillary record of this bead.  If it is, extracts the relevant
 * information and returns true; otherwise, leaves it alone and returns false.
 */
bool FltHeader::
extract_ancillary(FltRecordReader &reader) {
  switch (reader.get_opcode()) {
  case FO_vertex_palette:
    // We're about to begin the vertex palette!
    clear_vertices();
    _current_vertex_offset = reader.get_record_length();
    return true;

  case FO_vertex_c:
  case FO_vertex_cn:
  case FO_vertex_cnu:
  case FO_vertex_cu:
    // Here's a new vertex for the palette.
    return extract_vertex(reader);

  case FO_color_palette:
    return extract_color_palette(reader);

  case FO_15_material:
    return extract_material(reader);

  case FO_14_material_palette:
    return extract_14_material_palette(reader);

  case FO_texture:
    return extract_texture(reader);

  case FO_texture_map_palette:
    return extract_texture_map(reader);

  case FO_light_definition:
    return extract_light_source(reader);

  case FO_eyepoint_palette:
    return extract_eyepoint_palette(reader);

  default:
    return FltBeadID::extract_ancillary(reader);
  }
}

/**
 * Fills up the current record on the FltRecordWriter with data for this
 * record, but does not advance the writer.  Returns true on success, false if
 * there is some error.
 */
bool FltHeader::
build_record(FltRecordWriter &writer) const {
  if (!FltBeadID::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_header);
  Datagram &datagram = writer.update_datagram();

  datagram.add_be_int32(_format_revision_level);
  datagram.add_be_int32(_edit_revision_level);
  datagram.add_fixed_string(_last_revision, 32);
  datagram.add_be_int16(_next_group_id);
  datagram.add_be_int16(_next_lod_id);
  datagram.add_be_int16(_next_object_id);
  datagram.add_be_int16(_next_face_id);
  datagram.add_be_int16(_unit_multiplier);
  datagram.add_int8(_vertex_units);
  datagram.add_int8(_texwhite_new);
  datagram.add_be_uint32(_flags);
  datagram.pad_bytes(24);
  datagram.add_be_int32(_projection_type);
  datagram.pad_bytes(28);
  datagram.add_be_int16(_next_dof_id);
  datagram.add_be_int16(_vertex_storage_type);
  datagram.add_be_int32(_database_origin);
  datagram.add_be_float64(_sw_x);
  datagram.add_be_float64(_sw_y);
  datagram.add_be_float64(_delta_x);
  datagram.add_be_float64(_delta_y);
  datagram.add_be_int16(_next_sound_id);
  datagram.add_be_int16(_next_path_id);
  datagram.pad_bytes(8);
  datagram.add_be_int16(_next_clip_id);
  datagram.add_be_int16(_next_text_id);
  datagram.add_be_int16(_next_bsp_id);
  datagram.add_be_int16(_next_switch_id);
  datagram.pad_bytes(4);
  datagram.add_be_float64(_sw_lat);
  datagram.add_be_float64(_sw_long);
  datagram.add_be_float64(_ne_lat);
  datagram.add_be_float64(_ne_long);
  datagram.add_be_float64(_origin_lat);
  datagram.add_be_float64(_origin_long);
  datagram.add_be_float64(_lambert_upper_lat);
  datagram.add_be_float64(_lambert_lower_lat);
  datagram.add_be_int16(_next_light_id);
  datagram.pad_bytes(2);
  datagram.add_be_int16(_next_road_id);
  datagram.add_be_int16(_next_cat_id);

  if (get_flt_version() >= 1520) {
    // New with 15.2
    datagram.pad_bytes(2 + 2 + 2 + 2);
    datagram.add_be_int32(_earth_model);

    datagram.pad_bytes(4);

    if (get_flt_version() >= 1560) {
      // New with 15.6
      datagram.add_be_int16(_next_adaptive_id);
      datagram.add_be_int16(_next_curve_id);
      datagram.pad_bytes(4);

      if (get_flt_version() >= 1570) {
        // New with 15.7
        datagram.add_be_float64(_delta_z);
        datagram.add_be_float64(_radius);
        datagram.add_be_int16(_next_mesh_id);
        datagram.pad_bytes(2);
        datagram.pad_bytes(4);
      }
    }
  }

  return true;
}

/**
 * Writes whatever ancillary records are required for this bead.  Returns
 * FE_ok on success, or something else on error.
 */
FltError FltHeader::
write_ancillary(FltRecordWriter &writer) const {
  FltError result;

  result = write_color_palette(writer);
  if (result != FE_ok) {
    return result;
  }

  result = write_material_palette(writer);
  if (result != FE_ok) {
    return result;
  }

  result = write_texture_palette(writer);
  if (result != FE_ok) {
    return result;
  }

  result = write_light_source_palette(writer);
  if (result != FE_ok) {
    return result;
  }

  result = write_eyepoint_palette(writer);
  if (result != FE_ok) {
    return result;
  }

  result = write_vertex_palette(writer);
  if (result != FE_ok) {
    return result;
  }

  return FltBeadID::write_ancillary(writer);
}

/**
 * Reads a single vertex ancillary record.  It is assumed that all the vertex
 * records will immediately follow the vertex palette record.
 */
bool FltHeader::
extract_vertex(FltRecordReader &reader) {
  FltVertex *vertex = new FltVertex(this);
  if (!vertex->extract_record(reader)) {
    return false;
  }
  _vertices.push_back(vertex);
  _unique_vertices.insert(vertex);
  _offsets_by_vertex[vertex] = _current_vertex_offset;
  _vertices_by_offset[_current_vertex_offset] = vertex;
  _current_vertex_offset += reader.get_record_length();

  // _vertex_lookups_stale remains false.

  return true;
}

/**
 * Reads the color palette.
 */
bool FltHeader::
extract_color_palette(FltRecordReader &reader) {
  nassertr(reader.get_opcode() == FO_color_palette, false);
  DatagramIterator &iterator = reader.get_iterator();

  if (_got_color_palette) {
    nout << "Warning: multiple color palettes found.\n";
  }
  _got_color_palette = true;

  static const int expected_color_entries = 1024;

  iterator.skip_bytes(128);
  _colors.clear();
  for (int i = 0; i < expected_color_entries; i++) {
    if (iterator.get_remaining_size() == 0) {
      // An early end to the palette is acceptable.
      return true;
    }
    FltPackedColor color;
    if (!color.extract_record(reader)) {
      return false;
    }
    _colors.push_back(color);
  }

  // Now pull out the color names.
  while (iterator.get_remaining_size() > 0) {
    int entry_length = iterator.get_be_uint16();
    iterator.skip_bytes(2);
    if (iterator.get_remaining_size() > 0) {
      int color_index = iterator.get_be_int16();
      iterator.skip_bytes(2);

      int name_length = entry_length - 8;
      nassertr(color_index >= 0 && color_index < (int)_colors.size(), false);
      _color_names[color_index] = iterator.get_fixed_string(name_length);
    }
  }

  check_remaining_size(iterator, "color palette");
  return true;
}

/**
 * Reads a single material ancillary record.
 */
bool FltHeader::
extract_material(FltRecordReader &reader) {
  PT(FltMaterial) material = new FltMaterial(this);
  if (!material->extract_record(reader)) {
    return false;
  }
  add_material(material);

  return true;
}

/**
 * Reads the v14.2 material palette.
 */
bool FltHeader::
extract_14_material_palette(FltRecordReader &reader) {
  nassertr(reader.get_opcode() == FO_14_material_palette, false);
  DatagramIterator &iterator = reader.get_iterator();

  if (_got_14_material_palette) {
    nout << "Warning: multiple material palettes found.\n";
  }
  _got_14_material_palette = true;

  static const int expected_material_entries = 64;

  _materials.clear();
  for (int i = 0; i < expected_material_entries; i++) {
    if (iterator.get_remaining_size() == 0) {
      // An early end to the palette is acceptable.
      return true;
    }
    PT(FltMaterial) material = new FltMaterial(this);
    if (!material->extract_14_record(i, iterator)) {
      return false;
    }
    add_material(material);
  }

  check_remaining_size(iterator, "material palette");
  return true;
}

/**
 * Reads a single texture ancillary record.
 */
bool FltHeader::
extract_texture(FltRecordReader &reader) {
  FltTexture *texture = new FltTexture(this);
  if (!texture->extract_record(reader)) {
    return false;
  }
  add_texture(texture);

  return true;
}

/**
 * Reads the a single texture mapping ancillary record.  This describes a kind
 * of texture mapping in the texture mapping palette.
 */
bool FltHeader::
extract_texture_map(FltRecordReader &reader) {
  // At the moment, we ignore this, since it's not needed for meaningful
  // extraction of data: we can get this information from the UV's for a
  // particular model.  We just add an UnsupportedRecord for it.
  FltUnsupportedRecord *rec = new FltUnsupportedRecord(this);
  if (!rec->extract_record(reader)) {
    return false;
  }
  add_ancillary(rec);

  return true;
}

/**
 * Reads a single light source ancillary record.
 */
bool FltHeader::
extract_light_source(FltRecordReader &reader) {
  FltLightSourceDefinition *light_source = new FltLightSourceDefinition(this);
  if (!light_source->extract_record(reader)) {
    return false;
  }
  add_light_source(light_source);

  return true;
}

/**
 * Reads the eyepoint/trackplane palette.
 */
bool FltHeader::
extract_eyepoint_palette(FltRecordReader &reader) {
  nassertr(reader.get_opcode() == FO_eyepoint_palette, false);
  DatagramIterator &iterator = reader.get_iterator();

  iterator.skip_bytes(4);

  int i;
  int num_eyepoints = get_num_eyepoints();
  for (i = 0; i < num_eyepoints; i++) {
    if (!_eyepoints[i].extract_record(reader)) {
      return false;
    }
  }

  int num_trackplanes = get_num_trackplanes();
  for (i = 0; i < num_trackplanes; i++) {
    if (!_trackplanes[i].extract_record(reader)) {
      return false;
    }
  }

  _got_eyepoint_trackplane_palette = true;

  if (get_flt_version() >= 1420) {
    // I have no idea what bytes are supposed to be here in earlier versions
    // that 14.2, but who really cares?  Don't bother reporting it if there
    // are too many bytes in old versions.
    check_remaining_size(iterator, "eyepoint palette");
  }
  return true;
}

/**
 * Writes out the vertex palette with all of its vertices.
 */
FltError FltHeader::
write_vertex_palette(FltRecordWriter &writer) const {
  FltError result;

  int vertex_palette_length =
    ((FltHeader *)this)->update_vertex_lookups();
  Datagram vertex_palette;
  vertex_palette.add_be_int32(vertex_palette_length);
  result = writer.write_record(FO_vertex_palette, vertex_palette);
  if (result != FE_ok) {
    return result;
  }
  // Now write out each vertex in the palette.
  Vertices::const_iterator vi;
  for (vi = _vertices.begin(); vi != _vertices.end(); ++vi) {
    FltVertex *vertex = (*vi);
    vertex->build_record(writer);
    result = writer.advance();
    if (result != FE_ok) {
      return result;
    }
  }

  return FE_ok;
}


/**
 * Writes out the color palette.
 */
FltError FltHeader::
write_color_palette(FltRecordWriter &writer) const {
  writer.set_opcode(FO_color_palette);
  Datagram &datagram = writer.update_datagram();

  datagram.pad_bytes(128);

  // How many colors should we write?
  int num_colors = 1024;

  Colors::const_iterator ci;
  for (ci = _colors.begin(); num_colors > 0 && ci != _colors.end(); ++ci) {
    if (!(*ci).build_record(writer)) {
      assert(!flt_error_abort);
      return FE_invalid_record;
    }
    num_colors--;
  }

  // Now we might need to pad the record to fill up the required number of
  // colors.
  if (num_colors > 0) {
    FltPackedColor empty;
    while (num_colors > 0) {
      if (!empty.build_record(writer)) {
        assert(!flt_error_abort);
        return FE_invalid_record;
      }
      num_colors--;
    }
  }

  // Now append all the names at the end.
  ColorNames::const_iterator ni;
  for (ni = _color_names.begin(); ni != _color_names.end(); ++ni) {
    std::string name = (*ni).second.substr(0, 80);
    int entry_length = name.length() + 8;
    datagram.add_be_uint16(entry_length);
    datagram.pad_bytes(2);
    datagram.add_be_uint16((*ni).first);
    datagram.pad_bytes(2);
    datagram.add_fixed_string(name, name.length());
  }

  return writer.advance();
}

/**
 * Writes out the material palette.
 */
FltError FltHeader::
write_material_palette(FltRecordWriter &writer) const {
  FltError result;

  if (get_flt_version() >= 1520) {
    // Write a version 15 material palette.
    Materials::const_iterator mi;
    for (mi = _materials.begin(); mi != _materials.end(); ++mi) {
      FltMaterial *material = (*mi).second;
      material->build_record(writer);

      result = writer.advance();
      if (result != FE_ok) {
        return result;
      }
    }

  } else {
    // Write a version 14 material palette.
    if (_materials.empty()) {
      // No palette is OK.
      return FE_ok;
    }
    writer.set_opcode(FO_14_material_palette);
    Datagram &datagram = writer.update_datagram();

    PT(FltMaterial) dummy_material = new FltMaterial(_header);

    Materials::const_iterator mi = _materials.lower_bound(0);
    int index;
    static const int expected_material_entries = 64;
    for (index = 0; index < expected_material_entries; index++) {
      if (mi == _materials.end() || index < (*mi).first) {
        dummy_material->build_14_record(datagram);
      } else {
        nassertr(index == (*mi).first, FE_internal);
        FltMaterial *material = (*mi).second;
        material->build_14_record(datagram);
        ++mi;
      }
    }

    result = writer.advance();
    if (result != FE_ok) {
      return result;
    }
  }

  return FE_ok;
}

/**
 * Writes out the texture palette.
 */
FltError FltHeader::
write_texture_palette(FltRecordWriter &writer) const {
  FltError result;

  Textures::const_iterator ti;
  for (ti = _textures.begin(); ti != _textures.end(); ++ti) {
    FltTexture *texture = (*ti).second;
    texture->build_record(writer);
    result = writer.advance();
    if (result != FE_ok) {
      return result;
    }
  }

  return FE_ok;
}

/**
 * Writes out the light source palette.
 */
FltError FltHeader::
write_light_source_palette(FltRecordWriter &writer) const {
  FltError result;

  LightSources::const_iterator li;
  for (li = _light_sources.begin(); li != _light_sources.end(); ++li) {
    FltLightSourceDefinition *light_source = (*li).second;
    light_source->build_record(writer);
    result = writer.advance();
    if (result != FE_ok) {
      return result;
    }
  }

  return FE_ok;
}

/**
 * Writes out the eyepoint/trackplane palette, if we have one.
 */
FltError FltHeader::
write_eyepoint_palette(FltRecordWriter &writer) const {
  if (!_got_eyepoint_trackplane_palette) {
    return FE_ok;
  }

  writer.set_opcode(FO_eyepoint_palette);
  Datagram &datagram = writer.update_datagram();
  datagram.pad_bytes(4);

  int i;
  int num_eyepoints = get_num_eyepoints();
  for (i = 0; i < num_eyepoints; i++) {
    if (!_eyepoints[i].build_record(writer)) {
      assert(!flt_error_abort);
      return FE_bad_data;
    }
  }

  int num_trackplanes = get_num_trackplanes();
  for (i = 0; i < num_trackplanes; i++) {
    if (!_trackplanes[i].build_record(writer)) {
      assert(!flt_error_abort);
      return FE_bad_data;
    }
  }

  return writer.advance();
}
