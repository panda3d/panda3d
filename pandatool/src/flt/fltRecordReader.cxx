// Filename: fltRecordReader.cxx
// Created by:  drose (24Aug00)
// 
////////////////////////////////////////////////////////////////////

#include "fltRecordReader.h"
#include "config_flt.h"

#include <datagramIterator.h>

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

  // Get the first four bytes of the record.  This will be the opcode
  // and length.
  static const int header_size = 4;
  char bytes[header_size];
  _in.read(bytes, header_size);

  if (_in.eof()) {
    _state = S_eof;
    if (ok_eof) {
      return FE_ok;
    }
    assert(!flt_error_abort);
    return FE_end_of_file;

  } else if (_in.fail()) {
    _state = S_error;
    assert(!flt_error_abort);
    return FE_read_error;
  }

  // Now extract out the opcode and length.
  Datagram dg(bytes, header_size);
  DatagramIterator dgi(dg);
  _opcode = (FltOpcode)dgi.get_be_int16();
  _record_length = dgi.get_be_uint16();

  if (_record_length < header_size) {
    assert(!flt_error_abort);
    return FE_invalid_record;
  }

  if (flt_cat.is_debug()) {
    flt_cat.debug()
      << "Reading " << _opcode << " of length " << _record_length << "\n";
  }

  // And now read the full record based on the length.
  int length = _record_length - header_size;
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

