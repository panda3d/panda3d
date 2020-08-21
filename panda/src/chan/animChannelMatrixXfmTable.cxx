/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file animChannelMatrixXfmTable.cxx
 * @author drose
 * @date 1999-02-20
 */

#include "animChannelMatrixXfmTable.h"
#include "animBundle.h"
#include "config_chan.h"

#include "compose_matrix.h"
#include "indent.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "fftCompressor.h"
#include "config_linmath.h"

TypeHandle AnimChannelMatrixXfmTable::_type_handle;

/**
 * Used only for bam loader.
 */
AnimChannelMatrixXfmTable::
AnimChannelMatrixXfmTable() {
  for (int i = 0; i < num_matrix_components; i++) {
    _tables[i] = CPTA_stdfloat(get_class_type());
  }
}

/**
 * Creates a new AnimChannelMatrixXfmTable, just like this one, without
 * copying any children.  The new copy is added to the indicated parent.
 * Intended to be called by make_copy() only.
 */
AnimChannelMatrixXfmTable::
AnimChannelMatrixXfmTable(AnimGroup *parent, const AnimChannelMatrixXfmTable &copy) :
  AnimChannelMatrix(parent, copy)
{
  for (int i = 0; i < num_matrix_components; i++) {
    _tables[i] = copy._tables[i];
  }
}

/**
 *
 */
AnimChannelMatrixXfmTable::
AnimChannelMatrixXfmTable(AnimGroup *parent, const std::string &name)
  : AnimChannelMatrix(parent, name)
{
  for (int i = 0; i < num_matrix_components; i++) {
    _tables[i] = CPTA_stdfloat(get_class_type());
  }
}

/**
 *
 */
AnimChannelMatrixXfmTable::
~AnimChannelMatrixXfmTable() {
}


/**
 * Returns true if the value has changed since the last call to has_changed().
 * last_frame is the frame number of the last call; this_frame is the current
 * frame number.
 */
bool AnimChannelMatrixXfmTable::
has_changed(int last_frame, double last_frac,
            int this_frame, double this_frac) {
  if (last_frame != this_frame) {
    for (int i = 0; i < num_matrix_components; i++) {
      if (_tables[i].size() > 1) {
        if (_tables[i][last_frame % _tables[i].size()] !=
            _tables[i][this_frame % _tables[i].size()]) {
          return true;
        }
      }
    }
  }

  if (last_frac != this_frac) {
    // If we have some fractional changes, also check the next subsequent
    // frame (since we'll be blending with that).
    for (int i = 0; i < num_matrix_components; i++) {
      if (_tables[i].size() > 1) {
        if (_tables[i][last_frame % _tables[i].size()] !=
            _tables[i][(this_frame + 1) % _tables[i].size()]) {
          return true;
        }
      }
    }
  }

  return false;
}

/**
 * Gets the value of the channel at the indicated frame.
 */
void AnimChannelMatrixXfmTable::
get_value(int frame, LMatrix4 &mat) {
  PN_stdfloat components[num_matrix_components];

  for (int i = 0; i < num_matrix_components; i++) {
    if (_tables[i].empty()) {
      components[i] = get_default_value(i);
    } else {
      components[i] = _tables[i][frame % _tables[i].size()];
    }
  }

  compose_matrix(mat, components);
}

/**
 * Gets the value of the channel at the indicated frame, without any scale or
 * shear information.
 */
void AnimChannelMatrixXfmTable::
get_value_no_scale_shear(int frame, LMatrix4 &mat) {
  PN_stdfloat components[num_matrix_components];
  components[0] = 1.0f;
  components[1] = 1.0f;
  components[2] = 1.0f;
  components[3] = 0.0f;
  components[4] = 0.0f;
  components[5] = 0.0f;

  for (int i = 6; i < num_matrix_components; i++) {
    if (_tables[i].empty()) {
      components[i] = get_default_value(i);
    } else {
      components[i] = _tables[i][frame % _tables[i].size()];
    }
  }

  compose_matrix(mat, components);
}

/**
 * Gets the scale value at the indicated frame.
 */
void AnimChannelMatrixXfmTable::
get_scale(int frame, LVecBase3 &scale) {
  for (int i = 0; i < 3; i++) {
    if (_tables[i].empty()) {
      scale[i] = 1.0f;
    } else {
      scale[i] = _tables[i][frame % _tables[i].size()];
    }
  }
}

/**
 * Returns the h, p, and r components associated with the current frame.  As
 * above, this only makes sense for a matrix-type channel.
 */
void AnimChannelMatrixXfmTable::
get_hpr(int frame, LVecBase3 &hpr) {
  for (int i = 0; i < 3; i++) {
    if (_tables[i + 6].empty()) {
      hpr[i] = 0.0f;
    } else {
      hpr[i] = _tables[i + 6][frame % _tables[i + 6].size()];
    }
  }
}

/**
 * Returns the rotation component associated with the current frame, expressed
 * as a quaternion.  As above, this only makes sense for a matrix-type
 * channel.
 */
void AnimChannelMatrixXfmTable::
get_quat(int frame, LQuaternion &quat) {
  LVecBase3 hpr;
  for (int i = 0; i < 3; i++) {
    if (_tables[i + 6].empty()) {
      hpr[i] = 0.0f;
    } else {
      hpr[i] = _tables[i + 6][frame % _tables[i + 6].size()];
    }
  }

  quat.set_hpr(hpr);
}

/**
 * Returns the x, y, and z translation components associated with the current
 * frame.  As above, this only makes sense for a matrix-type channel.
 */
void AnimChannelMatrixXfmTable::
get_pos(int frame, LVecBase3 &pos) {
  for (int i = 0; i < 3; i++) {
    if (_tables[i + 9].empty()) {
      pos[i] = 0.0f;
    } else {
      pos[i] = _tables[i + 9][frame % _tables[i + 9].size()];
    }
  }
}

/**
 * Returns the a, b, and c shear components associated with the current frame.
 * As above, this only makes sense for a matrix-type channel.
 */
void AnimChannelMatrixXfmTable::
get_shear(int frame, LVecBase3 &shear) {
  for (int i = 0; i < 3; i++) {
    if (_tables[i + 3].empty()) {
      shear[i] = 0.0f;
    } else {
      shear[i] = _tables[i + 3][frame % _tables[i + 3].size()];
    }
  }
}

/**
 * Assigns the indicated table.  table_id is one of 'i', 'j', 'k', for scale,
 * 'a', 'b', 'c' for shear, 'h', 'p', 'r', for rotation, and 'x', 'y', 'z',
 * for translation.  The new table must have either zero, one, or
 * get_num_frames() frames.
 */
void AnimChannelMatrixXfmTable::
set_table(char table_id, const CPTA_stdfloat &table) {
  int num_frames = _root->get_num_frames();

  if (table.size() > 1 && (int)table.size() < num_frames) {
    // The new table has an invalid number of frames--it doesn't match the
    // bundle's requirement.
    nassert_raise("mismatched number of frames");
    return;
  }

  int i = get_table_index(table_id);
  if (i < 0) {
    return;
  }

  _tables[i] = table;
}


/**
 * Removes all the tables from the channel, and resets it to its initial
 * state.
 */
void AnimChannelMatrixXfmTable::
clear_all_tables() {
  for (int i = 0; i < num_matrix_components; i++) {
    _tables[i] = CPTA_stdfloat(get_class_type());
  }
}

/**
 * Writes a brief description of the table and all of its descendants.
 */
void AnimChannelMatrixXfmTable::
write(std::ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type() << " " << get_name() << " ";

  // Write a list of all the sub-tables that have data.
  bool found_any = false;
  for (int i = 0; i < num_matrix_components; i++) {
    if (!_tables[i].empty()) {
      out << get_table_id(i) << _tables[i].size();
      found_any = true;
    }
  }

  if (!found_any) {
    out << "(no data)";
  }

  if (!_children.empty()) {
    out << " {\n";
    write_descendants(out, indent_level + 2);
    indent(out, indent_level) << "}";
  }

  out << "\n";
}

/**
 * Returns a copy of this object, and attaches it to the indicated parent
 * (which may be NULL only if this is an AnimBundle).  Intended to be called
 * by copy_subtree() only.
 */
AnimGroup *AnimChannelMatrixXfmTable::
make_copy(AnimGroup *parent) const {
  return new AnimChannelMatrixXfmTable(parent, *this);
}

/**
 * Returns the table index number, a value between 0 and
 * num_matrix_components, that corresponds to the indicated table id.  Returns
 * -1 if the table id is invalid.
 */
int AnimChannelMatrixXfmTable::
get_table_index(char table_id) {
  for (int i = 0; i < num_matrix_components; i++) {
    if (table_id == get_table_id(i)) {
      return i;
    }
  }

  return -1;
}

/**
 * Function to write the important information in the particular object to a
 * Datagram
 */
void AnimChannelMatrixXfmTable::
write_datagram(BamWriter *manager, Datagram &me) {
  AnimChannelMatrix::write_datagram(manager, me);

  if (compress_channels) {
    chan_cat.warning()
      << "FFT compression of animations is deprecated.  For compatibility "
         "with future versions of Panda3D, set compress-channels to false.\n";

    if (!FFTCompressor::is_compression_available()) {
      chan_cat.error()
        << "Compression is not available; writing uncompressed channels.\n";
      compress_channels = false;
    }
  }

  me.add_bool(compress_channels);

  // We now always use the new HPR conventions.
  me.add_bool(true);

  if (!compress_channels) {
    // Write out everything uncompressed, as a stream of floats.
    for (int i = 0; i < num_matrix_components; i++) {
      me.add_uint16(_tables[i].size());
      for(int j = 0; j < (int)_tables[i].size(); j++) {
        me.add_stdfloat(_tables[i][j]);
      }
    }

  } else {
    // Write out everything using lossy compression.
    FFTCompressor compressor;
    compressor.set_quality(compress_chan_quality);
    compressor.set_use_error_threshold(true);
    compressor.write_header(me);

    // First, write out the scales and shears.
    int i;
    for (i = 0; i < 6; i++) {
      compressor.write_reals(me, _tables[i], _tables[i].size());
    }

    // Now, write out the joint angles.  For these we need to build up a HPR
    // array.
    pvector<LVecBase3> hprs;
    int hprs_length = std::max(std::max(_tables[6].size(), _tables[7].size()), _tables[8].size());
    hprs.reserve(hprs_length);
    for (i = 0; i < hprs_length; i++) {
      PN_stdfloat h = _tables[6].empty() ? 0.0f : _tables[6][i % _tables[6].size()];
      PN_stdfloat p = _tables[7].empty() ? 0.0f : _tables[7][i % _tables[7].size()];
      PN_stdfloat r = _tables[8].empty() ? 0.0f : _tables[8][i % _tables[8].size()];
      hprs.push_back(LVecBase3(h, p, r));
    }
    const LVecBase3 *hprs_array = nullptr;
    if (hprs_length != 0) {
      hprs_array = &hprs[0];
    }
    compressor.write_hprs(me, hprs_array, hprs_length);

    // And now the translations.
    for(i = 9; i < num_matrix_components; i++) {
      compressor.write_reals(me, _tables[i], _tables[i].size());
    }
  }
}

/**
 * Function that reads out of the datagram (or asks manager to read) all of
 * the data that is needed to re-create this object and stores it in the
 * appropiate place
 */
void AnimChannelMatrixXfmTable::
fillin(DatagramIterator &scan, BamReader *manager) {
  AnimChannelMatrix::fillin(scan, manager);

  bool wrote_compressed = scan.get_bool();

  // If this is false, the file still uses the old HPR conventions, and we'll
  // have to convert the HPR values to the new convention.
  bool new_hpr = scan.get_bool();

  if (!wrote_compressed) {
    // Regular floats.

    for (int i = 0; i < num_matrix_components; i++) {
      int size = scan.get_uint16();
      PTA_stdfloat ind_table(get_class_type());
      for (int j = 0; j < size; j++) {
        ind_table.push_back(scan.get_stdfloat());
      }
      _tables[i] = ind_table;
    }

    if (!new_hpr) {
      // Convert between the old HPR form and the new HPR form.
      size_t num_hprs = std::max(std::max(_tables[6].size(), _tables[7].size()),
                            _tables[8].size());

      LVecBase3 default_hpr(0.0, 0.0, 0.0);
      if (!_tables[6].empty()) {
        default_hpr[0] = _tables[6][0];
      }
      if (!_tables[7].empty()) {
        default_hpr[1] = _tables[7][0];
      }
      if (!_tables[8].empty()) {
        default_hpr[2] = _tables[8][0];
      }

      PTA_stdfloat h_table = PTA_stdfloat::empty_array(num_hprs, get_class_type());
      PTA_stdfloat p_table = PTA_stdfloat::empty_array(num_hprs, get_class_type());
      PTA_stdfloat r_table = PTA_stdfloat::empty_array(num_hprs, get_class_type());

      for (size_t hi = 0; hi < num_hprs; hi++) {
        PN_stdfloat h = (hi < _tables[6].size() ? _tables[6][hi] : default_hpr[0]);
        PN_stdfloat p = (hi < _tables[7].size() ? _tables[7][hi] : default_hpr[1]);
        PN_stdfloat r = (hi < _tables[8].size() ? _tables[8][hi] : default_hpr[2]);

        LVecBase3 hpr = old_to_new_hpr(LVecBase3(h, p, r));
        h_table[hi] = hpr[0];
        p_table[hi] = hpr[1];
        r_table[hi] = hpr[2];
      }
      _tables[6] = h_table;
      _tables[7] = p_table;
      _tables[8] = r_table;
    }

  } else {
    // Compressed channels.
    if (!read_compressed_channels) {
      chan_cat.info()
        << "Not reading compressed animation channels.\n";
      clear_all_tables();
      return;
    }

    FFTCompressor compressor;
    compressor.read_header(scan, manager->get_file_minor_ver());

    int i;
    // First, read in the scales and shears.
    for (i = 0; i < 6; i++) {
      PTA_stdfloat ind_table = PTA_stdfloat::empty_array(0, get_class_type());
      compressor.read_reals(scan, ind_table.v());
      _tables[i] = ind_table;
    }

    // Read in the HPR array and store it back in the joint angles.
    pvector<LVecBase3> hprs;
    compressor.read_hprs(scan, hprs, new_hpr);
    PTA_stdfloat h_table = PTA_stdfloat::empty_array(hprs.size(), get_class_type());
    PTA_stdfloat p_table = PTA_stdfloat::empty_array(hprs.size(), get_class_type());
    PTA_stdfloat r_table = PTA_stdfloat::empty_array(hprs.size(), get_class_type());

    for (i = 0; i < (int)hprs.size(); i++) {
      if (!new_hpr) {
        // Convert the old HPR form to the new HPR form.
        LVecBase3 hpr = old_to_new_hpr(hprs[i]);
        h_table[i] = hpr[0];
        p_table[i] = hpr[1];
        r_table[i] = hpr[2];

      } else {
        // Store the HPR angle directly.
        h_table[i] = hprs[i][0];
        p_table[i] = hprs[i][1];
        r_table[i] = hprs[i][2];
      }
    }
    _tables[6] = h_table;
    _tables[7] = p_table;
    _tables[8] = r_table;

    // Now read in the translations.
    for (i = 9; i < num_matrix_components; i++) {
      PTA_stdfloat ind_table = PTA_stdfloat::empty_array(0, get_class_type());
      compressor.read_reals(scan, ind_table.v());
      _tables[i] = ind_table;
    }
  }
}

/**
 * Factory method to generate an AnimChannelMatrixXfmTable object.
 */
TypedWritable *AnimChannelMatrixXfmTable::
make_AnimChannelMatrixXfmTable(const FactoryParams &params) {
  AnimChannelMatrixXfmTable *me = new AnimChannelMatrixXfmTable;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

/**
 * Factory method to generate an AnimChannelMatrixXfmTable object.
 */
void AnimChannelMatrixXfmTable::
register_with_read_factory() {
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimChannelMatrixXfmTable);
}
