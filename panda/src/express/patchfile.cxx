// Filename: patchfile.cxx
// Created by:  mike, darren (09Jan97)
//
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95,96,97,98
// Walt Disney Imagineering, Inc.
//
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "patchfile.h"
#include "config_express.h"

#include <stdio.h> // for tempnam

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
const PN_uint32 Patchfile::_magic_number = 0xfeebfaab;

const PN_uint32 Patchfile::_HASHTABLESIZE = PN_uint32(1) << 16;
const PN_uint32 Patchfile::_footprint_length = 16;
const PN_uint32 Patchfile::_NULL_VALUE = PN_uint32(0) - 1;

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patchfile::
Patchfile(void) {
  PT(Buffer) buffer = new Buffer(patchfile_buffer_size);
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patchfile::
Patchfile(PT(Buffer) buffer) {
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::init
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Patchfile::
init(PT(Buffer) buffer) {
  nassertv(!buffer.is_null());
  _buffer = buffer;

  // get all set up with a temp file
  char *temp_name = tempnam(NULL, "pf");
  _temp_file_name = temp_name;
  _temp_file_name.set_binary();
  delete temp_name;

  reset();
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::reset
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void Patchfile::
reset(void) {
  _datagram.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patchfile::
~Patchfile(void) {
  _temp_file_name.unlink();
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::calc_hash
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PN_uint16 Patchfile::
calc_hash(const char *buffer) {
  PN_uint16 hash_value = 0;

  for(int i = 0; i < (int)_footprint_length; i++) {
    // this is probably not such a good hash. to be replaced
    hash_value ^= (*buffer) << (i % 8);
    buffer++;
  }

  return hash_value;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::build_hash_link_tables
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Patchfile::
build_hash_link_tables(const char *buffer_orig, PN_uint32 length_orig,
  PN_uint32 *hash_table, PN_uint32 *link_table) {

  PN_uint32 i;

  // clear hash table
  for(i = 0; i < _HASHTABLESIZE; i++) {
    hash_table[i] = _NULL_VALUE;
  }

  // clear link table
  for(i = 0; i < length_orig; i++) {
    link_table[i] = _NULL_VALUE;
  }

  // run through original file and hash each footprint
  for(i = 0; i < (length_orig - _footprint_length); i++) {
    PN_uint16 hash_value = calc_hash(&buffer_orig[i]);
    if (_NULL_VALUE == hash_table[hash_value]) {
      // hash entry is empty, store this offset
      hash_table[hash_value] = i;
    } else {
      // hash entry is taken, go to the link table
      PN_uint32 link_offset = hash_table[hash_value];
      while (_NULL_VALUE != link_table[link_offset]) {
        link_offset = link_table[link_offset];
      }
      link_table[link_offset] = i;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::calc_match_length
//       Access: Private
//  Description:
//               This function calculates the length of a match between
//               two strings of bytes
////////////////////////////////////////////////////////////////////
PN_uint32 Patchfile::
calc_match_length(const char* buf1, const char* buf2, PN_uint32 max_length) {
  PN_uint32 length = 0;
  while ((length < max_length) && (*buf1 == *buf2)) {
    buf1++, buf2++, length++;
  }
  return length;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::find_longest_match
//       Access: Private
//  Description:
//               This function will find the longest string in the
//               original file that matches a string in the new file.
////////////////////////////////////////////////////////////////////
void Patchfile::
find_longest_match(PN_uint32 new_pos, PN_uint32 &copy_offset, PN_uint32 &copy_length,
  PN_uint32 *hash_table, PN_uint32 *link_table, const char* buffer_orig,
  PN_uint32 length_orig, const char* buffer_new, PN_uint32 length_new) {

  // set length to a safe value
  copy_length = 0;

  // get offset of matching string (in orig file) from hash table
  PN_uint16 hash_value = calc_hash(&buffer_new[new_pos]);

  // if no match, bail
  if (_NULL_VALUE == hash_table[hash_value])
    return;

  copy_offset = hash_table[hash_value];

  // calc match length
  copy_length = calc_match_length(&buffer_new[new_pos], &buffer_orig[copy_offset],
    min((length_new - new_pos),(length_orig - copy_offset)));

  // run through link table, see if we find any longer matches
  PN_uint32 match_offset, match_length;
  match_offset = link_table[copy_offset];

  while (match_offset != _NULL_VALUE) {
    match_length = calc_match_length(&buffer_new[new_pos], &buffer_orig[match_offset],
                      min((length_new - new_pos),(length_orig - match_offset)));

    // have we found a longer match?
    if (match_length > copy_length) {
      copy_offset = match_offset;
      copy_length = match_length;
    }

    // traverse the link table
    match_offset = link_table[match_offset];
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::emit_ADD
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Patchfile::
emit_ADD(ofstream &write_stream, PN_uint32 length, const char* buffer) {
//  cout << "ADD: " << length << " bytes" << endl;

  // write ADD length
  _datagram.clear();
  _datagram.add_uint32(length);
  string msg = _datagram.get_message();
  write_stream.write((char *)msg.data(), msg.length());

  // if there are bytes to add, add them
  if (length > 0) {
    _datagram.clear();
    _datagram.append_data(buffer, length);
    string msg = _datagram.get_message();
    write_stream.write((char *)msg.data(), msg.length());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::emit_COPY
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Patchfile::
emit_COPY(ofstream &write_stream, PN_uint32 length, PN_uint32 offset) {
//  cout << "COPY: " << length << " bytes at offset " << offset << endl;

  // write COPY length
  _datagram.clear();
  _datagram.add_uint32(length);
  string msg = _datagram.get_message();
  write_stream.write((char *)msg.data(), msg.length());

  if(length > 0) {
    // write COPY offset
    _datagram.clear();
    _datagram.add_uint32(offset);
    string msg2 = _datagram.get_message();
    write_stream.write((char *)msg2.data(), msg2.length());
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::build
//       Access: Public
//  Description:
//               This implementation uses the "greedy differencing
//               algorithm" described in the masters thesis
//               "Differential Compression: A Generalized Solution
//               for Binary Files" by Randal C. Burns (p.13).
//               For an original file of size M and a new file of
//               size N, this algorithm is O(M) in space and O(M*N)
//               in time.
////////////////////////////////////////////////////////////////////
bool Patchfile::
build(Filename &file_orig, Filename &file_new) {
  Filename patch_name;
  patch_name.set_binary();

  // Open the original file for read
  ifstream stream_orig;
  file_orig.set_binary();
  if (!file_orig.open_read(stream_orig)) {
    express_cat.error()
      << "Patchfile::build() - Failed to open file: " << file_orig << endl;
    return false;
  }

  // Open the new file for read
  ifstream stream_new;
  file_new.set_binary();
  if (!file_new.open_read(stream_new)) {
    express_cat.error()
      << "Patchfile::build() - Failed to open file: " << file_new << endl;
    return false;
  }

  // Open patch file for write
  ofstream write_stream;
  patch_name = file_orig.get_fullpath() + ".pch";
  if (!patch_name.open_write(write_stream)) {
    express_cat.error()
      << "Patchfile::build() - Failed to open file: " << patch_name << endl;
    return false;
  }

  // read in original file
  stream_orig.seekg(0, ios::end);
  int length_orig = stream_orig.tellg();
  char *buffer_orig = new char[length_orig];
  stream_orig.seekg(0, ios::beg);
  stream_orig.read(buffer_orig, length_orig);

  // read in new file
  stream_new.seekg(0, ios::end);
  int length_new = stream_new.tellg();
  char *buffer_new = new char[length_new];
  stream_new.seekg(0, ios::beg);
  stream_new.read(buffer_new, length_new);

  // close the original and new files (we have em in memory)
  stream_orig.close();
  stream_new.close();

  // allocate hash/link tables
  PN_uint32* hash_table = new PN_uint32[_HASHTABLESIZE];
  PN_uint32* link_table = new PN_uint32[length_orig];

  // build hash and link tables for original file
  build_hash_link_tables(buffer_orig, length_orig, hash_table, link_table);

  // write the patch file header
  // Strip the v# out of the filename
  // Save the original extension
  string ext = file_orig.get_extension();
  // Strip out the extension
  Filename tfile = file_orig.get_basename_wo_extension();
  // Now strip out the .v#
  string fname = tfile.get_basename_wo_extension();
  fname += ".";
  fname += ext;
  write_header(write_stream, fname);

  // run through new file
  PN_uint32 new_pos = 0;
  PN_uint32 ADD_offset = new_pos; // this is the offset for the start of ADD operations

  while (new_pos < (length_new - _footprint_length)) {

    // find best match for current position
    PN_uint32 COPY_offset, COPY_length;

    find_longest_match(new_pos, COPY_offset, COPY_length, hash_table, link_table,
      buffer_orig, length_orig, buffer_new, length_new);

    // if no match or match not longer than footprint length, skip to next byte
    if (COPY_length < _footprint_length) {
      // go to next byte
      new_pos++;
    } else {
      // emit ADD for all skipped bytes
      emit_ADD(write_stream, new_pos - ADD_offset, &buffer_new[ADD_offset]);

      // emit COPY for matching string
      emit_COPY(write_stream, COPY_length, COPY_offset);

      // skip past match in new_file
      new_pos += COPY_length;
      ADD_offset = new_pos;
    }
  }

  // are there still more bytes left in the new file?
  if ((int)ADD_offset != length_new) {
    // emit ADD for all remaining bytes
    emit_ADD(write_stream, length_new - ADD_offset, &buffer_new[ADD_offset]);

    // write null COPY
    emit_COPY(write_stream, 0, 0);
  }

  // write terminator (null ADD, null COPY)
  emit_ADD(write_stream, 0, NULL);
  emit_COPY(write_stream, 0, 0);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::apply
//       Access: Public
//  Description: Apply the patch to the file (original file and
//		 patch are destroyed in the process).
////////////////////////////////////////////////////////////////////
bool Patchfile::
apply(Filename &patch, Filename &file) {
  const int _header_length = sizeof(PN_uint32) + sizeof(PN_int32);

  // Open the patch file for read
  ifstream patch_stream;
  patch.set_binary();
  if (!patch.open_read(patch_stream)) {
    express_cat.error()
      << "Patchfile::apply() - Failed to open file: " << patch << endl;
    return false;
  }

  // Open the original file for read
  ifstream origfile_stream;
  file.set_binary();
  if (!file.open_read(origfile_stream)) {
    express_cat.error()
      << "Patchfile::apply() - Failed to open file: " << file << endl;
    return false;
  }

  // Open the temp file for write
  ofstream write_stream;
  Filename mofile = "patcher_temp_file";
  mofile.set_binary();
  if (!mofile.open_write(write_stream)) {
    express_cat.error()
      << "Patchfile::apply() - Failed to open file: " << mofile << endl;
    return false;
  }

  /////////////
  // read header, make sure the patch file is valid

  // check the magic number
  nassertr(_buffer->get_length() >= _header_length, false);
  patch_stream.read(_buffer->_buffer, _header_length);
  _datagram.clear();
  _datagram.append_data(_buffer->_buffer, _header_length);
  DatagramIterator di(_datagram);
  PN_uint32 magic_number = di.get_uint32();
  if (magic_number != _magic_number) {
    express_cat.error()
      << "Patchfile::apply() - invalid patch file: " << patch << endl;
    return false;
  }

  // check the filename
  PN_int32 name_length = di.get_int32();
  nassertr(_buffer->get_length() >= name_length, false);
  patch_stream.read(_buffer->_buffer, name_length);
  _datagram.clear();
  _datagram.append_data(_buffer->_buffer, name_length);
  DatagramIterator di2(_datagram);
  string name = di2.extract_bytes(name_length);
  if (name != file.get_basename()) {
    express_cat.error()
      << "Patchfile::apply() - patch intended for file: " << name
      << ", not file: " << file << endl;
    return false;
  }

  express_cat.debug()
    << "Patchfile::apply() - valid patchfile for file: " << name << endl;

  // Now patch the file using the given buffer
  int buflen = _buffer->get_length();
  int done = 0;
  PN_uint32 ADD_length;
  PN_uint32 COPY_length;
  PN_uint32 COPY_offset;

  while (!done)
  {
    ///////////
    // read # of ADD bytes
    nassertr(_buffer->get_length() >= (int)sizeof(ADD_length), false);
    patch_stream.read(_buffer->_buffer, sizeof(ADD_length));
    _datagram.clear();
    _datagram.append_data(_buffer->_buffer, sizeof(ADD_length));
    DatagramIterator di(_datagram);
    ADD_length = di.get_uint32();

    // if there are bytes to add, read them from patch file and write them to output
    if (0 != ADD_length) {
      PN_uint32 bytes_left = ADD_length;

      while (bytes_left > 0) {
        PN_uint32 bytes_this_time = ((int)bytes_left < buflen) ? bytes_left : buflen;
        patch_stream.read(_buffer->_buffer, bytes_this_time);
        write_stream.write(_buffer->_buffer, bytes_this_time);
        bytes_left -= bytes_this_time;
      }
    }

    ///////////
    // read # of COPY bytes
    nassertr(_buffer->get_length() >= (int)sizeof(COPY_length), false);
    patch_stream.read(_buffer->_buffer, sizeof(COPY_length));
    _datagram.clear();
    _datagram.append_data(_buffer->_buffer, sizeof(COPY_length));
    DatagramIterator di2(_datagram);
    COPY_length = di2.get_uint32();

    // if there are bytes to copy, read them from original file and write them to output
    if (0 != COPY_length) {
      // read copy offset
      nassertr(_buffer->get_length() >= (int)sizeof(COPY_offset), false);
      patch_stream.read(_buffer->_buffer, sizeof(COPY_offset));
      _datagram.clear();
      _datagram.append_data(_buffer->_buffer, sizeof(COPY_offset));
      DatagramIterator di(_datagram);
      COPY_offset = di.get_uint32();

      // seek to the offset
      origfile_stream.seekg(COPY_offset, ios::beg);

      // read the copy bytes from original file and write them to output
      PN_uint32 bytes_left = COPY_length;

      while (bytes_left > 0) {
        PN_uint32 bytes_this_time = ((int)bytes_left < buflen) ? bytes_left : buflen;
        origfile_stream.read(_buffer->_buffer, bytes_this_time);
        write_stream.write(_buffer->_buffer, bytes_this_time);
        bytes_left -= bytes_this_time;
      }
    }

    // if we got a pair of zero-length ADD and COPY blocks, we're done
    if ((0 == ADD_length) && (0 == COPY_length)) {
      done = 1;
    }
  }

  // close files
  patch_stream.close();
  origfile_stream.close();
  write_stream.close();

  // delete the patch file and the original file
  patch.unlink();
  file.unlink();

  // rename the temp file
  if (!mofile.rename_to(file)) {
    express_cat.error()
      << "Patchfile::apply() failed to rename temp file to: " << file
      << endl;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::find_longest_sequence
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Patchfile::
find_longest_sequence(Filename &infile, int &pos, int &len) const {
  // Open the file for read
  ifstream read_stream;
  infile.set_binary();
  if (!infile.open_read(read_stream)) {
    express_cat.error()
      << "Patchfile::find_longest_sequence() - Failed to open file: "
      << infile << endl;
    return 0;
  }

  // Determine file length
  read_stream.seekg(0, ios::end);
  len = read_stream.tellg();
  char *buffer = new char[len];
  read_stream.seekg(0, ios::beg);
  read_stream.read(buffer, len);

  pos = 0;
  char holder = 0;
  int seq_len;
  int longest_seq_len = 0;
  for (int i = 0; i < len; i++) {
    if (buffer[i] != holder) {
      holder = buffer[i];
      seq_len = 0;
    } else {
      if (++seq_len > longest_seq_len) {
	longest_seq_len = seq_len;
	pos = i;
      }
    }
  }

  read_stream.close();

  return longest_seq_len;
}

