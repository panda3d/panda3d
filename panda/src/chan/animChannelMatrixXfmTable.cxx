// Filename: animChannelMatrixXfmTable.cxx
// Created by:  drose (20Feb99)
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

TypeHandle AnimChannelMatrixXfmTable::_type_handle;

const char 
AnimChannelMatrixXfmTable::_table_ids[AnimChannelMatrixXfmTable::num_tables] =
{ 'i', 'j', 'k', 'h', 'p', 'r', 'x', 'y', 'z' };

const float 
AnimChannelMatrixXfmTable::_default_values[AnimChannelMatrixXfmTable::num_tables] =
{ 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };

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
  components[0] = 1.0;
  components[1] = 1.0;
  components[2] = 1.0;

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

  me.add_uint8(quantize_bam_channels);
  if (!quantize_bam_channels) {
    // Write out everything the old way, as floats.
    for(int i = 0; i < num_tables; i++) {
      me.add_uint16(_tables[i].size());
      for(int j = 0; j < (int)_tables[i].size(); j++) {
	me.add_float64(_tables[i][j]);
      }
    }

  } else {
    // Write out everything the compact way, as quantized integers.

    // First, write out the scales.  These will be in the range 1/256 .. 255.
    int i;
    for(i = 0; i < 3; i++) {
      me.add_uint16(_tables[i].size());
      for(int j = 0; j < (int)_tables[i].size(); j++) {
	me.add_uint16((int)max(min(_tables[i][j]*256.0, 65535.0), 0.0));
      }
    }

    // Now, write out the joint angles.  These are in the range 0 .. 360.
    for(i = 3; i < 6; i++) {
      me.add_uint16(_tables[i].size());
      for(int j = 0; j < (int)_tables[i].size(); j++) {
	me.add_uint16((unsigned int)(_tables[i][j] * 65536.0 / 360.0));
      }
    }

    // And now write out the translations.  These are in the range
    // -1000 .. 1000.
    for(i = 6; i < 9; i++) {
      me.add_uint16(_tables[i].size());
      for(int j = 0; j < (int)_tables[i].size(); j++) {
	me.add_int16((int)max(min(_tables[i][j] * 32767.0 / 1000.0, 32767.0), -32767.0));
      }
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

  bool wrote_quantized = false;
  if (manager->get_file_minor_ver() >= 1) {
    // Version 1 and later: we might have quantized channels.
    wrote_quantized = (bool)scan.get_uint8();
  }

  if (!wrote_quantized) {
    // Regular floats.
    for(int i = 0; i < num_tables; i++) {
      int size = scan.get_uint16();
      PTA_float ind_table;
      for(int j = 0; j < size; j++) {
	ind_table.push_back(scan.get_float64());
      }
      _tables[i] = ind_table;
    }

  } else {
    // Quantized int16's and expand to floats.
    int i;

    // First, read in the scales.
    for(i = 0; i < 3; i++) {
      int size = scan.get_uint16();
      PTA_float ind_table;
      for(int j = 0; j < size; j++) {
	ind_table.push_back((double)scan.get_uint16() / 256.0);
      }
      _tables[i] = ind_table;
    }

    // Then, read in the joint angles.
    for(i = 3; i < 6; i++) {
      int size = scan.get_uint16();
      PTA_float ind_table;
      for(int j = 0; j < size; j++) {
	ind_table.push_back((double)scan.get_uint16() * 360.0 / 65536.0);
      }
      _tables[i] = ind_table;
    }

    // Now read in the translations.
    for(i = 6; i < 9; i++) {
      int size = scan.get_uint16();
      PTA_float ind_table;
      for(int j = 0; j < size; j++) {
	ind_table.push_back((double)scan.get_int16() * 1000.0 / 32767.0);
      }
      _tables[i] = ind_table;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelMatrixXfmTable::make_AnimChannelMatrixXfmTable
//       Access: Protected
//  Description: Factory method to generate a AnimChannelMatrixXfmTable object
////////////////////////////////////////////////////////////////////
TypedWriteable* AnimChannelMatrixXfmTable::
make_AnimChannelMatrixXfmTable(const FactoryParams &params)
{
  AnimChannelMatrixXfmTable *me = new AnimChannelMatrixXfmTable;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

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


