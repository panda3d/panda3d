// Filename: animChannelMatrixXfmTable.cxx
// Created by:  drose (20Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////


#include "animChannelMatrixXfmTable.h"
#include "animBundle.h"
#include "config_chan.h"

#include <compose_matrix.h>
#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>
#include <fftCompressor.h>

TypeHandle AnimChannelMatrixXfmTable::_type_handle;

const char
AnimChannelMatrixXfmTable::_table_ids[AnimChannelMatrixXfmTable::num_tables] =
{ 'i', 'j', 'k', 'h', 'p', 'r', 'x', 'y', 'z' };

const float
AnimChannelMatrixXfmTable::_default_values[AnimChannelMatrixXfmTable::num_tables] =
{ 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
AnimChannelMatrixXfmTable::
AnimChannelMatrixXfmTable(AnimGroup *parent, const string &name)
  : AnimChannelMatrix(parent, name) {
}

/////////////////////////////////////////////////////////////
AnimChannelMatrixXfmTable::
AnimChannelMatrixXfmTable(void)
{
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::has_changed
//       Access: Public, Virtual
//  Description: Returns true if the value has changed since the last
//               call to has_changed().  last_frame is the frame
//               number of the last call; this_frame is the current
//               frame number.
////////////////////////////////////////////////////////////////////
bool AnimChannelMatrixXfmTable::
has_changed(int last_frame, int this_frame) {
  for (int i = 0; i < num_tables; i++) {
    if (_tables[i].size() > 1) {
      if (_tables[i][last_frame % _tables[i].size()] !=
          _tables[i][this_frame % _tables[i].size()]) {
        return true;
      }
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::get_value
//       Access: Public, Virtual
//  Description: Gets the value of the channel at the indicated frame.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixXfmTable::
get_value(int frame, LMatrix4f &mat) {
  float components[num_tables];

  for (int i = 0; i < num_tables; i++) {
    if (_tables[i].empty()) {
      components[i] = get_default_value(i);
    } else {
      components[i] = _tables[i][frame % _tables[i].size()];
    }
  }

  compose_matrix(mat, components);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::get_value_no_scale
//       Access: Public, Virtual
//  Description: Gets the value of the channel at the indicated frame,
//               without any scale information.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixXfmTable::
get_value_no_scale(int frame, LMatrix4f &mat) {
  float components[num_tables];
  components[0] = 1.0f;
  components[1] = 1.0f;
  components[2] = 1.0f;

  for (int i = 3; i < num_tables; i++) {
    if (_tables[i].empty()) {
      components[i] = get_default_value(i);
    } else {
      components[i] = _tables[i][frame % _tables[i].size()];
    }
  }

  compose_matrix(mat, components);
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::get_scale
//       Access: Public, Virtual
//  Description: Gets the scale value at the indicated frame.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixXfmTable::
get_scale(int frame, float scale[3]) {
  for (int i = 0; i < 3; i++) {
    if (_tables[i].empty()) {
      scale[i] = get_default_value(i);
    } else {
      scale[i] = _tables[i][frame % _tables[i].size()];
    }
  }
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::clear_all_tables
//       Access: Public
//  Description: Removes all the tables from the channel, and resets
//               it to its initial state.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixXfmTable::
clear_all_tables() {
  for (int i = 0; i < num_tables; i++) {
    _tables[i] = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::set_table
//       Access: Public
//  Description: Assigns the indicated table.  table_id is one of 'i',
//               'j', 'k', for scale, 'h', 'p', 'r', for rotation, and
//               'x', 'y', 'z', for translation.  The new table must
//               have either zero, one, or get_num_frames() frames.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixXfmTable::
set_table(char table_id, const CPTA_float &table) {
  int num_frames = _root->get_num_frames();

  if (table.size() > 1 && (int)table.size() < num_frames) {
    // The new table has an invalid number of frames--it doesn't match
    // the bundle's requirement.
    return;
  }

  int i = get_table_index(table_id);
  if (i < 0) {
    return;
  }

  _tables[i] = table;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::write
//       Access: Public, Virtual
//  Description: Writes a brief description of the table and all of
//               its descendants.
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixXfmTable::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type() << " " << get_name() << " ";

  // Write a list of all the sub-tables that have data.
  bool found_any = false;
  for (int i = 0; i < num_tables; i++) {
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


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::get_table_index
//       Access: Protected, Static
//  Description: Returns the table index number, a value between 0 and
//               num_tables, that corresponds to the indicate table
//               id.  Returns -1 if the table id is invalid.
////////////////////////////////////////////////////////////////////
int AnimChannelMatrixXfmTable::
get_table_index(char table_id) {
  for (int i = 0; i < num_tables; i++) {
    if (table_id == get_table_id(i)) {
      return i;
    }
  }

  return -1;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixXfmTable::
write_datagram(BamWriter *manager, Datagram &me)
{
  AnimChannelMatrix::write_datagram(manager, me);

  if (compress_channels && !FFTCompressor::is_compression_available()) {
    chan_cat.error()
      << "Compression is not available; writing uncompressed channels.\n";
    compress_channels = false;
  }

  me.add_bool(compress_channels);
  if (!compress_channels) {
    // Write out everything uncompressed, as a stream of floats.
    for(int i = 0; i < num_tables; i++) {
      me.add_uint16(_tables[i].size());
      for(int j = 0; j < (int)_tables[i].size(); j++) {
        me.add_float32(_tables[i][j]);
      }
    }

  } else {
    // Write out everything using lossy compression.

    FFTCompressor compressor;
    compressor.set_quality(compress_chan_quality);
    compressor.write_header(me);

    // First, write out the scales.
    int i;
    for(i = 0; i < 3; i++) {
      compressor.write_reals(me, _tables[i], _tables[i].size());
    }

    // Now, write out the joint angles.  For these we need to build up
    // a HPR array.
    vector_LVecBase3f hprs;
    int hprs_length =
      max(max(_tables[3].size(), _tables[4].size()), _tables[5].size());
    hprs.reserve(hprs_length);
    for (i = 0; i < hprs_length; i++) {
      float h = _tables[3].empty() ? 0.0f : _tables[3][i % _tables[3].size()];
      float p = _tables[4].empty() ? 0.0f : _tables[4][i % _tables[4].size()];
      float r = _tables[5].empty() ? 0.0f : _tables[5][i % _tables[5].size()];
      hprs.push_back(LVecBase3f(h, p, r));
    }
    compressor.write_hprs(me, &hprs[0], hprs_length);

    // And now the translations.
    for(i = 6; i < 9; i++) {
      compressor.write_reals(me, _tables[i], _tables[i].size());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixXfmTable::
fillin(DatagramIterator& scan, BamReader* manager)
{
  AnimChannelMatrix::fillin(scan, manager);

  bool wrote_compressed = scan.get_bool();

  if (!wrote_compressed) {
    // Regular floats.
    for(int i = 0; i < num_tables; i++) {
      int size = scan.get_uint16();
      PTA_float ind_table;
      for(int j = 0; j < size; j++) {
        ind_table.push_back(scan.get_float32());
      }
      _tables[i] = ind_table;
    }

  } else {
    // Compressed channels.
    if (manager->get_file_minor_ver() < 1) {
      chan_cat.error()
        << "Cannot read old-style quantized channels.\n";
      clear_all_tables();
      return;
    }

    if (!read_compressed_channels) {
      chan_cat.info()
        << "Not reading compressed animation channels.\n";
      clear_all_tables();
      return;
    }

    FFTCompressor compressor;
    compressor.read_header(scan);

    int i;
    // First, read in the scales.
    for(i = 0; i < 3; i++) {
      PTA_float ind_table=PTA_float::empty_array(0);
      compressor.read_reals(scan, ind_table.v());
      _tables[i] = ind_table;
    }

    // Read in the HPR array and store it back in the joint angles.
    vector_LVecBase3f hprs;
    compressor.read_hprs(scan, hprs);
    PTA_float h_table=PTA_float::empty_array(hprs.size());
    PTA_float p_table=PTA_float::empty_array(hprs.size());
    PTA_float r_table=PTA_float::empty_array(hprs.size());

    for (i = 0; i < (int)hprs.size(); i++) {
      h_table[i] = hprs[i][0];
      p_table[i] = hprs[i][1];
      r_table[i] = hprs[i][2];
    }
    _tables[3] = h_table;
    _tables[4] = p_table;
    _tables[5] = r_table;

    // Now read in the translations.
    for (i = 6; i < 9; i++) {
      PTA_float ind_table=PTA_float::empty_array(0);
      compressor.read_reals(scan, ind_table.v());
      _tables[i] = ind_table;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::make_AnimChannelMatrixXfmTable
//       Access: Protected
//  Description: Factory method to generate a AnimChannelMatrixXfmTable object
////////////////////////////////////////////////////////////////////
TypedWritable* AnimChannelMatrixXfmTable::
make_AnimChannelMatrixXfmTable(const FactoryParams &params)
{
  AnimChannelMatrixXfmTable *me = new AnimChannelMatrixXfmTable;
  DatagramIterator scan;
  BamReader *manager;

  parse_params(params, scan, manager);
  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a AnimChannelMatrixXfmTable object
////////////////////////////////////////////////////////////////////
void AnimChannelMatrixXfmTable::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimChannelMatrixXfmTable);
}


