/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltTexture.cxx
 * @author drose
 * @date 2000-08-25
 */

#include "fltTexture.h"
#include "fltRecordReader.h"
#include "fltRecordWriter.h"
#include "fltHeader.h"
#include "pathReplace.h"
#include "config_putil.h"

TypeHandle FltTexture::_type_handle;

/**
 *
 */
FltTexture::
FltTexture(FltHeader *header) : FltRecord(header) {
  _pattern_index = -1;
  _x_location = 0;
  _y_location = 0;

  _num_texels_u = 0;
  _num_texels_v = 0;
  _real_world_size_u = 0;
  _real_world_size_v = 0;
  _up_vector_x = 0;
  _up_vector_y = 1;
  _file_format = FF_none;
  _min_filter = MN_point;
  _mag_filter = MG_point;
  _repeat = RT_repeat;
  _repeat_u = RT_repeat;
  _repeat_v = RT_repeat;
  _modify_flag = 0;
  _x_pivot_point = 0;
  _y_pivot_point = 0;
  _env_type = ET_modulate;
  _intensity_is_alpha = false;
  _float_real_world_size_u = 0.0;
  _float_real_world_size_v = 0.0;
  _imported_origin_code = 0;
  _kernel_version = 1520;
  _internal_format = IF_default;
  _external_format = EF_default;
  _use_mipmap_kernel = false;
  memset(_mipmap_kernel, 0, sizeof(_mipmap_kernel));
  _use_lod_scale = false;
  memset(_lod_scale, 0, sizeof(_lod_scale));
  _clamp = 0.0;
  _mag_filter_alpha = MG_point;
  _mag_filter_color = MG_point;
  _lambert_conic_central_meridian = 0.0;
  _lambert_conic_upper_latitude = 0.0;
  _lambert_conic_lower_latitude = 0.0;
  _use_detail = false;
  _detail_j = 0;
  _detail_k = 0;
  _detail_m = 0;
  _detail_n = 0;
  _detail_scramble = 0;
  _use_tile = false;
  _tile_lower_left_u = 0.0;
  _tile_lower_left_v = 0.0;
  _tile_upper_right_u = 0.0;
  _tile_upper_right_v = 0.0;
  _projection = PT_flat_earth;
  _earth_model = EM_wgs84;
  _utm_zone = 0;
  _image_origin = IO_lower_left;
  _geospecific_points_units = PU_degrees;
  _geospecific_hemisphere = H_southern;
  _file_version = 1501;
}

/**
 * Walks the hierarchy at this record and below and copies the
 * _converted_filename record into the _orig_filename record, so the flt file
 * will be written out with the converted filename instead of what was
 * originally read in.
 */
void FltTexture::
apply_converted_filenames() {
  _orig_filename = _converted_filename.to_os_generic();
  FltRecord::apply_converted_filenames();
}

/**
 * Returns the name of the texture image file.
 */
Filename FltTexture::
get_texture_filename() const {
  return _converted_filename;
}

/**
 * Changes the name of the texture image file.
 */
void FltTexture::
set_texture_filename(const Filename &filename) {
  _converted_filename = filename;
  _orig_filename = _converted_filename.to_os_generic();
}

/**
 * Returns the name of the texture's associated .attr file.  This contains
 * some additional MultiGen information about the texture parameters.  This
 * is, of course, just the name of the texture with .attr appended.
 *
 * Normally, it won't be necessary to access this file directly; you can call
 * read_attr_data() or write_attr_data() to get at the data stored in this
 * file.  (And read_attr_data() is called automatically when the Flt file is
 * read in.)
 */
Filename FltTexture::
get_attr_filename() const {
  std::string texture_filename = get_texture_filename();
  return Filename::binary_filename(texture_filename + ".attr");
}

/**
 * Opens up the texture's .attr file and reads its data into the extra
 * FltTexture fields.  This is normally performed automatically when the Flt
 * file is read from disk.
 */
FltError FltTexture::
read_attr_data() {
  Filename attr_filename = get_attr_filename();

  std::ifstream attr;
  if (!attr_filename.open_read(attr)) {
    return FE_could_not_open;
  }

  // Determine the file's size so we can read it all into one big datagram.
  attr.seekg(0, std::ios::end);
  if (attr.fail()) {
    return FE_read_error;
  }
  std::streampos length = attr.tellg();

  char *buffer = new char[length];

  attr.seekg(0, std::ios::beg);
  attr.read(buffer, length);
  if (attr.fail()) {
    return FE_read_error;
  }

  Datagram datagram(buffer, length);
  delete[] buffer;

  return unpack_attr(datagram);
}

/**
 * Writes the texture's .attr file.  This may or may not be performed
 * automatically, according to the setting of
 * FltHeader::set_auto_attr_update().
 */
FltError FltTexture::
write_attr_data() const {
  return write_attr_data(get_attr_filename());
}

/**
 * Writes the texture's .attr file to the named file.
 */
FltError FltTexture::
write_attr_data(Filename attr_filename) const {
  Datagram datagram;
  FltError result = pack_attr(datagram);
  if (result != FE_ok) {
    return result;
  }

  attr_filename.set_binary();
  std::ofstream attr;
  if (!attr_filename.open_write(attr)) {
    return FE_could_not_open;
  }

  attr.write((const char *)datagram.get_data(), datagram.get_length());
  if (attr.fail()) {
    return FE_write_error;
  }
  return FE_ok;
}

/**
 * Fills in the information in this record based on the information given in
 * the indicated datagram, whose opcode has already been read.  Returns true
 * on success, false if the datagram is invalid.
 */
bool FltTexture::
extract_record(FltRecordReader &reader) {
  if (!FltRecord::extract_record(reader)) {
    return false;
  }

  nassertr(reader.get_opcode() == FO_texture, false);
  DatagramIterator &iterator = reader.get_iterator();

  if (_header->get_flt_version() < 1420) {
    _orig_filename = iterator.get_fixed_string(80);
  } else {
    _orig_filename = iterator.get_fixed_string(200);
  }
  _converted_filename = _header->convert_path(Filename::from_os_specific(_orig_filename), get_model_path());
  _pattern_index = iterator.get_be_int32();
  _x_location = iterator.get_be_int32();
  _y_location = iterator.get_be_int32();

  if (read_attr_data() != FE_ok) {
    nout << "Unable to read attribute file " << get_attr_filename() << "\n";
  }

  check_remaining_size(iterator);
  return true;
}

/**
 * Fills up the current record on the FltRecordWriter with data for this
 * record, but does not advance the writer.  Returns true on success, false if
 * there is some error.
 */
bool FltTexture::
build_record(FltRecordWriter &writer) const {
  if (!FltRecord::build_record(writer)) {
    return false;
  }

  writer.set_opcode(FO_texture);
  Datagram &datagram = writer.update_datagram();

  datagram.add_fixed_string(_orig_filename, 200);
  datagram.add_be_int32(_pattern_index);
  datagram.add_be_int32(_x_location);
  datagram.add_be_int32(_y_location);

  if (_header->get_auto_attr_update() == FltHeader::AU_always ||
      (_header->get_auto_attr_update() == FltHeader::AU_if_missing &&
       !get_attr_filename().exists())) {
    if (write_attr_data() != FE_ok) {
      nout << "Unable to write attribute file " << get_attr_filename() << "\n";
    }
  }

  return true;
}

/**
 * Reads the data from the attribute file.
 */
FltError FltTexture::
unpack_attr(const Datagram &datagram) {
  DatagramIterator iterator(datagram);

  _num_texels_u = iterator.get_be_int32();
  _num_texels_v = iterator.get_be_int32();
  _real_world_size_u = iterator.get_be_int32();
  _real_world_size_v = iterator.get_be_int32();
  _up_vector_x = iterator.get_be_int32();
  _up_vector_y = iterator.get_be_int32();
  _file_format = (FileFormat)iterator.get_be_int32();
  _min_filter = (Minification)iterator.get_be_int32();
  _mag_filter = (Magnification)iterator.get_be_int32();
  _repeat = (RepeatType)iterator.get_be_int32();
  _repeat_u = (RepeatType)iterator.get_be_int32();
  _repeat_v = (RepeatType)iterator.get_be_int32();
  _modify_flag = iterator.get_be_int32();
  _x_pivot_point = iterator.get_be_int32();
  _y_pivot_point = iterator.get_be_int32();
  _env_type = (EnvironmentType)iterator.get_be_int32();
  _intensity_is_alpha = (iterator.get_be_int32() != 0);
  iterator.skip_bytes(4 * 8);
  iterator.skip_bytes(4);  // Undocumented padding.
  _float_real_world_size_u = iterator.get_be_float64();
  _float_real_world_size_v = iterator.get_be_float64();
  _imported_origin_code = iterator.get_be_int32();
  _kernel_version = iterator.get_be_int32();
  _internal_format = (InternalFormat)iterator.get_be_int32();
  _external_format = (ExternalFormat)iterator.get_be_int32();
  _use_mipmap_kernel = (iterator.get_be_int32() != 0);
  int i;
  for (i = 0; i < 8; i++) {
    _mipmap_kernel[i] = iterator.get_be_float32();
  }
  _use_lod_scale = (iterator.get_be_int32() != 0);
  for (i = 0; i < 8; i++) {
    _lod_scale[i]._lod = iterator.get_be_float32();
    _lod_scale[i]._scale = iterator.get_be_float32();
  }
  _clamp = iterator.get_be_float32();
  _mag_filter_alpha = (Magnification)iterator.get_be_int32();
  _mag_filter_color = (Magnification)iterator.get_be_int32();
  iterator.skip_bytes(4 + 4 * 8);
  _lambert_conic_central_meridian = iterator.get_be_float64();
  _lambert_conic_upper_latitude = iterator.get_be_float64();
  _lambert_conic_lower_latitude = iterator.get_be_float64();
  iterator.skip_bytes(8 + 4 * 5);
  _use_detail = (iterator.get_be_int32() != 0);
  _detail_j = iterator.get_be_int32();
  _detail_k = iterator.get_be_int32();
  _detail_m = iterator.get_be_int32();
  _detail_n = iterator.get_be_int32();
  _detail_scramble = iterator.get_be_int32();
  _use_tile = (iterator.get_be_int32() != 0);
  _tile_lower_left_u = iterator.get_be_float32();
  _tile_lower_left_v = iterator.get_be_float32();
  _tile_upper_right_u = iterator.get_be_float32();
  _tile_upper_right_v = iterator.get_be_float32();
  _projection = (ProjectionType)iterator.get_be_int32();
  _earth_model = (EarthModel)iterator.get_be_int32();
  iterator.skip_bytes(4);
  _utm_zone = iterator.get_be_int32();
  _image_origin = (ImageOrigin)iterator.get_be_int32();
  _geospecific_points_units = (PointsUnits)iterator.get_be_int32();
  _geospecific_hemisphere = (Hemisphere)iterator.get_be_int32();
  iterator.skip_bytes(4 + 4 + 149 * 4);
  iterator.skip_bytes(8);  // Undocumented padding.
  _comment = iterator.get_fixed_string(512);

  if (iterator.get_remaining_size() != 0) {
    iterator.skip_bytes(13 * 4);
    iterator.skip_bytes(4);  // Undocumented padding.
    _file_version = iterator.get_be_int32();

    // Now read the geospecific control points.
    _geospecific_control_points.clear();
    int num_points = iterator.get_be_int32();
    if (num_points > 0) {
      iterator.skip_bytes(4);

      while (num_points > 0) {
        GeospecificControlPoint gcp;
        gcp._uv[0] = iterator.get_be_float64();
        gcp._uv[1] = iterator.get_be_float64();
        gcp._real_earth[0] = iterator.get_be_float64();
        gcp._real_earth[1] = iterator.get_be_float64();
      }
    }

    if (iterator.get_remaining_size() != 0) {
      int num_defs = iterator.get_be_int32();
      while (num_defs > 0) {
        SubtextureDef def;
        def._name = iterator.get_fixed_string(32);
        def._left = iterator.get_be_int32();
        def._bottom = iterator.get_be_int32();
        def._right = iterator.get_be_int32();
        def._top = iterator.get_be_int32();
      }
    }
  }

  check_remaining_size(iterator);
  return FE_ok;
}

/**
 * Packs the attribute data into a big datagram.
 */
FltError FltTexture::
pack_attr(Datagram &datagram) const {
  datagram.add_be_int32(_num_texels_u);
  datagram.add_be_int32(_num_texels_v);
  datagram.add_be_int32(_real_world_size_u);
  datagram.add_be_int32(_real_world_size_v);
  datagram.add_be_int32(_up_vector_x);
  datagram.add_be_int32(_up_vector_y);
  datagram.add_be_int32(_file_format);
  datagram.add_be_int32(_min_filter);
  datagram.add_be_int32(_mag_filter);
  datagram.add_be_int32(_repeat);
  datagram.add_be_int32(_repeat_u);
  datagram.add_be_int32(_repeat_v);
  datagram.add_be_int32(_modify_flag);
  datagram.add_be_int32(_x_pivot_point);
  datagram.add_be_int32(_y_pivot_point);
  datagram.add_be_int32(_env_type);
  datagram.add_be_int32(_intensity_is_alpha);
  datagram.pad_bytes(4 * 8);
  datagram.pad_bytes(4);  // Undocumented padding.
  datagram.add_be_float64(_float_real_world_size_u);
  datagram.add_be_float64(_float_real_world_size_v);
  datagram.add_be_int32(_imported_origin_code);
  datagram.add_be_int32(_kernel_version);
  datagram.add_be_int32(_internal_format);
  datagram.add_be_int32(_external_format);
  datagram.add_be_int32(_use_mipmap_kernel);
  int i;
  for (i = 0; i < 8; i++) {
    datagram.add_be_float32(_mipmap_kernel[i]);
  }
  datagram.add_be_int32(_use_lod_scale);
  for (i = 0; i < 8; i++) {
    datagram.add_be_float32(_lod_scale[i]._lod);
    datagram.add_be_float32(_lod_scale[i]._scale);
  }
  datagram.add_be_float32(_clamp);
  datagram.add_be_int32(_mag_filter_alpha);
  datagram.add_be_int32(_mag_filter_color);
  datagram.pad_bytes(4 + 4 * 8);
  datagram.add_be_float64(_lambert_conic_central_meridian);
  datagram.add_be_float64(_lambert_conic_upper_latitude);
  datagram.add_be_float64(_lambert_conic_lower_latitude);
  datagram.pad_bytes(8 + 4 * 5);
  datagram.add_be_int32(_use_detail);
  datagram.add_be_int32(_detail_j);
  datagram.add_be_int32(_detail_k);
  datagram.add_be_int32(_detail_m);
  datagram.add_be_int32(_detail_n);
  datagram.add_be_int32(_detail_scramble);
  datagram.add_be_int32(_use_tile);
  datagram.add_be_float32(_tile_lower_left_u);
  datagram.add_be_float32(_tile_lower_left_v);
  datagram.add_be_float32(_tile_upper_right_u);
  datagram.add_be_float32(_tile_upper_right_v);
  datagram.add_be_int32(_projection);
  datagram.add_be_int32(_earth_model);
  datagram.pad_bytes(4);
  datagram.add_be_int32(_utm_zone);
  datagram.add_be_int32(_image_origin);
  datagram.add_be_int32(_geospecific_points_units);
  datagram.add_be_int32(_geospecific_hemisphere);
  datagram.pad_bytes(4 + 4 + 149 * 4);
  datagram.pad_bytes(8);  // Undocumented padding.
  datagram.add_fixed_string(_comment, 512);
  datagram.pad_bytes(13 * 4);
  datagram.pad_bytes(4);  // Undocumented padding.
  datagram.add_be_int32(_file_version);

  // Now write the geospecific control points.
  datagram.add_be_int32(_geospecific_control_points.size());
  if (!_geospecific_control_points.empty()) {
    datagram.pad_bytes(4);
    GeospecificControlPoints::const_iterator pi;
    for (pi = _geospecific_control_points.begin();
         pi != _geospecific_control_points.end();
         ++pi) {
      const GeospecificControlPoint &gcp = (*pi);
      datagram.add_be_float64(gcp._uv[0]);
      datagram.add_be_float64(gcp._uv[1]);
      datagram.add_be_float64(gcp._real_earth[0]);
      datagram.add_be_float64(gcp._real_earth[1]);
    }
  }

  // Also write out the subtexture definitions.
  datagram.add_be_int32(_subtexture_defs.size());
  SubtextureDefs::const_iterator di;
  for (di = _subtexture_defs.begin();
       di != _subtexture_defs.end();
       ++di) {
    const SubtextureDef &def = (*di);
    datagram.add_fixed_string(def._name, 31);
    datagram.add_int8(0);
    datagram.add_be_int32(def._left);
    datagram.add_be_int32(def._bottom);
    datagram.add_be_int32(def._right);
    datagram.add_be_int32(def._top);
  }

  return FE_ok;
}
