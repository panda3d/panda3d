// Filename: iffInputFile.cxx
// Created by:  drose (24Apr01)
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

#include "iffInputFile.h"
#include "iffGenericChunk.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "virtualFileSystem.h"

TypeHandle IffInputFile::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
IffInputFile::
IffInputFile() {
  _input = (istream *)NULL;
  _owns_istream = false;
  _eof = true;
  _unexpected_eof = false;
  _bytes_read = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
IffInputFile::
~IffInputFile() {
  if (_owns_istream) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_read_file(_input);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::open_read
//       Access: Public
//  Description: Attempts to open the indicated filename for reading.
//               Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool IffInputFile::
open_read(Filename filename) {
  filename.set_binary();

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *in = vfs->open_read_file(filename, true);
  if (in == (istream *)NULL) {
    return false;
  }

  set_input(in, true);
  set_filename(filename);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::set_input
//       Access: Public
//  Description: Sets up the input to use an arbitrary istream.  If
//               owns_istream is true, the istream will be deleted
//               (via vfs->close_read_file()) when the IffInputFile
//               destructs.
////////////////////////////////////////////////////////////////////
void IffInputFile::
set_input(istream *input, bool owns_istream) {
  if (_owns_istream) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_read_file(_input);
  }
  _input = input;
  _owns_istream = owns_istream;
  _eof = false;
  _unexpected_eof = false;
  _bytes_read = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_int8
//       Access: Public
//  Description: Extracts a signed 8-bit integer.
////////////////////////////////////////////////////////////////////
PN_int8 IffInputFile::
get_int8() {
  Datagram dg;
  if (!read_bytes(dg, 1)) {
    return 0;
  }
  DatagramIterator dgi(dg);
  return dgi.get_int8();
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_uint8
//       Access: Public
//  Description: Extracts an unsigned 8-bit integer.
////////////////////////////////////////////////////////////////////
PN_uint8 IffInputFile::
get_uint8() {
  Datagram dg;
  if (!read_bytes(dg, 1)) {
    return 0;
  }
  DatagramIterator dgi(dg);
  return dgi.get_int8();
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_be_int16
//       Access: Public
//  Description: Extracts a signed 16-bit big-endian integer.
////////////////////////////////////////////////////////////////////
PN_int16 IffInputFile::
get_be_int16() {
  Datagram dg;
  if (!read_bytes(dg, 2)) {
    return 0;
  }
  DatagramIterator dgi(dg);
  return dgi.get_be_int16();
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_be_int32
//       Access: Public
//  Description: Extracts a signed 32-bit big-endian integer.
////////////////////////////////////////////////////////////////////
PN_int32 IffInputFile::
get_be_int32() {
  Datagram dg;
  if (!read_bytes(dg, 4)) {
    return 0;
  }
  DatagramIterator dgi(dg);
  return dgi.get_be_int32();
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_be_uint16
//       Access: Public
//  Description: Extracts an unsigned 16-bit big-endian integer.
////////////////////////////////////////////////////////////////////
PN_uint16 IffInputFile::
get_be_uint16() {
  Datagram dg;
  if (!read_bytes(dg, 2)) {
    return 0;
  }
  DatagramIterator dgi(dg);
  return dgi.get_be_uint16();
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_be_uint32
//       Access: Public
//  Description: Extracts an unsigned 32-bit big-endian integer.
////////////////////////////////////////////////////////////////////
PN_uint32 IffInputFile::
get_be_uint32() {
  Datagram dg;
  if (!read_bytes(dg, 4)) {
    return 0;
  }
  DatagramIterator dgi(dg);
  return dgi.get_be_uint32();
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_be_float32
//       Access: Public
//  Description: Extracts a 32-bit big-endian single-precision
//               floating-point number.
////////////////////////////////////////////////////////////////////
PN_stdfloat IffInputFile::
get_be_float32() {
  Datagram dg;
  if (!read_bytes(dg, 4)) {
    return 0;
  }
  DatagramIterator dgi(dg);
  return dgi.get_be_float32();
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_string
//       Access: Public
//  Description: Extracts a null-terminated string.
////////////////////////////////////////////////////////////////////
string IffInputFile::
get_string() {
  string result;
  char byte;
  while (read_byte(byte)) {
    if (byte == 0) {
      break;
    }
    result += byte;
  }

  align();
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_id
//       Access: Public
//  Description: Extracts a 4-character IFF ID.
////////////////////////////////////////////////////////////////////
IffId IffInputFile::
get_id() {
  Datagram dg;
  if (!read_bytes(dg, 4)) {
    return IffId();
  }
  const char *id = (const char *)dg.get_data();
  return IffId(id);
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_chunk
//       Access: Public
//  Description: Reads a single IffChunk, determining its type based
//               on its ID.  Allocates and returns a new IffChunk
//               object of the appropriate type.  Returns NULL if EOF
//               is reached before the chunk can be read completely,
//               or if there is some other error in reading the chunk.
////////////////////////////////////////////////////////////////////
PT(IffChunk) IffInputFile::
get_chunk() {
  if (is_eof()) {
    return (IffChunk *)NULL;
  }

  IffId id = get_id();
  PN_uint32 length = get_be_uint32();

  if (!is_eof()) {
    PT(IffChunk) chunk = make_new_chunk(id);
    chunk->set_id(id);

    size_t start_point = get_bytes_read();
    size_t end_point = start_point + length;

    if (chunk->read_iff(this, end_point)) {
      if (is_eof()) {
        if (!_unexpected_eof) {
          nout << "Unexpected EOF on file reading " << *chunk << "\n";
          _unexpected_eof = true;
        }
        return (IffChunk *)NULL;
      }

      size_t num_bytes_read = get_bytes_read() - start_point;
      if (num_bytes_read > length) {
        nout << *chunk << " read " << num_bytes_read
             << " instead of " << length << " bytes.\n";
        return (IffChunk *)NULL;

      } else if (num_bytes_read < length) {
        size_t skip_count = length - num_bytes_read;
        nout << "Ignoring " << skip_count << " bytes at the end of "
             << *chunk << "\n";
        skip_bytes(skip_count);
      }
      return chunk;
    }
  }

  return (IffChunk *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::get_subchunk
//       Access: Public
//  Description: Similar to get_chunk(), except the chunk size is only
//               a 16-bit number instead of 32-bit, and it takes a
//               context, which is the chunk in which this chunk is
//               encountered.  The parent chunk may (or may not)
//               decide what kind of chunk is meant by the various
//               id's encountered.
////////////////////////////////////////////////////////////////////
PT(IffChunk) IffInputFile::
get_subchunk(IffChunk *context) {
  if (is_eof()) {
    return (IffChunk *)NULL;
  }

  IffId id = get_id();
  PN_uint16 length = get_be_uint16();

  if (!is_eof()) {
    PT(IffChunk) chunk = context->make_new_chunk(this, id);
    chunk->set_id(id);

    size_t start_point = get_bytes_read();
    size_t end_point = start_point + length;

    if (chunk->read_iff(this, end_point)) {
      if (is_eof()) {
        if (!_unexpected_eof) {
          nout << "Unexpected EOF on file reading " << *chunk << "\n";
          _unexpected_eof = true;
        }
        return (IffChunk *)NULL;
      }

      size_t num_bytes_read = get_bytes_read() - start_point;
      if (num_bytes_read > length) {
        nout << *chunk << " read " << num_bytes_read
             << " instead of " << length << " bytes.\n";
        return (IffChunk *)NULL;

      } else if (num_bytes_read < length) {
        size_t skip_count = length - num_bytes_read;
        nout << "Ignoring " << skip_count << " bytes at the end of "
             << *chunk << "\n";
        skip_bytes(skip_count);
      }
      return chunk;
    }
  }

  return (IffChunk *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::read_byte
//       Access: Public
//  Description: Reads a single byte.  Returns true if successful,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool IffInputFile::
read_byte(char &byte) {
  if (is_eof()) {
    return false;
  }

  _input->get(byte);
  _bytes_read++;
  _eof = _input->eof() || _input->fail();
  return !is_eof();
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::read_bytes
//       Access: Public
//  Description: Reads a series of bytes, and stores them in the
//               indicated Datagram.  Returns true if successful,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool IffInputFile::
read_bytes(Datagram &datagram, int length) {
  if (is_eof()) {
    return false;
  }

  char *buffer = new char[length];
  _input->read(buffer, length);
  _eof = (_input->gcount() != length);
  if (is_eof()) {
    return false;
  }

  _bytes_read += length;
  datagram = Datagram(buffer, length);
  delete[] buffer;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::skip_bytes
//       Access: Public
//  Description: Reads a series of bytes, but does not store them.
//               Returns true if successful, false otherwise.
////////////////////////////////////////////////////////////////////
bool IffInputFile::
skip_bytes(int length) {
  if (is_eof()) {
    return false;
  }

  char byte;
  while (length > 0 && !is_eof()) {
    read_byte(byte);
    length--;
  }

  return !is_eof();
}

////////////////////////////////////////////////////////////////////
//     Function: IffInputFile::make_new_chunk
//       Access: Protected, Virtual
//  Description: Allocates and returns a new chunk of the appropriate
//               type based on the given ID.
////////////////////////////////////////////////////////////////////
IffChunk *IffInputFile::
make_new_chunk(IffId) {
  return new IffGenericChunk;
}
