// Filename: animChannelScalarTable.cxx
// Created by:  drose (22Feb99)
// 
////////////////////////////////////////////////////////////////////

#include "animChannelScalarTable.h"
#include "animBundle.h"
#include "config_chan.h"

#include <indent.h>
#include <datagram.h>
#include <datagramIterator.h>
#include <bamReader.h>
#include <bamWriter.h>

TypeHandle AnimChannelScalarTable::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
AnimChannelScalarTable::
AnimChannelScalarTable(AnimGroup *parent, const string &name) 
  : AnimChannelScalar(parent, name) {
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
AnimChannelScalarTable::
AnimChannelScalarTable(void){
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::has_changed
//       Access: Public, Virtual
//  Description: Returns true if the value has changed since the last
//               call to has_changed().  last_frame is the frame
//               number of the last call; this_frame is the current
//               frame number.
////////////////////////////////////////////////////////////////////
bool AnimChannelScalarTable::
has_changed(int last_frame, int this_frame) {
  if (_table.size() > 1) {
    if (_table[last_frame % _table.size()] !=
	_table[this_frame % _table.size()]) {
      return true;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::get_value
//       Access: Public, Virtual
//  Description: Gets the value of the channel at the indicated frame.
////////////////////////////////////////////////////////////////////
void AnimChannelScalarTable::
get_value(int frame, float &value) {
  if (_table.empty()) {
    value = 0.0;
  } else {
    value = _table[frame % _table.size()];
  }
}


////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::set_table
//       Access: Public
//  Description: Assigns the data table.
////////////////////////////////////////////////////////////////////
void AnimChannelScalarTable::
set_table(const CPTA_float &table) {
  int num_frames = _root->get_num_frames();

  if (table.size() > 1 && (int)table.size() < num_frames) {
    // The new table has an invalid number of frames--it doesn't match
    // the bundle's requirement.
    return;
  }

  _table = table;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::write
//       Access: Public, Virtual
//  Description: Writes a brief description of the table and all of
//               its descendants.
////////////////////////////////////////////////////////////////////
void AnimChannelScalarTable::
write(ostream &out, int indent_level) const {
  indent(out, indent_level)
    << get_type() << " " << get_name() << " " << _table.size();

  if (!_children.empty()) {
    out << " {\n";
    write_descendants(out, indent_level + 2);
    indent(out, indent_level) << "}";
  }

  out << "\n";
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::write_datagram
//       Access: Public
//  Description: Function to write the important information in
//               the particular object to a Datagram
////////////////////////////////////////////////////////////////////
void AnimChannelScalarTable::
write_datagram(BamWriter *manager, Datagram &me)
{
  AnimChannelScalar::write_datagram(manager, me);

  me.add_bool(quantize_bam_channels);
  if (!quantize_bam_channels) {
    // Write out everything the old way, as floats.
    me.add_uint16(_table.size());
    for(int i = 0; i < (int)_table.size(); i++) {
      me.add_float32(_table[i]);
    }

  } else {
    // Write out everything the compact way, as quantized integers.

    // We quantize morphs within the range -100 .. 100.
    me.add_uint16(_table.size());
    for(int i = 0; i < (int)_table.size(); i++) {
      me.add_int16((int)max(min(_table[i] * 32767.0 / 100.0, 32767.0), -32767.0));
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::fillin
//       Access: Protected
//  Description: Function that reads out of the datagram (or asks
//               manager to read) all of the data that is needed to
//               re-create this object and stores it in the appropiate
//               place
////////////////////////////////////////////////////////////////////
void AnimChannelScalarTable::
fillin(DatagramIterator& scan, BamReader* manager)
{
  AnimChannelScalar::fillin(scan, manager);

  bool wrote_quantized = scan.get_bool();

  if (!wrote_quantized) {
    // Regular floats.
    int size = scan.get_uint16();
    PTA_float temp_table;
    for(int i = 0; i < size; i++) {
      temp_table.push_back(scan.get_float32());
    }
    _table = temp_table;

  } else {
    // Quantized int16's and expand to floats.
    int size = scan.get_uint16();
    PTA_float temp_table;
    for(int i = 0; i < size; i++) {
      temp_table.push_back((double)scan.get_int16() * 100.0 / 32767.0);
    }
    _table = temp_table;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::make_AnimChannelScalarTable
//       Access: Protected
//  Description: Factory method to generate a AnimChannelScalarTable object
////////////////////////////////////////////////////////////////////
TypedWriteable* AnimChannelScalarTable::
make_AnimChannelScalarTable(const FactoryParams &params)
{
  AnimChannelScalarTable *me = new AnimChannelScalarTable;
  BamReader *manager;
  Datagram packet;

  parse_params(params, manager, packet);
  DatagramIterator scan(packet);

  me->fillin(scan, manager);
  return me;
}

////////////////////////////////////////////////////////////////////
//     Function: AnimChannelScalarTable::register_with_factory
//       Access: Public, Static
//  Description: Factory method to generate a AnimChannelScalarTable object
////////////////////////////////////////////////////////////////////
void AnimChannelScalarTable::
register_with_read_factory(void)
{
  BamReader::get_factory()->register_factory(get_class_type(), make_AnimChannelScalarTable);
}




