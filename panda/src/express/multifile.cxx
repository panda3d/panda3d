// Filename: multifile.cxx
// Created by:  mike (09Jan97)
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
#if defined(WIN32_VC) && !defined(NO_PCH)
#include "express_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include <pandabase.h>
#include "config_express.h"
#include "error_utils.h"

#include <algorithm>
#include <errno.h>
#endif

#include "multifile.h"

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
PN_uint32 Multifile::_magic_number = 0xbeeffeeb;

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Memfile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Multifile::Memfile::
Memfile(void) {
  reset();
  _header_length_buf_length = sizeof(_header_length);
  _header_length_buf = new char[_header_length_buf_length];
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Memfile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Multifile::Memfile::
~Memfile(void) {
  if (_buffer != (char *)0L)
    delete _buffer;
  _buffer = (char *)0L;
  delete _header_length_buf;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Memfile::reset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Multifile::Memfile::
reset(void) {
  _header_length_parsed = false;
  _header_parsed = false;
  _header_length = 0;
  _buffer_length = 0;
  //if (_buffer != (char *)0L)
  //  delete _buffer;
  _buffer = (char *)0L;
  _file_open = false;
  _bytes_written = 0;
  //_datagram.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Memfile::parse_header_length
//       Access: Public
//  Description: Fills up _datagram until it has sizeof(_header_length)
//               bytes and then extracts _header_length from _datagram.
//               Returns true when this has been accomplished.
//               Advances start to end of header_length.
////////////////////////////////////////////////////////////////////
bool Multifile::Memfile::
parse_header_length(char *&start, int &size) {
  if (_header_length_parsed == true)
    return true;

  int bytes_so_far = _datagram.get_length();
  if (bytes_so_far + size < _header_length_buf_length) {
    _datagram.append_data(start, size);
    start += size;
    size = 0;
    return false;
  }

  _datagram.append_data(start, _header_length_buf_length - bytes_so_far);
  nassertr((int)_datagram.get_length() == _header_length_buf_length, false);

  // Advance start and adjust size
  nassertr(_header_length_buf_length >= bytes_so_far, false);
  start += (_header_length_buf_length - bytes_so_far);
  nassertr(size >= _header_length_buf_length, false);
  size -= (_header_length_buf_length - bytes_so_far);

  DatagramIterator di(_datagram);
  _header_length = di.get_int32();

  express_cat.debug()
    << "Multifile::Memfile::parse_header_length() - header length: "
    << _header_length << endl;

  nassertr(_header_length > _header_length_buf_length + (int)sizeof(_buffer_length), false);

  _header_length_parsed = true;

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Memfile::parse_header
//       Access: Public
//  Description: Returns true when a complete header has been parsed.
//               Advances the start pointer to the end of the header.
////////////////////////////////////////////////////////////////////
bool Multifile::Memfile::
parse_header(char *&start, int& size) {
  if (_header_parsed == true)
    return true;

  // Determine header length
  if (parse_header_length(start, size) == false)
    return false;

  int bytes_so_far = _datagram.get_length();

  // Make sure we don't exceed the length of the header
  int tsize = size;
  if ((size + bytes_so_far) >= _header_length) {
    nassertr(bytes_so_far <= _header_length, false);
    tsize = _header_length - bytes_so_far;
    _header_parsed = true;
  }

  // Accumulate bytes collected so far
  _datagram.append_data(start, tsize);

  // Fill in data for the memfile
  if (_header_parsed == true) {
    nassertr((int)_datagram.get_length() == _header_length, false);
    DatagramIterator di(_datagram);
    PN_int32 header_length = di.get_int32();
    nassertr(header_length == _header_length, false);
    _name = di.extract_bytes(_header_length - _header_length_buf_length -
                             sizeof(_buffer_length));
    _buffer_length = di.get_int32();
    nassertr(_buffer_length >= 0, false);

    express_cat.debug()
      << "Multifile::Memfile::parse_header() - Got a header for mem "
      << "file: " << _name << " header length: " << _header_length
      << " buffer length: " << _buffer_length << endl;

    // Advance start pointer to the end of the header
    start += tsize;
    nassertr(size >= tsize, false);
    size -= tsize;
    _datagram.clear();
  }

  return _header_parsed;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Memfile::read
//       Access: Public
//  Description: Reads from an individual file
////////////////////////////////////////////////////////////////////
bool Multifile::Memfile::
read(const Filename &name) {
  ifstream read_stream;
  if (!name.open_read(read_stream)) {
    express_cat.error()
      << "Multifile::Memfile() - Failed to open input file: "
      << name << endl;
    return false;
  }

  if (express_cat.is_debug())
    express_cat.debug()
      << "Multifile::Memfile::read() - Reading file: " << name << endl;

  _header_length = name.length() + sizeof(_header_length) +
        sizeof(_buffer_length);
  _name = name;

  // Determine the length of the file
  read_stream.seekg(0, ios::end);
  _buffer_length = read_stream.tellg();
  _buffer = new char[_buffer_length];

  // Read the file
  read_stream.seekg(0, ios::beg);
  read_stream.read(_buffer, _buffer_length);

  return (_buffer_length > 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Memfile::read_from_multifile
//       Access: Public
//  Description: Reads a Memfile from an open Multifile ifstream
////////////////////////////////////////////////////////////////////
bool Multifile::Memfile::
read_from_multifile(ifstream &read_stream) {
  read_stream.read(_header_length_buf, _header_length_buf_length);
  char *start = _header_length_buf;
  int size = _header_length_buf_length;
  if (parse_header_length(start, size) == false) {
    express_cat.error()
      << "Multifile::Memfile::read_from_multifile() - invalid header "
      << "length" << endl;
    return false;
  }

  // Read the rest of the header
  char *header_buf = new char[_header_length - _header_length_buf_length];
  read_stream.read(header_buf, _header_length - _header_length_buf_length);
  start = header_buf;
  size = _header_length;
  if (parse_header(start, size) == false) {
    delete header_buf;
    express_cat.error()
      << "Multifile::Memfile::read_from_multifile() - Invalid header"
      << endl;
    return false;
  }

  delete header_buf;

  express_cat.debug()
    << "Multifile::Memfile::read_from_multifile() - Got a valid Memfile "
    << "header: name: " << _name << " length: " << _buffer_length << endl;

  _buffer = new char[_buffer_length];
  read_stream.read(_buffer, _buffer_length);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Memfile::write
//       Access: Public
//  Description: Writes to a individual file
////////////////////////////////////////////////////////////////////
bool Multifile::Memfile::
write(const Filename &rel_path) {
  ofstream write_stream;
  Filename name = rel_path.get_fullpath() + _name.get_fullpath();
  name.set_binary();
  name.make_dir();
  if (!name.open_write(write_stream)) {
    express_cat.error()
      << "Multifile::Memfile::write() - Failed to open output file: "
      << name << endl;
    return false;
  }

  express_cat.debug()
    << "Writing to file: " << name << endl;

  write_stream.write(_buffer, _buffer_length);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Memfile::write
//       Access: Public
//  Description: Writes a Memfile to an open Multifile ofstream
////////////////////////////////////////////////////////////////////
void Multifile::Memfile::
write_to_multifile(ofstream &write_stream) {
  express_cat.debug()
    << "Writing: " << _name << " to multifile" << endl;

  _datagram.clear();
  _datagram.add_int32(_header_length);
  string path_name = _name.get_fullpath();
  _datagram.append_data(path_name.c_str(), path_name.length());
  _datagram.add_int32(_buffer_length);

  string msg = _datagram.get_message();
  write_stream.write((char *)msg.data(), msg.length());
  write_stream.write(_buffer, _buffer_length);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Memfile::write
//       Access: Public
//  Description: Returns true when the memfile has been parsed and
//               written to disk.
//               Advances the start pointer as it writes.
////////////////////////////////////////////////////////////////////
int Multifile::Memfile::
write(char *&start, int &size, const Filename &rel_path) {
  // Make sure we've got a complete header first
  if (parse_header(start, size) == false) {
    return EU_ok;
  }

  // Try to open the file for writing
  if (_file_open == false) {
    Filename name = rel_path.get_fullpath() + _name.get_fullpath();
    name.set_binary();
    name.make_dir();
    if (express_cat.is_debug())
      express_cat.debug()
        << "Multifile::Memfile::write() - Opening mem file: " << name
        << " for writing" << endl;
    if ((_file_open = name.open_write(_write_stream)) == false) {
      express_cat.error()
        << "Multfile::Memfile::write() - Couldn't open file: "
        << name << " : " << strerror(errno) << endl;
      return get_write_error();
    }
    _file_open = true;
  }

  // Don't write more than the buffer length
  int done = EU_ok;
  int tsize = size;
  nassertr(_buffer_length >= _bytes_written, false);
  int missing_bytes = _buffer_length - _bytes_written;
  if (size >= missing_bytes) {
    tsize = missing_bytes;
    done = EU_success;
  }

  _write_stream.write(start, tsize);
  start += tsize;
  _bytes_written += tsize;
  nassertr(size >= tsize, false);
  size -= tsize;

  if (done == EU_success) {
    _write_stream.close();
    express_cat.debug()
      << "Multifile::Memfile::write() - Closing mem file" << endl;
  }

  return done;
}



////////////////////////////////////////////////////////////////////
//     Function: Multifile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Multifile::
Multifile(void) {
  reset();
  _header_length = sizeof(_magic_number) + sizeof(_num_mfiles);
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Multifile::
~Multifile(void) {
  _files.erase(_files.begin(), _files.end());
  if (_current_mfile != NULL)
    delete _current_mfile;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::evaluate
//       Access: Public
//  Description: Checks for a valid compressed or uncompressed
//               Multifile.
////////////////////////////////////////////////////////////////////
int Multifile::
evaluate(const char *start, int size) {
  if (size < (int)sizeof(_magic_number)) {
    express_cat.debug()
      << "Multifile::evaluate() - not enough bytes to determine" << endl;
    return T_unknown;
  }

  Datagram dgram;
  dgram.append_data(start, sizeof(_magic_number));
  DatagramIterator di(dgram);
  PN_uint32 magic_number = di.get_uint32();
  if (magic_number == _magic_number)
    return T_valid;

  express_cat.debug()
    << "Invalid magic number: " << magic_number << " ("
    << _magic_number << ")" << endl;

  return T_invalid;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::parse_header
//       Access: Public
//  Description: Returns true when a complete header has been parsed.
////////////////////////////////////////////////////////////////////
int Multifile::
parse_header(char *&start, int &size) {
  if (_header_parsed == true)
    return EU_success;

  int dgramsize = _datagram.get_length();
  int tsize = size;

  // Make sure we don't exceed the length of the header
  nassertr(_header_length >= dgramsize, EU_error_abort);
  int missing_bytes = _header_length - dgramsize;
  if (size >= missing_bytes) {
    tsize = missing_bytes;
    _header_parsed = true;
  }

  // Accumulate bytes collected so far
  _datagram.append_data(start, tsize);

  // Verify magic number
  if (_header_parsed == true) {
    DatagramIterator di(_datagram);
    PN_uint32 magic_number = di.get_uint32();
    if (magic_number != _magic_number) {
      express_cat.error()
        << "Multifile::parse_header() - Invalid magic number: "
        << magic_number << " (" << _magic_number << ")" << endl;
      return EU_error_abort;
    }
    _num_mfiles = di.get_int32();
    if (_num_mfiles <= 0) {
      express_cat.debug()
        << "Multifile::parse_header() - No memfiles in multifile"
        << endl;
      return EU_error_file_empty;
    }

    // Advance start pointer to the end of the header
    start += tsize;
    nassertr(size >= tsize, false);
    size -= tsize;
    _datagram.clear();
    return EU_success;
  }

  return EU_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::add
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Multifile::
add(const Filename &name) {
  Memfile *mfile = new Memfile;
  if (mfile->read(name)) {
    _files.push_back(mfile);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::remove
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Multifile::
remove(const Filename &name) {
  MemfileList::iterator found;
  found = find_if(_files.begin(), _files.end(), MemfileMatch(name));
  if (found != _files.end()) {
    _files.erase(found);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::has_file
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Multifile::
has_file(const Filename &name) {
  MemfileList::iterator found;
  found = find_if(_files.begin(), _files.end(), MemfileMatch(name));
  return (found != _files.end());
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::read
//       Access: Public
//  Description: Reads a multifile from disk
////////////////////////////////////////////////////////////////////
bool Multifile::
read(Filename &name) {

  // Open the multifile for reading
  ifstream read_stream;
  name.set_binary();
  if (!name.open_read(read_stream)) {
    express_cat.error()
      << "Multifile::read() - Failed to open input file: "
      << name << endl;
    return false;
  }

  // Check for a valid header
  char *buffer = new char[_header_length];
  read_stream.read(buffer, _header_length);
  char *start = buffer;
  int len = _header_length;
  int ret = parse_header(start, len);
  delete buffer;
  if (ret < 0) {
    express_cat.error()
      << "Multifile::read() - invalid header" << endl;
    return false;
  }

  if (_num_mfiles <= 0) {
    express_cat.error()
      << "Multfile::read() - No files in Multfile: " << name << endl;
    return false;
  }

  // Read all the Memfiles
  for (int i = 0; i < _num_mfiles; i++) {
    Memfile *mfile = new Memfile;
    mfile->read_from_multifile(read_stream);
    _files.push_back(mfile);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Multifile::
write(Filename name) {

  ofstream write_stream;
  name.set_binary();
  if (!name.open_write(write_stream)) {
    express_cat.error()
      << "Multifile::write() - Error opening file: " << name << endl;
    return false;
  }

  _num_mfiles = _files.size();
  if (_num_mfiles == 0) {
    express_cat.debug()
      << "No files in Multifile to write" << endl;
    return false;
  }

  express_cat.debug()
    << "Multifile::write() - Writing multifile: " << name << endl;

  write_header(write_stream);

  MemfileList::iterator i;
  for (i = _files.begin(); i != _files.end(); ++i)
    (*i)->write_to_multifile(write_stream);

  write_stream.close();

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::write
//       Access: Public
//  Description: Returns true when all the memfiles have been
//               written.
//               Advances the start pointer as it writes.
////////////////////////////////////////////////////////////////////
int Multifile::
write(char *&start, int &size, const Filename &rel_path) {
  // Make sure we have a complete header first
  int parse_ret = parse_header(start, size);
  if (parse_ret < 0) {
    express_cat.error()
     << "Multifile::write() - bad header" << endl;
    return parse_ret;
  }

  while (_num_mfiles > 0) {
    if (_current_mfile == NULL) {
      _current_mfile = new Memfile;
    }
    int write_ret = _current_mfile->write(start, size, rel_path);
    if (write_ret == EU_success) {
      _num_mfiles--;
      delete _current_mfile;
      _current_mfile = NULL;
    } else {
      if (write_ret < 0) {
        express_cat.error()
          << "Multifile::write() - bad write: " << write_ret << endl;
      }
      return write_ret;
    }
  }

  return EU_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::write_extract
//       Access: Public
//  Description: Returns true when entire Multifile has been
//               extracted to disk files.
////////////////////////////////////////////////////////////////////
bool Multifile::
write_extract(char *&start, int &size, const Filename &rel_path) {
  int parse_ret = parse_header(start, size);
  if (parse_ret < 0)
    return false;
  if (_current_mfile == (Memfile *)0L)
    _current_mfile = new Memfile;
  for (;;) {
    if (_current_mfile->write(start, size, rel_path) == false)
      return false;
    if (++_mfiles_written == _num_mfiles)
      return true;
    _current_mfile->reset();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::reset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Multifile::
reset(void) {
  _header_parsed = false;
  _num_mfiles = 0;
  _current_mfile = (Memfile *)0L;
  _datagram.clear();
  _files.erase(_files.begin(), _files.end());
  _mfiles_written = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::extract
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Multifile::
extract(const Filename &name, const Filename &rel_path) {
  MemfileList::iterator found;
  found = find_if(_files.begin(), _files.end(), MemfileMatch(name));
  if (found != _files.end()) {
    (*found)->write(rel_path);
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Multifile::extract_all
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Multifile::
extract_all(const Filename &rel_path) {
  express_cat.debug()
    << "Multifile::extract_all() - Extracting all files" << endl;

  MemfileList::iterator i;
  for (i = _files.begin(); i != _files.end(); ++i)
    (*i)->write(rel_path);
}
