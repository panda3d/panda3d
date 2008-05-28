// Filename: fltRecordReader.cxx
// Created by:  drose (24Aug00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "fltRecordReader.h"
#include "config_flt.h"

#include "datagramIterator.h"

#include <assert.h>

////////////////////////////////////////////////////////////////////
//     Function: FltRecordReader::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltRecordReader::
FltRecordReader(istream &in) :
  _in(in)
{
  _opcode = FO_none;
  _record_length = 0;
  _iterator = (DatagramIterator *)NULL;
  _state = S_begin;
  _next_error = FE_ok;
  _next_opcode = FO_none;
  _next_record_length = 0;

  // Read the first header to get us going.
  read_next_header();
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordReader::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
FltRecordReader::
~FltRecordReader() {
  if (_iterator != (DatagramIterator *)NULL) {
    delete _iterator;
    _iterator = (DatagramIterator *)NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordReader::get_opcode
//       Access: Public
//  Description: Returns the opcode associated with the current
//               record.
////////////////////////////////////////////////////////////////////
FltOpcode FltRecordReader::
get_opcode() const {
  nassertr(_state == S_normal, FO_none);
  return _opcode;
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordReader::get_iterator
//       Access: Public
//  Description: Returns an iterator suitable for extracting data from
//               the current record.
////////////////////////////////////////////////////////////////////
DatagramIterator &FltRecordReader::
get_iterator() {
  nassertr(_state == S_normal, *_iterator);
  return *_iterator;
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordReader::get_datagram
//       Access: Public
//  Description: Returns the datagram representing the entire record,
//               less the four-byte header.
////////////////////////////////////////////////////////////////////
const Datagram &FltRecordReader::
get_datagram() {
#ifndef NDEBUG
  static Datagram bogus_datagram;
  nassertr(_state == S_normal, bogus_datagram);
#endif
  return _iterator->get_datagram();
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordReader::get_record_length
//       Access: Public
//  Description: Returns the entire length of the record, including
//               the four-byte header.
////////////////////////////////////////////////////////////////////
int FltRecordReader::
get_record_length() const {
  return _record_length;
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordReader::advance
//       Access: Public
//  Description: Extracts the next record from the file.  Returns true
//               if there is another record, or false if the end of
//               file has been reached.
////////////////////////////////////////////////////////////////////
FltError FltRecordReader::
advance(bool ok_eof) {
  if (_state == S_eof) {
    assert(!flt_error_abort);
    return FE_end_of_file;
  }
  if (_state == S_error) {
    assert(!flt_error_abort);
    return FE_read_error;
  }
  if (_iterator != (DatagramIterator *)NULL) {
    delete _iterator;
    _iterator = (DatagramIterator *)NULL;
  }

  if (_next_error == FE_end_of_file) {
    _state = S_eof;
    if (ok_eof) {
      return FE_ok;
    }
    assert(!flt_error_abort);
    return FE_end_of_file;

  } else if (_next_error != FE_ok) {
    _state = S_error;
    assert(!flt_error_abort);
    return _next_error;
  }

  _opcode = _next_opcode;
  _record_length = _next_record_length;

  if (flt_cat.is_debug()) {
    flt_cat.debug()
      << "Reading " << _opcode
      << " of length " << _record_length << "\n";
  }

  // And now read the full record based on the length.
  int length = _next_record_length - header_size;
  char *buffer = new char[length];
  if (length > 0) {
    _in.read(buffer, length);
  }
  _datagram = Datagram(buffer, length);
  delete[] buffer;

  if (_in.eof()) {
    _state = S_eof;
    assert(!flt_error_abort);
    return FE_end_of_file;
  }

  if (_in.fail()) {
    _state = S_error;
    assert(!flt_error_abort);
    return FE_read_error;
  }

  // Check out the next header in case it's a continuation.
  read_next_header();
  while (_next_error == FE_ok && _next_opcode == FO_continuation) {
    if (flt_cat.is_debug()) {
      flt_cat.debug()
        << "Reading continuation of length " << _next_record_length << "\n";
    }

    // Read the continuation and tack it on.
    _record_length += _next_record_length;
    length = _next_record_length - header_size;

    buffer = new char[length];
    if (length > 0) {
      _in.read(buffer, length);
    }
    _datagram.append_data(buffer, length);
    delete[] buffer;

    if (_in.eof()) {
      _state = S_eof;
      assert(!flt_error_abort);
      return FE_end_of_file;
    }

    if (_in.fail()) {
      _state = S_error;
      assert(!flt_error_abort);
      return FE_read_error;
    }

    read_next_header();
  }

  // Finally, create a new iterator to read this record.
  _iterator = new DatagramIterator(_datagram);
  _state = S_normal;

  return FE_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordReader::eof
//       Access: Public
//  Description: Returns true if end-of-file has been reached without
//               error.
////////////////////////////////////////////////////////////////////
bool FltRecordReader::
eof() const {
  return _state == S_eof;
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordReader::error
//       Access: Public
//  Description: Returns true if some error has been encountered while
//               reading (for instance, a truncated file).
////////////////////////////////////////////////////////////////////
bool FltRecordReader::
error() const {
  return _state == S_error;
}

////////////////////////////////////////////////////////////////////
//     Function: FltRecordReader::read_next_header
//       Access: Private
//  Description: Reads the four-byte header for the next record, which
//               contains the next opcode and record length.
//
//               We need read the next header in advance so we can
//               check to see if it happens to be a continuation
//               record.  If it is, we will need to concatenate the
//               records together before returning.
////////////////////////////////////////////////////////////////////
void FltRecordReader::
read_next_header() {
  char bytes[header_size];
  _in.read(bytes, header_size);

  if (_in.eof()) {
    _next_error = FE_end_of_file;
    return;

  } else if (_in.fail()) {
    _next_error = FE_read_error;
    return;
  }

  // Now extract out the opcode and length.
  Datagram dg(bytes, header_size);
  DatagramIterator dgi(dg);
  _next_opcode = (FltOpcode)dgi.get_be_int16();
  _next_record_length = dgi.get_be_uint16();

  if (_next_record_length < header_size) {
    _next_error = FE_invalid_record;
    return;
  }
}
