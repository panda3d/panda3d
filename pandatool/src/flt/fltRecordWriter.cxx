// Filename: fltRecordWriter.cxx
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "fltRecordWriter.h"
#include "fltInstanceDefinition.h"
#include "fltHeader.h"
#include "config_flt.h"

#include <datagram.h>

#include <assert.h>

////////////////////////////////////////////////////////////////////
//     Function: FltRecordWriter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltRecordWriter::
FltRecordWriter(ostream &out) :
  _out(out)
{
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordWriter::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
FltRecordWriter::
~FltRecordWriter() {
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordWriter::set_opcode
//       Access: Public
//  Description: Sets the opcode associated with the current record.
////////////////////////////////////////////////////////////////////
void FltRecordWriter::
set_opcode(FltOpcode opcode) {
  _opcode = opcode;
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordWriter::set_datagram
//       Access: Public
//  Description: Sets the datagram that will be written when advance()
//               is called.
////////////////////////////////////////////////////////////////////
void FltRecordWriter::
set_datagram(const Datagram &datagram) {
  _datagram = datagram;
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordWriter::update_datagram
//       Access: Public
//  Description: Returns a modifiable reference to the datagram
//               associated with the current record.  This datagram
//               should then be stuffed with data corresponding to the
//               data in the record, in preparation for calling
//               advance() to write the data.
////////////////////////////////////////////////////////////////////
Datagram &FltRecordWriter::
update_datagram() {
  return _datagram;
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordWriter::advance
//       Access: Public
//  Description: Writes the current record to the flt file, and resets
//               the current record to receive new data.  Returns
//               FE_ok on success, or something else on error.
////////////////////////////////////////////////////////////////////
FltError FltRecordWriter::
advance() {
  if (flt_cat.is_debug()) {
    flt_cat.debug()
      << "Writing " << _opcode << " of length "
      << _datagram.get_length() << "\n";
  }

  // Build a mini-datagram to write the header.
  static const int header_size = 4;

  Datagram dg;
  dg.add_be_int16(_opcode);
  dg.add_be_int16(_datagram.get_length() + header_size);

  nassertr((int)dg.get_length() == header_size, FE_internal);

  _out.write(dg.get_message().data(), dg.get_length());
  if (_out.fail()) {
    assert(!flt_error_abort);
    return FE_write_error;
  }

  // Now write the rest of the record.
  _out.write(_datagram.get_message().data(), _datagram.get_length());
  if (_out.fail()) {
    assert(!flt_error_abort);
    return FE_write_error;
  }

  _datagram.clear();
  _opcode = FO_none;

  return FE_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordWriter::write_record
//       Access: Public
//  Description: A convenience function to quickly write a simple
//               record that consists of an opcode and possibly a
//               datagram.
////////////////////////////////////////////////////////////////////
FltError FltRecordWriter::
write_record(FltOpcode opcode, const Datagram &datagram) {
  _opcode = opcode;
  _datagram = datagram;
  return advance();
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordWriter::write_instance_def
//       Access: Public
//  Description: Ensures that the given instance definition has
//               already been written to the file.  If it has not,
//               writes it now.
////////////////////////////////////////////////////////////////////
FltError FltRecordWriter::
write_instance_def(FltHeader *header, int instance_index) {
  bool inserted = _instances_written.insert(instance_index).second;

  if (!inserted) {
    // It's already been written.
    return FE_ok;
  }

  FltInstanceDefinition *instance = header->get_instance(instance_index);
  if (instance == (FltInstanceDefinition *)NULL) {
    assert(!flt_error_abort);
    return FE_undefined_instance;
  }

  return instance->write_record_and_children(*this);
}
