// Filename: patchfile.cxx
// Created by:  darren, mike (09Jan97)
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

#include "config_express.h"
#include "error_utils.h"
#include "patchfile.h"
#include "crypto_utils.h" // MD5 stuff
#include "streamReader.h"
#include "streamWriter.h"

#include <stdio.h> // for tempnam

// PROFILING ///////////////////////////////////////////////////////
//#define PROFILE_PATCH_BUILD
#ifdef PROFILE_PATCH_BUILD

#include "clockObject.h"
ClockObject *globalClock = ClockObject::get_global_clock();

#define GET_PROFILE_TIME() globalClock->get_real_time()
#define START_PROFILE(var) double var = GET_PROFILE_TIME()
#define END_PROFILE(startTime, name) \
  cout << name << " took " << (GET_PROFILE_TIME() - (startTime)) << " seconds" << endl

#else
#define START_PROFILE(var)
#define END_PROFILE(startTime, name)
#endif
////////////////////////////////////////////////////////////////////

// this actually slows things down...
//#define USE_MD5_FOR_HASHTABLE_INDEX_VALUES

// Patch File Format ///////////////////////////////////////////////
///// IF THIS CHANGES, UPDATE installerApplyPatch.cxx IN THE INSTALLER
////////////////////////////////////////////////////////////////////
// [ HEADER ]
//   4 bytes  0xfeebfaab ("magic number")
//   4 bytes  length of resulting patched file
//  16 bytes  MD5 of resultant patched file
const int _header_length = sizeof(PN_uint32) + sizeof(PN_uint32) + (4*sizeof(PN_uint32));
//
// [ ADD/COPY pairs; repeated N times ]
//   2 bytes  AL = ADD length
//  AL bytes  bytes to add
//   2 bytes  CL = COPY length
//   4 bytes  offset of data to copy from original file, if CL != 0
//
// [ TERMINATOR ]
//   2 bytes  zero-length ADD
//   2 bytes  zero-length COPY
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
const PN_uint32 Patchfile::_magic_number = 0xfeebfaab;

const PN_uint32 Patchfile::_HASHTABLESIZE = PN_uint32(1) << 16;
const PN_uint32 Patchfile::_DEFAULT_FOOTPRINT_LENGTH = 9; // this produced the smallest patch file for libpanda.dll when tested, 12/20/2000
const PN_uint32 Patchfile::_NULL_VALUE = PN_uint32(0) - 1;
const PN_uint32 Patchfile::_MAX_RUN_LENGTH = (PN_uint32(1) << 16) - 1;

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
  _initiated = false;
  nassertv(!buffer.is_null());
  _buffer = buffer;

  reset_footprint_length();
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patchfile::
~Patchfile(void) {
  if (true == _initiated)
    cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::cleanup
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Patchfile::
cleanup(void) {
  if (false == _initiated) {
    express_cat.error()
      << "Patchfile::cleanup() - Patching has not been initiated"
      << endl;
    return;
  }

  // close files
  _origfile_stream.close();
  _patch_stream.close();
  _write_stream.close();

  _initiated = false;
}

////////////////////////////////////////////////////////////////////
///// PATCH FILE APPLY MEMBER FUNCTIONS
/////
////////////////////
///// NOTE: this patch-application functionality unfortunately has to be
/////       duplicated in the Installer. It is contained in the file
/////       installerApplyPatch.cxx
/////       PLEASE MAKE SURE THAT THAT FILE GETS UPDATED IF ANY OF THIS
/////       LOGIC CHANGES! (i.e. if the patch file format changes)
////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::initiate
//       Access: Public
//  Description: Set up to apply the patch to the file (original
//     file and patch are destroyed in the process).
////////////////////////////////////////////////////////////////////
int Patchfile::
initiate(Filename &patch_file, Filename &file) {
  if (true == _initiated) {
    express_cat.error()
      << "Patchfile::initiate() - Patching has already been initiated"
      << endl;
    return EU_error_abort;
  }

  // Open the patch file for read
  _patch_file = patch_file;
  _patch_file.set_binary();
  if (!_patch_file.open_read(_patch_stream)) {
    express_cat.error()
      << "Patchfile::initiate() - Failed to open file: " << _patch_file << endl;
    return get_write_error();
  }

  // Open the original file for read
  _orig_file = file;
  _orig_file.set_binary();
  if (!_orig_file.open_read(_origfile_stream)) {
    express_cat.error()
      << "Patchfile::initiate() - Failed to open file: " << _orig_file << endl;
    return get_write_error();
  }

  // Open the temp file for write
  {
    char *tempfilename = _tempnam(".", "pf");
    if (NULL == tempfilename) {
      express_cat.error()
        << "Patchfile::initiate() - Failed to create temp file name, using default" << endl;
      _temp_file = "patcher_temp_file";
    } else {
      _temp_file = tempfilename;
      free(tempfilename);
    }
  }

  _temp_file.set_binary();
  if (!_temp_file.open_write(_write_stream)) {
    express_cat.error()
      << "Patchfile::initiate() - Failed to open file: " << _temp_file << endl;
    return get_write_error();
  }

  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Using temporary file " << _temp_file << "\n";
  }

  /////////////
  // read header, make sure the patch file is valid

  StreamReader patch_reader(_patch_stream);

  // check the magic number
  nassertr(_buffer->get_length() >= _header_length, false);
  PN_uint32 magic_number = patch_reader.get_uint32();
  if (magic_number != _magic_number) {
    express_cat.error()
      << "Patchfile::initiate() - invalid patch file: " << _patch_file << endl;
    return EU_error_file_invalid;
  }

  // get the length of the patched result file
  _result_file_length = patch_reader.get_uint32();

  // get the MD5 of the resultant patched file
  _MD5_ofResult.set_value(0, patch_reader.get_uint32());
  _MD5_ofResult.set_value(1, patch_reader.get_uint32());
  _MD5_ofResult.set_value(2, patch_reader.get_uint32());
  _MD5_ofResult.set_value(3, patch_reader.get_uint32());

  express_cat.debug()
    << "Patchfile::initiate() - valid patchfile" << endl;

  _total_bytes_processed = 0;

  _initiated = true;

  return EU_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::run
//       Access: Public
//  Description: Perform one buffer's worth of patching
////////////////////////////////////////////////////////////////////
int Patchfile::
run(void) {
  // Now patch the file using the given buffer
  int buflen;
  int bytes_read;
  PN_uint16 ADD_length;
  PN_uint16 COPY_length;
  PN_uint32 COPY_offset;

  if (_initiated == false) {
    express_cat.error()
      << "Patchfile::run() - Patching has not been initiated"
      << endl;
    return EU_error_abort;
  }

  StreamReader patch_reader(_patch_stream);

  buflen = _buffer->get_length();
  bytes_read = 0;

  while (bytes_read < buflen) {
    ///////////
    // read # of ADD bytes
    nassertr(_buffer->get_length() >= (int)sizeof(ADD_length), false);
    ADD_length = patch_reader.get_uint16();

    bytes_read += (int)ADD_length;
    _total_bytes_processed += (int)ADD_length;

    // if there are bytes to add, read them from patch file and write them to output
    if (0 != ADD_length) {
      PN_uint32 bytes_left = (PN_uint32)ADD_length;

      if (express_cat.is_spam()) {
        express_cat.spam()
          << "ADD: " << ADD_length << endl;
      }

      while (bytes_left > 0) {
        PN_uint32 bytes_this_time = (PN_uint32) min(bytes_left, (PN_uint32) buflen);
        _patch_stream.read(_buffer->_buffer, bytes_this_time);
        _write_stream.write(_buffer->_buffer, bytes_this_time);
        bytes_left -= bytes_this_time;
      }
    }

    ///////////
    // read # of COPY bytes
    nassertr(_buffer->get_length() >= (int)sizeof(COPY_length), false);
    COPY_length = patch_reader.get_uint16();

    bytes_read += (int)COPY_length;
    _total_bytes_processed += (int)COPY_length;

    // if there are bytes to copy, read them from original file and write them to output
    if (0 != COPY_length) {
      // read copy offset
      nassertr(_buffer->get_length() >= (int)sizeof(COPY_offset), false);
      COPY_offset = patch_reader.get_uint32();

      if (express_cat.is_spam()) {
        express_cat.spam()
          << "COPY: " << COPY_length << " bytes at offset " << COPY_offset
          << endl;
      }

      // seek to the offset
      _origfile_stream.seekg(COPY_offset, ios::beg);

      // read the copy bytes from original file and write them to output
      PN_uint32 bytes_left = (PN_uint32)COPY_length;

      while (bytes_left > 0) {
        PN_uint32 bytes_this_time = (PN_uint32) min(bytes_left, (PN_uint32) buflen);
        _origfile_stream.read(_buffer->_buffer, bytes_this_time);
        _write_stream.write(_buffer->_buffer, bytes_this_time);
        bytes_left -= bytes_this_time;
      }
    }

    // if we got a pair of zero-length ADD and COPY blocks, we're done
    if ((0 == ADD_length) && (0 == COPY_length)) {
      cleanup();

      if (express_cat.is_spam()) {
        express_cat.spam()
          << "result file = " << _result_file_length 
          << " total bytes = " << _total_bytes_processed << endl;
      }

      // check the MD5 from the patch file against the newly patched file
      {
        HashVal MD5_actual;
        md5_a_file(_temp_file, MD5_actual);
        if (_MD5_ofResult != MD5_actual) {
          express_cat.info()
            << "Patching produced incorrect checksum.  Got:\n"
            << "    " << MD5_actual
            << "\nExpected:\n"
            << "    " << _MD5_ofResult
            << "\n";

          // delete the temp file and the patch file
          if (!keep_temporary_files) {
            _temp_file.unlink();
            _patch_file.unlink();
          }
          // return "invalid checksum"
          return EU_error_invalid_checksum;
        }
      }

      // delete the patch file and the original file
      if (!keep_temporary_files) {
        _patch_file.unlink();
        _orig_file.unlink();

      } else {
        // If we're keeping temporary files, rename the orig file to a
        // backup.
        Filename orig_backup = _orig_file.get_fullpath() + ".orig";
        orig_backup.unlink();
        _orig_file.rename_to(orig_backup);
      }

      // rename the temp file to the original file name
      if (!_temp_file.rename_to(_orig_file)) {
        express_cat.error()
          << "Patchfile::run() failed to rename temp file to: " << _orig_file
          << endl;
        return EU_error_write_file_rename;
      }

      return EU_success;
    }
  }

  return EU_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::apply
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Patchfile::
apply(Filename &patch_file, Filename &file) {
  int ret = initiate(patch_file, file);
  if (ret < 0)
    return false;
  for (;;) {
    ret = run();
    if (ret == EU_success)
      return true;
    if (ret < 0)
      return false;
  }
  return false;
}

////////////////////////////////////////////////////////////////////
///// PATCH FILE BUILDING MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::calc_hash
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PN_uint16 Patchfile::
calc_hash(const char *buffer) {
#ifdef USE_MD5_FOR_HASHTABLE_INDEX_VALUES
  HashVal hash;

  md5_a_buffer((const unsigned char*)buffer, (int)_footprint_length, hash);

  //cout << PN_uint16(hash.get_value(0)) << " ";

  return PN_uint16(hash.get_value(0));
#else
  PN_uint16 hash_value = 0;

  for(int i = 0; i < (int)_footprint_length; i++) {
    // this is probably not such a good hash. to be replaced
    /// --> TRIED MD5, was not worth it for the execution-time hit on 800Mhz PC
    hash_value ^= (*buffer) << (i % 8);
    buffer++;
  }

  //cout << hash_value << " ";

  return hash_value;
#endif
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::build_hash_link_tables
//       Access: Private
//  Description:
//               The hash and link tables allow for a quick, linear
//               search of all locations in the file that begin with
//               a particular sequence of bytes, or "footprint."
//
//               The hash table is a table of offsets into the file,
//               with one entry for every possible footprint hash
//               value. For a hash of a footprint, the entry at the
//               offset of the hash value provides an initial location
//               in the file that has a matching footprint.
//
//               The link table is a large linked list of file offsets,
//               with one entry for every byte in the file. Each offset
//               in the link table will point to another offset that
//               has the same footprint at the corresponding offset in the
//               actual file. Starting with an offset taken from the hash
//               table, one can rapidly produce a list of offsets that
//               all have the same footprint.
////////////////////////////////////////////////////////////////////
void Patchfile::
build_hash_link_tables(const char *buffer_orig, PN_uint32 length_orig,
  PN_uint32 *hash_table, PN_uint32 *link_table) {

  PN_uint32 i;

  START_PROFILE(clearTables);

  // clear hash table
  for(i = 0; i < _HASHTABLESIZE; i++) {
    hash_table[i] = _NULL_VALUE;
  }

  // clear link table
  for(i = 0; i < length_orig; i++) {
    link_table[i] = _NULL_VALUE;
  }

  END_PROFILE(clearTables, "clearing hash and link tables");

  if(length_orig < _footprint_length) return;

  START_PROFILE(hashingFootprints);
#ifdef PROFILE_PATCH_BUILD
  double hashCalc = 0.0;
  double linkSearch = 0.0;
#endif

  // run through original file and hash each footprint
  for(i = 0; i < (length_orig - _footprint_length); i++) {

#ifdef PROFILE_PATCH_BUILD
    double t = GET_PROFILE_TIME();
#endif

    PN_uint16 hash_value = calc_hash(&buffer_orig[i]);

#ifdef PROFILE_PATCH_BUILD
    hashCalc += GET_PROFILE_TIME() - t;
#endif

    // we must now store this file index in the hash table
    // at the offset of the hash value

    // to account for multiple file offsets with identical
    // hash values, there is a link table with an entry for
    // every footprint in the file. We create linked lists
    // of offsets in the link table.

    // first, set the value in the link table for the current
    // offset to whatever the current list head is (the
    // value in the hash table) (note that this only works
    // because the hash and link tables both use
    // _NULL_VALUE to indicate a null index)
    link_table[i] = hash_table[hash_value];

    // set the new list head; store the current offset in the
    // hash table at the offset of the footprint's hash value
    hash_table[hash_value] = i;

    /*
    if (_NULL_VALUE == hash_table[hash_value]) {
      // hash entry is empty, store this offset
      hash_table[hash_value] = i;
    } else {
      // hash entry is taken, go to the link table
      PN_uint32 link_offset = hash_table[hash_value];

#ifdef PROFILE_PATCH_BUILD
      double t = GET_PROFILE_TIME();
#endif
      while (_NULL_VALUE != link_table[link_offset]) {
        link_offset = link_table[link_offset];
      }
#ifdef PROFILE_PATCH_BUILD
      linkSearch += GET_PROFILE_TIME() - t;
#endif

      link_table[link_offset] = i;
    }
    */
  }

#ifdef PROFILE_PATCH_BUILD
  cout << "calculating footprint hashes took " << hashCalc << " seconds" << endl;
  cout << "traversing the link table took " << linkSearch << " seconds" << endl;
#endif

  END_PROFILE(hashingFootprints, "hashing footprints");
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
find_longest_match(PN_uint32 new_pos, PN_uint32 &copy_offset, PN_uint16 &copy_length,
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
  copy_length = (PN_uint16)calc_match_length(&buffer_new[new_pos], &buffer_orig[copy_offset],
                  min(min((length_new - new_pos),(length_orig - copy_offset)), _MAX_RUN_LENGTH));

  // run through link table, see if we find any longer matches
  PN_uint32 match_offset;
  PN_uint16 match_length;
  match_offset = link_table[copy_offset];

  while (match_offset != _NULL_VALUE) {
    match_length = (PN_uint16)calc_match_length(&buffer_new[new_pos], &buffer_orig[match_offset],
                      min(min((length_new - new_pos),(length_orig - match_offset)), _MAX_RUN_LENGTH));

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
emit_ADD(ofstream &write_stream, PN_uint16 length, const char* buffer) {
  //cout << "ADD: " << length << " bytes" << endl;

  // write ADD length
  StreamWriter patch_writer(write_stream);
  patch_writer.add_uint16(length);

  // if there are bytes to add, add them
  if (length > 0) {
    patch_writer.append_data(buffer, length);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::emit_COPY
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Patchfile::
emit_COPY(ofstream &write_stream, PN_uint16 length, PN_uint32 offset) {
  //cout << "COPY: " << length << " bytes at offset " << offset << endl;

  // write COPY length
  StreamWriter patch_writer(write_stream);
  patch_writer.add_uint16(length);

  if(length > 0) {
    // write COPY offset
    patch_writer.add_uint32(offset);
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

  START_PROFILE(overall);

  START_PROFILE(readFiles);

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

  END_PROFILE(readFiles, "reading files");

  START_PROFILE(allocTables);

  // allocate hash/link tables
  PN_uint32* hash_table = new PN_uint32[_HASHTABLESIZE];
  PN_uint32* link_table = new PN_uint32[length_orig];

  END_PROFILE(allocTables, "allocating hash and link tables");

  START_PROFILE(buildTables);

  // build hash and link tables for original file
  build_hash_link_tables(buffer_orig, length_orig, hash_table, link_table);

  END_PROFILE(buildTables, "building hash and link tables");

  // prepare to write the patch file header

  START_PROFILE(writeHeader);

  // write the patch file header
  StreamWriter patch_writer(write_stream);
  patch_writer.add_uint32(_magic_number);
  patch_writer.add_uint32(length_new);
  {
    // calc MD5 of resultant patched file
    HashVal md5New;
    md5_a_buffer((const unsigned char*)buffer_new, (int)length_new, md5New);
    // add it to the header
    patch_writer.add_uint32(md5New.get_value(0));
    patch_writer.add_uint32(md5New.get_value(1));
    patch_writer.add_uint32(md5New.get_value(2));
    patch_writer.add_uint32(md5New.get_value(3));
  }

  END_PROFILE(writeHeader, "writing patch file header");

  // run through new file
  START_PROFILE(buildPatchfile);

  PN_uint32 new_pos = 0;
  PN_uint32 ADD_offset = new_pos; // this is the offset for the start of ADD operations

  if(((PN_uint32) length_new) >= _footprint_length)
  {
    while (new_pos < (length_new - _footprint_length)) {

      // find best match for current position
      PN_uint32 COPY_offset;
      PN_uint16 COPY_length;

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
        new_pos += (PN_uint32)COPY_length;
        ADD_offset = new_pos;
      }
    }
  }

  // are there still more bytes left in the new file?
  if ((int)ADD_offset != length_new) {
    // emit ADD for all remaining bytes
    emit_ADD(write_stream, length_new - ADD_offset, &buffer_new[ADD_offset]);

    // write null COPY
    emit_COPY(write_stream, 0, 0);
  }

  END_PROFILE(buildPatchfile, "building patch file");

  // write terminator (null ADD, null COPY)
  emit_ADD(write_stream, 0, NULL);
  emit_COPY(write_stream, 0, 0);

  END_PROFILE(overall, "total patch building operation");

  return true;
}

