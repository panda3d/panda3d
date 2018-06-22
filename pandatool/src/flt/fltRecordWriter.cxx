/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltRecordWriter.cxx
 * @author drose
 * @date 2000-08-24
 */

#include "fltRecordWriter.h"
#include "fltInstanceDefinition.h"
#include "fltHeader.h"
#include "config_flt.h"

#include "datagram.h"

#include <assert.h>

// Don't attempt to write more than this number of bytes in one record.  If
// the record requires more than this, use continuation records.
static const int max_write_length = 65532;

/**
 *
 */
FltRecordWriter::
FltRecordWriter(std::ostream &out) :
  _out(out)
{
}

/**
 *
 */
FltRecordWriter::
~FltRecordWriter() {
}

/**
 * Sets the opcode associated with the current record.
 */
void FltRecordWriter::
set_opcode(FltOpcode opcode) {
  _opcode = opcode;
}

/**
 * Sets the datagram that will be written when advance() is called.
 */
void FltRecordWriter::
set_datagram(const Datagram &datagram) {
  _datagram = datagram;
}

/**
 * Returns a modifiable reference to the datagram associated with the current
 * record.  This datagram should then be stuffed with data corresponding to
 * the data in the record, in preparation for calling advance() to write the
 * data.
 */
Datagram &FltRecordWriter::
update_datagram() {
  return _datagram;
}

/**
 * Writes the current record to the flt file, and resets the current record to
 * receive new data.  Returns FE_ok on success, or something else on error.
 */
FltError FltRecordWriter::
advance() {
  int start_byte = 0;
  int write_length =
    std::min((int)_datagram.get_length() - start_byte, max_write_length - header_size);
  FltOpcode opcode = _opcode;

  do {
    if (flt_cat.is_debug()) {
      flt_cat.debug()
        << "Writing " << opcode << " of length "
        << write_length + header_size << "\n";
    }

    // Build a mini-datagram to write the header.
    Datagram dg;
    dg.add_be_int16(opcode);
    dg.add_be_int16(write_length + header_size);

    nassertr((int)dg.get_length() == header_size, FE_internal);

    _out.write((const char *)dg.get_data(), dg.get_length());
    if (_out.fail()) {
      assert(!flt_error_abort);
      return FE_write_error;
    }

    // Now write the rest of the record.
    _out.write((const char *)_datagram.get_data() + start_byte, write_length);
    if (_out.fail()) {
      assert(!flt_error_abort);
      return FE_write_error;
    }

    start_byte += write_length;
    write_length =
      std::min((int)_datagram.get_length() - start_byte, max_write_length - header_size);
    opcode = FO_continuation;
  } while (write_length > 0);

  _datagram.clear();
  _opcode = FO_none;

  return FE_ok;
}

/**
 * A convenience function to quickly write a simple record that consists of an
 * opcode and possibly a datagram.
 */
FltError FltRecordWriter::
write_record(FltOpcode opcode, const Datagram &datagram) {
  _opcode = opcode;
  _datagram = datagram;
  return advance();
}

/**
 * Ensures that the given instance definition has already been written to the
 * file.  If it has not, writes it now.
 */
FltError FltRecordWriter::
write_instance_def(FltHeader *header, int instance_index) {
  bool inserted = _instances_written.insert(instance_index).second;

  if (!inserted) {
    // It's already been written.
    return FE_ok;
  }

  FltInstanceDefinition *instance = header->get_instance(instance_index);
  if (instance == nullptr) {
    assert(!flt_error_abort);
    return FE_undefined_instance;
  }

  return instance->write_record_and_children(*this);
}
