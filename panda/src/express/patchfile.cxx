// Filename: patchfile.cxx
// Created by:  darren, mike (09Jan97)
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

#include "pandabase.h"

#ifdef HAVE_OPENSSL

#include "config_express.h"
#include "error_utils.h"
#include "patchfile.h"
#include "streamReader.h"
#include "streamWriter.h"
#include "multifile.h"
#include "hashVal.h"
#include "virtualFileSystem.h"

#include <string.h>  // for strstr

#ifdef HAVE_TAR
#include "libtar.h"
#include <fcntl.h>  // for O_RDONLY
#endif  // HAVE_TAR

#ifdef HAVE_TAR
istream *Patchfile::_tar_istream = NULL;
#endif  // HAVE_TAR

////////////////////////////////////////////////////////////////////

// this actually slows things down...
//#define USE_MD5_FOR_HASHTABLE_INDEX_VALUES

// Patch File Format ///////////////////////////////////////////////
///// IF THIS CHANGES, UPDATE installerApplyPatch.cxx IN THE INSTALLER
////////////////////////////////////////////////////////////////////
// [ HEADER ]
//   4 bytes  0xfeebfaac ("magic number")
//            (older patch files have a magic number 0xfeebfaab,
//            indicating they are version number 0.)
//   2 bytes  version number (if magic number == 0xfeebfaac)
//   4 bytes  length of starting file (if version >= 1)
//  16 bytes  MD5 of starting file    (if version >= 1)
//   4 bytes  length of resulting patched file
//  16 bytes  MD5 of resultant patched file

// Note that MD5 hashes are written in the order observed by
// HashVal::read_stream() and HashVal::write_stream(), which is not
// the normal linear order.  (Each group of four bytes is reversed.)

const int _v0_header_length = 4 + 4 + 16;
const int _v1_header_length = 4 + 2 + 4 + 16 + 4 + 16;
//
// [ ADD/COPY pairs; repeated N times ]
//   2 bytes  AL = ADD length
//  AL bytes  bytes to add
//   2 bytes  CL = COPY length
//   4 bytes  offset of data to copy from original file, if CL != 0.
//            If version >= 2, offset is relative to end of previous
//            copy block; if version < 2, offset is relative to
//            beginning of file.
//
// [ TERMINATOR ]
//   2 bytes  zero-length ADD
//   2 bytes  zero-length COPY
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////
const PN_uint32 Patchfile::_v0_magic_number = 0xfeebfaab;
const PN_uint32 Patchfile::_magic_number = 0xfeebfaac;

// Created version 1 on 11/2/02 to store length and MD5 of original file.
// To version 2 on 11/2/02 to store copy offsets as relative.
const PN_uint16 Patchfile::_current_version = 2;

const PN_uint32 Patchfile::_HASH_BITS = 24;
const PN_uint32 Patchfile::_HASHTABLESIZE = PN_uint32(1) << Patchfile::_HASH_BITS;
const PN_uint32 Patchfile::_DEFAULT_FOOTPRINT_LENGTH = 9; // this produced the smallest patch file for libpanda.dll when tested, 12/20/2000
const PN_uint32 Patchfile::_NULL_VALUE = PN_uint32(0) - 1;
const PN_uint32 Patchfile::_MAX_RUN_LENGTH = (PN_uint32(1) << 16) - 1;
const PN_uint32 Patchfile::_HASH_MASK = (PN_uint32(1) << Patchfile::_HASH_BITS) - 1;

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Constructor
//       Access: Public
//  Description: Create a patch file and initializes internal data
////////////////////////////////////////////////////////////////////
Patchfile::
Patchfile() {
  PT(Buffer) buffer = new Buffer(patchfile_buffer_size);
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Constructor
//       Access: Public
//  Description: Create patch file with buffer to patch
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
  _rename_output_to_orig = false;
  _delete_patchfile = false;
  _hash_table = NULL;
  _initiated = false;
  nassertv(!buffer.is_null());
  _buffer = buffer;

  _version_number = 0;
  _allow_multifile = true;

  _patch_stream = NULL;
  _origfile_stream = NULL;

  reset_footprint_length();
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patchfile::
~Patchfile() {
  if (_hash_table != (PN_uint32 *)NULL) {
    PANDA_FREE_ARRAY(_hash_table);
  }

  if (_initiated) {
    cleanup();
  }

  nassertv(_patch_stream == NULL);
  nassertv(_origfile_stream == NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::cleanup
//       Access: Private
//  Description: Closes and clean up internal data structures
////////////////////////////////////////////////////////////////////
void Patchfile::
cleanup() {
  if (!_initiated) {
    express_cat.error()
      << "Patchfile::cleanup() - Patching has not been initiated"
      << endl;
    return;
  }

  // close files
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  if (_origfile_stream != NULL) {
    vfs->close_read_file(_origfile_stream);
    _origfile_stream = NULL;
  }
  if (_patch_stream != NULL) {
    vfs->close_read_file(_patch_stream);
    _patch_stream = NULL;
  }
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
//       Access: Published
//  Description: Set up to apply the patch to the file (original
//               file and patch are destroyed in the process).
////////////////////////////////////////////////////////////////////
int Patchfile::
initiate(const Filename &patch_file, const Filename &file) {
  int result = initiate(patch_file, file, Filename::temporary("", "patch_"));
  _rename_output_to_orig = true;
  _delete_patchfile = !keep_temporary_files;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::initiate
//       Access: Published
//  Description: Set up to apply the patch to the file.  In this form,
//               neither the original file nor the patch file are
//               destroyed.
////////////////////////////////////////////////////////////////////
int Patchfile::
initiate(const Filename &patch_file, const Filename &orig_file,
         const Filename &target_file) {
  if (_initiated) {
    express_cat.error()
      << "Patchfile::initiate() - Patching has already been initiated"
      << endl;
    return EU_error_abort;
  }

  nassertr(orig_file != target_file, EU_error_abort);

  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();

  // Open the original file for read
  nassertr(_origfile_stream == NULL, EU_error_abort);
  _orig_file = orig_file;
  _orig_file.set_binary();
  _origfile_stream = vfs->open_read_file(_orig_file, false);
  if (_origfile_stream == NULL) {
    express_cat.error()
      << "Patchfile::initiate() - Failed to open file: " << _orig_file << endl;
    return get_write_error();
  }

  // Open the temp file for write
  _output_file = target_file;
  _output_file.set_binary();
  if (!_output_file.open_write(_write_stream)) {
    express_cat.error()
      << "Patchfile::initiate() - Failed to open file: " << _output_file << endl;
    return get_write_error();
  }

  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Patchfile using output file " << _output_file << "\n";
  }

  int result = internal_read_header(patch_file);
  _total_bytes_processed = 0;

  _initiated = true;
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::read_header
//       Access: Published
//  Description: Opens the patch file for reading, and gets the header
//               information from the file but does not begin to do
//               any real work.  This can be used to query the data
//               stored in the patch.
////////////////////////////////////////////////////////////////////
int Patchfile::
read_header(const Filename &patch_file) {
  if (_initiated) {
    express_cat.error()
      << "Patchfile::initiate() - Patching has already been initiated"
      << endl;
    return EU_error_abort;
  }

  int result = internal_read_header(patch_file);
  if (_patch_stream != NULL) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    vfs->close_read_file(_patch_stream);
    _patch_stream = NULL;
  }
  return result;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::run
//       Access: Published
//  Description: Perform one buffer's worth of patching
//               Returns EU_ok while patching
//               Returns EU_success when done
//               If error happens will return one of:
//               EU_error_abort : Patching has not been initiated
//               EU_error_file_invalid : file is corrupted
//               EU_error_invalid_checksum : incompatible patch file
//               EU_error_write_file_rename : could not rename file
////////////////////////////////////////////////////////////////////
int Patchfile::
run() {
  // Now patch the file using the given buffer
  int buflen;
  int bytes_read;
  PN_uint16 ADD_length;
  PN_uint16 COPY_length;
  PN_int32 COPY_offset;

  if (_initiated == false) {
    express_cat.error()
      << "Patchfile::run() - Patching has not been initiated"
      << endl;
    return EU_error_abort;
  }

  nassertr(_patch_stream != NULL, EU_error_abort);
  nassertr(_origfile_stream != NULL, EU_error_abort);
  StreamReader patch_reader(*_patch_stream);

  buflen = _buffer->get_length();
  bytes_read = 0;

  while (bytes_read < buflen) {
    ///////////
    // read # of ADD bytes
    nassertr(_buffer->get_length() >= (int)sizeof(ADD_length), false);
    ADD_length = patch_reader.get_uint16();
    if (_patch_stream->fail()) {
      express_cat.error()
        << "Truncated patch file.\n";
      return EU_error_file_invalid;
    }

    bytes_read += (int)ADD_length;
    _total_bytes_processed += (int)ADD_length;
    if (_total_bytes_processed > _total_bytes_to_process) {
      express_cat.error()
        << "Runaway patch file.\n";
      return EU_error_file_invalid;
    }

    // if there are bytes to add, read them from patch file and write them to output
    if (express_cat.is_spam() && ADD_length != 0) {
      express_cat.spam()
        << "ADD: " << ADD_length << " (to "
        << _write_stream.tellp() << ")" << endl;
    }

    PN_uint32 bytes_left = (PN_uint32)ADD_length;
    while (bytes_left > 0) {
      PN_uint32 bytes_this_time = (PN_uint32) min(bytes_left, (PN_uint32) buflen);
      _patch_stream->read(_buffer->_buffer, bytes_this_time);
      if (_patch_stream->fail()) {
        express_cat.error()
          << "Truncated patch file.\n";
        return EU_error_file_invalid;
      }
      _write_stream.write(_buffer->_buffer, bytes_this_time);
      bytes_left -= bytes_this_time;
    }

    ///////////
    // read # of COPY bytes
    nassertr(_buffer->get_length() >= (int)sizeof(COPY_length), false);
    COPY_length = patch_reader.get_uint16();
    if (_patch_stream->fail()) {
      express_cat.error()
        << "Truncated patch file.\n";
      return EU_error_file_invalid;
    }

    bytes_read += (int)COPY_length;
    _total_bytes_processed += (int)COPY_length;
    if (_total_bytes_processed > _total_bytes_to_process) {
      express_cat.error()
        << "Runaway patch file.\n";
      return EU_error_file_invalid;
    }

    // if there are bytes to copy, read them from original file and write them to output
    if (0 != COPY_length) {
      // read copy offset
      nassertr(_buffer->get_length() >= (int)sizeof(COPY_offset), false);
      COPY_offset = patch_reader.get_int32();
      if (_patch_stream->fail()) {
        express_cat.error()
          << "Truncated patch file.\n";
        return EU_error_file_invalid;
      }

      // seek to the copy source pos
      if (_version_number < 2) {
        _origfile_stream->seekg(COPY_offset, ios::beg);
      } else {
        _origfile_stream->seekg(COPY_offset, ios::cur);
      }
      if (_origfile_stream->fail()) {
        express_cat.error()
          << "Invalid copy offset in patch file.\n";
        return EU_error_file_invalid;
      }

      if (express_cat.is_spam()) {
        express_cat.spam()
          << "COPY: " << COPY_length << " bytes from offset "
          << COPY_offset << " (from " << _origfile_stream->tellg()
          << " to " << _write_stream.tellp() << ")"
          << endl;
      }

      // read the copy bytes from original file and write them to output
      PN_uint32 bytes_left = (PN_uint32)COPY_length;

      while (bytes_left > 0) {
        PN_uint32 bytes_this_time = (PN_uint32) min(bytes_left, (PN_uint32) buflen);
        _origfile_stream->read(_buffer->_buffer, bytes_this_time);
        if (_origfile_stream->fail()) {
          express_cat.error()
            << "Invalid copy length in patch file.\n";
          return EU_error_file_invalid;
        }
        _write_stream.write(_buffer->_buffer, bytes_this_time);
        bytes_left -= bytes_this_time;
      }
    }

    // if we got a pair of zero-length ADD and COPY blocks, we're done
    if ((0 == ADD_length) && (0 == COPY_length)) {
      cleanup();

      if (express_cat.is_debug()) {
        express_cat.debug()
          //<< "result file = " << _result_file_length
          << " total bytes = " << _total_bytes_processed << endl;
      }

      // check the MD5 from the patch file against the newly patched file
      {
        HashVal MD5_actual;
        MD5_actual.hash_file(_output_file);
        if (_MD5_ofResult != MD5_actual) {
          // Whoops, patching screwed up somehow.
          if (_origfile_stream != NULL) {
            VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
            vfs->close_read_file(_origfile_stream);
            _origfile_stream = NULL;
          }
          _write_stream.close();

          express_cat.info()
            << "Patching produced incorrect checksum.  Got:\n"
            << "    " << MD5_actual
            << "\nExpected:\n"
            << "    " << _MD5_ofResult
            << "\n";

          // This is a fine time to double-check the starting
          // checksum.
          if (!has_source_hash()) {
            express_cat.info()
              << "No source hash in patch file to verify.\n";
          } else {
            HashVal MD5_orig;
            MD5_orig.hash_file(_orig_file);
            if (MD5_orig != get_source_hash()) {
              express_cat.info()
                << "Started from incorrect source file.  Got:\n"
                << "    " << MD5_orig
                << "\nExpected:\n"
                << "    " << get_source_hash()
                << "\n";
            } else {
              express_cat.info()
                << "Started from correct source file:\n"
                << "    " << MD5_orig
                << "\n";
            }
          }

          // delete the temp file and the patch file
          if (_rename_output_to_orig) {
            _output_file.unlink();
          }
          if (_delete_patchfile) {
            _patch_file.unlink();
          }
          // return "invalid checksum"
          return EU_error_invalid_checksum;
        }
      }

      // delete the patch file
      if (_delete_patchfile) {
        _patch_file.unlink();
      }

      // rename the temp file to the original file name
      if (_rename_output_to_orig) {
        _orig_file.unlink();
        if (!_output_file.rename_to(_orig_file)) {
          express_cat.error()
            << "Patchfile::run() failed to rename temp file to: " << _orig_file
            << endl;
          return EU_error_write_file_rename;
        }
      }

      return EU_success;
    }
  }

  return EU_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::apply
//       Access: Public
//  Description: Patches the entire file in one call
//               returns true on success and false on error
//
//               This version will delete the patch file and overwrite
//               the original file.
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
//     Function: Patchfile::apply
//       Access: Public
//  Description: Patches the entire file in one call
//               returns true on success and false on error
//
//               This version will not delete any files.
////////////////////////////////////////////////////////////////////
bool Patchfile::
apply(Filename &patch_file, Filename &orig_file, const Filename &target_file) {
  int ret = initiate(patch_file, orig_file, target_file);
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
//     Function: Patchfile::internal_read_header
//       Access: Private
//  Description: Reads the header and leaves the patch file open.
////////////////////////////////////////////////////////////////////
int Patchfile::
internal_read_header(const Filename &patch_file) {
  // Open the patch file for read
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  nassertr(_patch_stream == NULL, EU_error_abort);
  _patch_file = patch_file;
  _patch_file.set_binary();
  _patch_stream = vfs->open_read_file(_patch_file, true);
  if (_patch_stream == NULL) {
    express_cat.error()
      << "Patchfile::initiate() - Failed to open file: " << _patch_file << endl;
    return get_write_error();
  }

  /////////////
  // read header, make sure the patch file is valid

  StreamReader patch_reader(*_patch_stream);

  // check the magic number
  nassertr(_buffer->get_length() >= _v0_header_length, false);
  PN_uint32 magic_number = patch_reader.get_uint32();
  if (magic_number != _magic_number && magic_number != _v0_magic_number) {
    express_cat.error()
      << "Invalid patch file: " << _patch_file << endl;
    return EU_error_file_invalid;
  }

  _version_number = 0;
  if (magic_number != _v0_magic_number) {
    _version_number = patch_reader.get_uint16();
  }
  if (_version_number > _current_version) {
    express_cat.error()
      << "Can't read version " << _version_number << " patch files: "
      << _patch_file << endl;
    return EU_error_file_invalid;
  }

  if (_version_number >= 1) {
    // Get the length of the source file.
    /*PN_uint32 source_file_length =*/ patch_reader.get_uint32();

    // get the MD5 of the source file.
    _MD5_ofSource.read_stream(patch_reader);
  }

  // get the length of the patched result file
  _total_bytes_to_process = patch_reader.get_uint32();

  // get the MD5 of the resultant patched file
  _MD5_ofResult.read_stream(patch_reader);

  express_cat.debug()
    << "Patchfile::initiate() - valid patchfile" << endl;

  return EU_success;
}

////////////////////////////////////////////////////////////////////
///// PATCH FILE BUILDING MEMBER FUNCTIONS
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::calc_hash
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
PN_uint32 Patchfile::
calc_hash(const char *buffer) {
#ifdef USE_MD5_FOR_HASHTABLE_INDEX_VALUES
  HashVal hash;
  hash.hash_buffer(buffer, _footprint_length);

  //cout << PN_uint16(hash.get_value(0)) << " ";

  return PN_uint16(hash.get_value(0));
#else
  PN_uint32 hash_value = 0;

  for(int i = 0; i < (int)_footprint_length; i++) {
    // this is probably not such a good hash. to be replaced
    /// --> TRIED MD5, was not worth it for the execution-time hit on 800Mhz PC
    hash_value ^= PN_uint32(*buffer) << ((i * 2) % Patchfile::_HASH_BITS);
    buffer++;
  }

  // use the bits that overflowed past the end of the hash bit range
  // (this is intended for _HASH_BITS == 24)
  hash_value ^= (hash_value >> Patchfile::_HASH_BITS);

  //cout << hash_value << " ";

  return hash_value & _HASH_MASK;
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

  // clear hash table
  for(i = 0; i < _HASHTABLESIZE; i++) {
    hash_table[i] = _NULL_VALUE;
  }

  // clear link table
  for(i = 0; i < length_orig; i++) {
    link_table[i] = _NULL_VALUE;
  }

  if(length_orig < _footprint_length) return;

  // run through original file and hash each footprint
  for(i = 0; i < (length_orig - _footprint_length); i++) {

    PN_uint32 hash_value = calc_hash(&buffer_orig[i]);

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

      while (_NULL_VALUE != link_table[link_offset]) {
        link_offset = link_table[link_offset];
      }
      link_table[link_offset] = i;
    }
    */
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
calc_match_length(const char* buf1, const char* buf2, PN_uint32 max_length,
                  PN_uint32 min_length) {
  // early out: look ahead and sample the end of the minimum range
  if (min_length > 2) {
    if (min_length >= max_length)
      return 0;
    if (buf1[min_length] != buf2[min_length] ||
        buf1[min_length-1] != buf2[min_length-1] ||
        buf1[min_length-2] != buf2[min_length-2]) {
      return 0;
    }
  }

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
find_longest_match(PN_uint32 new_pos, PN_uint32 &copy_pos, PN_uint16 &copy_length,
  PN_uint32 *hash_table, PN_uint32 *link_table, const char* buffer_orig,
  PN_uint32 length_orig, const char* buffer_new, PN_uint32 length_new) {

  // set length to a safe value
  copy_length = 0;

  // get offset of matching string (in orig file) from hash table
  PN_uint32 hash_value = calc_hash(&buffer_new[new_pos]);

  // if no match, bail
  if (_NULL_VALUE == hash_table[hash_value])
    return;

  copy_pos = hash_table[hash_value];

  // calc match length
  copy_length = (PN_uint16)calc_match_length(&buffer_new[new_pos],
                                             &buffer_orig[copy_pos],
                                             min(min((length_new - new_pos),
                                                     (length_orig - copy_pos)),
                                                 _MAX_RUN_LENGTH),
                                             0);

  // run through link table, see if we find any longer matches
  PN_uint32 match_offset;
  PN_uint16 match_length;
  match_offset = link_table[copy_pos];

  while (match_offset != _NULL_VALUE) {
    match_length = (PN_uint16)calc_match_length(&buffer_new[new_pos],
                                                &buffer_orig[match_offset],
                                                min(min((length_new - new_pos),
                                                        (length_orig - match_offset)),
                                                    _MAX_RUN_LENGTH),
                                                copy_length);

    // have we found a longer match?
    if (match_length > copy_length) {
      copy_pos = match_offset;
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
emit_ADD(ostream &write_stream, PN_uint32 length, const char* buffer) {
  nassertv(length == (PN_uint16)length); //we only write a uint16

  if (express_cat.is_spam()) {
    express_cat.spam()
      << "ADD: " << length << " (to " << _add_pos << ")" << endl;
  }

  // write ADD length
  StreamWriter patch_writer(write_stream);
  patch_writer.add_uint16((PN_uint16)length);

  // if there are bytes to add, add them
  if (length > 0) {
    patch_writer.append_data(buffer, (PN_uint16)length);
  }

  _add_pos += length;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::emit_COPY
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Patchfile::
emit_COPY(ostream &write_stream, PN_uint32 length, PN_uint32 copy_pos) {
  nassertv(length == (PN_uint16)length); //we only write a uint16

  PN_int32 offset = (int)copy_pos - (int)_last_copy_pos;
  if (express_cat.is_spam()) {
    express_cat.spam()
      << "COPY: " << length << " bytes from offset " << offset
      << " (from " << copy_pos << " to " << _add_pos << ")" << endl;
  }

  // write COPY length
  StreamWriter patch_writer(write_stream);
  patch_writer.add_uint16((PN_uint16)length);

  if ((PN_uint16)length != 0) {
    // write COPY offset
    patch_writer.add_int32(offset);
    _last_copy_pos = copy_pos + length;
  }

  _add_pos += length;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::emit_add_and_copy
//       Access: Private
//  Description: Emits an add/copy pair.  If necessary, repeats the
//               pair as needed to work around the 16-bit chunk size
//               limit.
////////////////////////////////////////////////////////////////////
void Patchfile::
emit_add_and_copy(ostream &write_stream,
                  PN_uint32 add_length, const char *add_buffer,
                  PN_uint32 copy_length, PN_uint32 copy_pos) {
  if (add_length == 0 && copy_length == 0) {
    // Don't accidentally emit a termination code.
    return;
  }

  static const PN_uint16 max_write = 65535;
  while (add_length > max_write) {
    // Overflow.  This chunk is too large to fit into a single
    // ADD block, so we have to write it as multiple ADDs.
    emit_ADD(write_stream, max_write, add_buffer);
    add_buffer += max_write;
    add_length -= max_write;
    emit_COPY(write_stream, 0, 0);
  }

  emit_ADD(write_stream, add_length, add_buffer);

  while (copy_length > max_write) {
    // Overflow.
    emit_COPY(write_stream, max_write, copy_pos);
    copy_pos += max_write;
    copy_length -= max_write;
    emit_ADD(write_stream, 0, NULL);
  }

  emit_COPY(write_stream, copy_length, copy_pos);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::cache_add_and_copy
//       Access: Private
//  Description: Potentially emits one or more add/copy pairs.  The
//               current state is saved, so as to minimize wasted
//               emits from consecutive adds or copies.
////////////////////////////////////////////////////////////////////
void Patchfile::
cache_add_and_copy(ostream &write_stream,
                   PN_uint32 add_length, const char *add_buffer,
                   PN_uint32 copy_length, PN_uint32 copy_pos) {
  if (add_length != 0) {
    if (_cache_copy_length != 0) {
      // Have to flush.
      cache_flush(write_stream);
    }
    // Add the string to the current cache.
    _cache_add_data += string(add_buffer, add_length);
  }

  if (copy_length != 0) {
    if (_cache_copy_length == 0) {
      // Start a new copy phase.
      _cache_copy_start = copy_pos;
      _cache_copy_length = copy_length;

    } else if (_cache_copy_start + _cache_copy_length == copy_pos) {
      // We can just tack on the copy to what we've already got.
      _cache_copy_length += copy_length;

    } else {
      // It's a discontinuous copy.  We have to flush.
      cache_flush(write_stream);
      _cache_copy_start = copy_pos;
      _cache_copy_length = copy_length;
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::cache_flush
//       Access: Private
//  Description: Closes any copy or add phases that are still open
//               after a previous call to cache_add_and_copy().
////////////////////////////////////////////////////////////////////
void Patchfile::
cache_flush(ostream &write_stream) {
  emit_add_and_copy(write_stream,
                    _cache_add_data.size(), _cache_add_data.data(),
                    _cache_copy_length, _cache_copy_start);
  _cache_add_data = string();
  _cache_copy_length = 0;
}


////////////////////////////////////////////////////////////////////
//     Function: Patchfile::write_header
//       Access: Private
//  Description:
//               Writes the patchfile header.
////////////////////////////////////////////////////////////////////
void Patchfile::
write_header(ostream &write_stream,
             istream &stream_orig, istream &stream_new) {
  // prepare to write the patch file header

  // write the patch file header
  StreamWriter patch_writer(write_stream);
  patch_writer.add_uint32(_magic_number);
  patch_writer.add_uint16(_current_version);

  stream_orig.seekg(0, ios::end);
  streampos source_file_length = stream_orig.tellg();
  patch_writer.add_uint32((PN_uint32)source_file_length);

  // calc MD5 of original file
  _MD5_ofSource.hash_stream(stream_orig);
  // add it to the header
  _MD5_ofSource.write_stream(patch_writer);

  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Orig: " << _MD5_ofSource << "\n";
  }

  stream_new.seekg(0, ios::end);
  streampos result_file_length = stream_new.tellg();
  patch_writer.add_uint32((PN_uint32)result_file_length);

  // calc MD5 of resultant patched file
  _MD5_ofResult.hash_stream(stream_new);
  // add it to the header
  _MD5_ofResult.write_stream(patch_writer);

  if (express_cat.is_debug()) {
    express_cat.debug()
      << " New: " << _MD5_ofResult << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::write_terminator
//       Access: Private
//  Description: Writes the patchfile terminator.
////////////////////////////////////////////////////////////////////
void Patchfile::
write_terminator(ostream &write_stream) {
  cache_flush(write_stream);
  // write terminator (null ADD, null COPY)
  emit_ADD(write_stream, 0, NULL);
  emit_COPY(write_stream, 0, 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::compute_file_patches
//       Access: Private
//  Description: Computes the patches for the entire file (if it is
//               not a multifile) or for a single subfile (if it is)
//
//               Returns true if successful, false on error.
////////////////////////////////////////////////////////////////////
bool Patchfile::
compute_file_patches(ostream &write_stream,
                     PN_uint32 offset_orig, PN_uint32 offset_new,
                     istream &stream_orig, istream &stream_new) {
  // read in original file
  stream_orig.seekg(0, ios::end);
  nassertr(stream_orig, false);
  PN_uint32 source_file_length = stream_orig.tellg();
  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Allocating " << source_file_length << " bytes to read orig\n";
  }

  char *buffer_orig = (char *)PANDA_MALLOC_ARRAY(source_file_length);
  stream_orig.seekg(0, ios::beg);
  stream_orig.read(buffer_orig, source_file_length);

  // read in new file
  stream_new.seekg(0, ios::end);
  PN_uint32 result_file_length = stream_new.tellg();
  nassertr(stream_new, false);
  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Allocating " << result_file_length << " bytes to read new\n";
  }

  char *buffer_new = (char *)PANDA_MALLOC_ARRAY(result_file_length);
  stream_new.seekg(0, ios::beg);
  stream_new.read(buffer_new, result_file_length);

  // allocate hash/link tables
  if (_hash_table == (PN_uint32 *)NULL) {
    if (express_cat.is_debug()) {
      express_cat.debug()
        << "Allocating hashtable of size " << _HASHTABLESIZE << " * 4\n";
    }
    _hash_table = (PN_uint32 *)PANDA_MALLOC_ARRAY(_HASHTABLESIZE * sizeof(PN_uint32));
  }

  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Allocating linktable of size " << source_file_length << " * 4\n";
  }

  PN_uint32 *link_table = (PN_uint32 *)PANDA_MALLOC_ARRAY(source_file_length * sizeof(PN_uint32));

  // build hash and link tables for original file
  build_hash_link_tables(buffer_orig, source_file_length, _hash_table, link_table);

  // run through new file

  PN_uint32 new_pos = 0;
  PN_uint32 start_pos = new_pos; // this is the position for the start of ADD operations

  if(((PN_uint32) result_file_length) >= _footprint_length)
  {
    while (new_pos < (result_file_length - _footprint_length)) {

      // find best match for current position
      PN_uint32 COPY_pos;
      PN_uint16 COPY_length;

      find_longest_match(new_pos, COPY_pos, COPY_length, _hash_table, link_table,
        buffer_orig, source_file_length, buffer_new, result_file_length);

      // if no match or match not longer than footprint length, skip to next byte
      if (COPY_length < _footprint_length) {
        // go to next byte
        new_pos++;
      } else {
        // emit ADD for all skipped bytes
        int num_skipped = (int)new_pos - (int)start_pos;
        if (express_cat.is_spam()) {
          express_cat.spam()
            << "build: num_skipped = " << num_skipped
            << endl;
        }
        cache_add_and_copy(write_stream, num_skipped, &buffer_new[start_pos],
                           COPY_length, COPY_pos + offset_orig);
        new_pos += (PN_uint32)COPY_length;
        start_pos = new_pos;
      }
    }
  }

  if (express_cat.is_spam()) {
    express_cat.spam()
      << "build: result_file_length = " << result_file_length
      << " start_pos = " << start_pos
      << endl;
  }

  // are there still more bytes left in the new file?
  if (start_pos != result_file_length) {
    // emit ADD for all remaining bytes

    PN_uint32 remaining_bytes = result_file_length - start_pos;
    cache_add_and_copy(write_stream, remaining_bytes, &buffer_new[start_pos],
                       0, 0);
    start_pos += remaining_bytes;
  }

  PANDA_FREE_ARRAY(link_table);

  PANDA_FREE_ARRAY(buffer_orig);
  PANDA_FREE_ARRAY(buffer_new);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::compute_mf_patches
//       Access: Private
//  Description: Computes patches for the files, knowing that they are
//               both Panda Multifiles.  This will build patches one
//               subfile at a time, which can potentially be much,
//               much faster for large Multifiles that contain many
//               small subfiles.
////////////////////////////////////////////////////////////////////
bool Patchfile::
compute_mf_patches(ostream &write_stream,
                   PN_uint32 offset_orig, PN_uint32 offset_new,
                   istream &stream_orig, istream &stream_new) {
  Multifile mf_orig, mf_new;
  IStreamWrapper stream_origw(stream_orig);
  IStreamWrapper stream_neww(stream_new);
  if (!mf_orig.open_read(&stream_origw) ||
      !mf_new.open_read(&stream_neww)) {
    express_cat.error()
      << "Input multifiles appear to be corrupt.\n";
    return false;
  }

  if (mf_new.needs_repack()) {
    express_cat.error()
      << "Input multifiles need to be repacked.\n";
    return false;
  }

  // First, compute the patch for the header / index.

  {
    ISubStream index_orig(&stream_origw, 0, mf_orig.get_index_end());
    ISubStream index_new(&stream_neww, 0, mf_new.get_index_end());
    if (!do_compute_patches("", "",
                            write_stream, offset_orig, offset_new,
                            index_orig, index_new)) {
      return false;
    }
    nassertr(_add_pos + _cache_add_data.size() + _cache_copy_length == offset_new + mf_new.get_index_end(), false);
  }

  // Now walk through each subfile in the new multifile.  If a
  // particular subfile exists in both source files, we compute the
  // patches for the subfile; for a new subfile, we trivially add it.
  // If a subfile has been removed, we simply don't add it (we'll
  // never even notice this case).
  int new_num_subfiles = mf_new.get_num_subfiles();
  for (int ni = 0; ni < new_num_subfiles; ++ni) {
    nassertr(_add_pos + _cache_add_data.size() + _cache_copy_length == offset_new + mf_new.get_subfile_internal_start(ni), false);
    string name = mf_new.get_subfile_name(ni);
    int oi = mf_orig.find_subfile(name);

    if (oi < 0) {
      // This is a newly-added subfile.  Add it the hard way.
      express_cat.info()
        << "Adding subfile " << mf_new.get_subfile_name(ni) << "\n";

      streampos new_start = mf_new.get_subfile_internal_start(ni);
      size_t new_size = mf_new.get_subfile_internal_length(ni);
      char *buffer_new = (char *)PANDA_MALLOC_ARRAY(new_size);
      stream_new.seekg(new_start, ios::beg);
      stream_new.read(buffer_new, new_size);
      cache_add_and_copy(write_stream, new_size, buffer_new, 0, 0);
      PANDA_FREE_ARRAY(buffer_new);

    } else {
      // This subfile exists in both the original and the new files.
      // Patch it.
      streampos orig_start = mf_orig.get_subfile_internal_start(oi);
      size_t orig_size = mf_orig.get_subfile_internal_length(oi);

      streampos new_start = mf_new.get_subfile_internal_start(ni);
      size_t new_size = mf_new.get_subfile_internal_length(ni);

      if (!patch_subfile(write_stream, offset_orig, offset_new,
                         mf_new.get_subfile_name(ni),
                         stream_origw, orig_start, orig_start + (streampos)orig_size,
                         stream_neww, new_start, new_start + (streampos)new_size)) {
        return false;
      }
    }
  }

  return true;
}

#ifdef HAVE_TAR
////////////////////////////////////////////////////////////////////
//     Function: Patchfile::read_tar
//       Access: Private
//  Description: Uses libtar to extract the location within the tar
//               file of each of the subfiles.  Returns true if the
//               tar file is read successfully, false if there is an
//               error (e.g. it is not a tar file).
////////////////////////////////////////////////////////////////////
bool Patchfile::
read_tar(TarDef &tar, istream &stream) {
  TAR *tfile;
  tartype_t tt;
  tt.openfunc = tar_openfunc;
  tt.closefunc = tar_closefunc;
  tt.readfunc = tar_readfunc;
  tt.writefunc = tar_writefunc;

  stream.seekg(0, ios::beg);
  nassertr(_tar_istream == NULL, false);
  _tar_istream = &stream;
  if (tar_open(&tfile, (char *)"dummy", &tt, O_RDONLY, 0, 0) != 0) {
    _tar_istream = NULL;
    return false;
  }

  // Walk through the tar file, noting the current file position as we
  // reach each subfile.  Use this information to infer the start and
  // end of each subfile within the stream.

  streampos last_pos = 0;
  int flag = th_read(tfile);
  while (flag == 0) {
    TarSubfile subfile;
    subfile._name = th_get_pathname(tfile);
    subfile._header_start = last_pos;
    subfile._data_start = stream.tellg();
    subfile._data_end = subfile._data_start + (streampos)th_get_size(tfile);
    tar_skip_regfile(tfile);
    subfile._end = stream.tellg();
    tar.push_back(subfile);

    last_pos = subfile._end;
    flag = th_read(tfile);
  }

  // Create one more "subfile" for the bytes at the tail of the file.
  // This subfile has no name.
  TarSubfile subfile;
  subfile._header_start = last_pos;
  stream.clear();
  stream.seekg(0, ios::end);
  subfile._data_start = stream.tellg();
  subfile._data_end = subfile._data_start;
  subfile._end = subfile._data_start;
  tar.push_back(subfile);

  tar_close(tfile);
  _tar_istream = NULL;
  return (flag == 1);
}
#endif  // HAVE_TAR

#ifdef HAVE_TAR
////////////////////////////////////////////////////////////////////
//     Function: Patchfile::compute_tar_patches
//       Access: Private
//  Description: Computes patches for the files, knowing that they are
//               both tar files.  This is similar to
//               compute_mf_patches().
//
//               The tar indexes should have been built up by a
//               previous call to read_tar().
////////////////////////////////////////////////////////////////////
bool Patchfile::
compute_tar_patches(ostream &write_stream,
                    PN_uint32 offset_orig, PN_uint32 offset_new,
                    istream &stream_orig, istream &stream_new,
                    TarDef &tar_orig, TarDef &tar_new) {

  // Sort the orig list by filename, so we can quickly look up files
  // from the new list.
  tar_orig.sort();

  // However, it is important to keep the new list in its original,
  // on-disk order.

  // Walk through each subfile in the new tar file.  If a particular
  // subfile exists in both source files, we compute the patches for
  // the subfile; for a new subfile, we trivially add it.  If a
  // subfile has been removed, we simply don't add it (we'll never
  // even notice this case).

  IStreamWrapper stream_origw(stream_orig);
  IStreamWrapper stream_neww(stream_new);

  TarDef::const_iterator ni;
  streampos last_pos = 0;
  for (ni = tar_new.begin(); ni != tar_new.end(); ++ni) {
    const TarSubfile &sf_new =(*ni);
    nassertr(sf_new._header_start == last_pos, false);

    TarDef::const_iterator oi = tar_orig.find(sf_new);

    if (oi == tar_orig.end()) {
      // This is a newly-added subfile.  Add it the hard way.
      express_cat.info()
        << "Adding subfile " << sf_new._name << "\n";

      streampos new_start = sf_new._header_start;
      size_t new_size = sf_new._end - sf_new._header_start;
      char *buffer_new = (char *)PANDA_MALLOC_ARRAY(new_size);
      stream_new.seekg(new_start, ios::beg);
      stream_new.read(buffer_new, new_size);
      cache_add_and_copy(write_stream, new_size, buffer_new, 0, 0);
      PANDA_FREE_ARRAY(buffer_new);

    } else {
      // This subfile exists in both the original and the new files.
      // Patch it.
      const TarSubfile &sf_orig =(*oi);

      // We patch the header and data of the file separately, so we
      // can accurately detect nested multifiles.  The extra data at
      // the end of the file (possibly introduced by a tar file's
      // blocking) is the footer, which is also patched separately.
      if (!patch_subfile(write_stream, offset_orig, offset_new, "",
                         stream_origw, sf_orig._header_start, sf_orig._data_start,
                         stream_neww, sf_new._header_start, sf_new._data_start)) {
        return false;
      }

      if (!patch_subfile(write_stream, offset_orig, offset_new, sf_new._name,
                         stream_origw, sf_orig._data_start, sf_orig._data_end,
                         stream_neww, sf_new._data_start, sf_new._data_end)) {
        return false;
      }

      if (!patch_subfile(write_stream, offset_orig, offset_new, "",
                         stream_origw, sf_orig._data_end, sf_orig._end,
                         stream_neww, sf_new._data_end, sf_new._end)) {
        return false;
      }
    }

    last_pos = sf_new._end;
  }

  return true;
}
#endif  // HAVE_TAR

#ifdef HAVE_TAR
////////////////////////////////////////////////////////////////////
//     Function: Patchfile::tar_openfunc
//       Access: Private, Static
//  Description: A callback function to redirect libtar to read from
//               our istream instead of using low-level Unix I/O.
////////////////////////////////////////////////////////////////////
int Patchfile::
tar_openfunc(const char *, int, ...) {
  // Since we don't actually open a file--the stream is already
  // open--we do nothing here.
  return 0;
}
#endif  // HAVE_TAR

#ifdef HAVE_TAR
////////////////////////////////////////////////////////////////////
//     Function: Patchfile::tar_closefunc
//       Access: Private, Static
//  Description: A callback function to redirect libtar to read from
//               our istream instead of using low-level Unix I/O.
////////////////////////////////////////////////////////////////////
int Patchfile::
tar_closefunc(int) {
  // Since we don't actually open a file, no need to close it either.
  return 0;
}
#endif  // HAVE_TAR

#ifdef HAVE_TAR
////////////////////////////////////////////////////////////////////
//     Function: Patchfile::tar_readfunc
//       Access: Private, Static
//  Description: A callback function to redirect libtar to read from
//               our istream instead of using low-level Unix I/O.
////////////////////////////////////////////////////////////////////
ssize_t Patchfile::
tar_readfunc(int, void *buffer, size_t nbytes) {
  nassertr(_tar_istream != NULL, 0);
  _tar_istream->read((char *)buffer, nbytes);
  return (ssize_t)_tar_istream->gcount();
}
#endif  // HAVE_TAR

#ifdef HAVE_TAR
////////////////////////////////////////////////////////////////////
//     Function: Patchfile::tar_writefunc
//       Access: Private, Static
//  Description: A callback function to redirect libtar to read from
//               our istream instead of using low-level Unix I/O.
////////////////////////////////////////////////////////////////////
ssize_t Patchfile::
tar_writefunc(int, const void *, size_t) {
  // Since we use libtar only for reading, it is an error if this
  // method gets called.
  nassertr(false, -1);
  return -1;
}
#endif  // HAVE_TAR

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::build
//       Access: Public
//  Description:
//               This implementation uses the "greedy differencing
//               algorithm" described in the masters thesis
//               "Differential Compression: A Generalized Solution
//               for Binary Files" by Randal C. Burns (p.13).
//               For an original file of size M and a new file of
//               size N, this algorithm is O(M) in space and
//               O(M*N) (worst-case) in time.
//               return false on error
////////////////////////////////////////////////////////////////////
bool Patchfile::
build(Filename file_orig, Filename file_new, Filename patch_name) {
  patch_name.set_binary();

  // Open the original file for read
  pifstream stream_orig;
  file_orig.set_binary();
  if (!file_orig.open_read(stream_orig)) {
    express_cat.error()
      << "Patchfile::build() - Failed to open file: " << file_orig << endl;
    return false;
  }

  // Open the new file for read
  pifstream stream_new;
  file_new.set_binary();
  if (!file_new.open_read(stream_new)) {
    express_cat.error()
      << "Patchfile::build() - Failed to open file: " << file_new << endl;
    return false;
  }

  // Open patch file for write
  pofstream write_stream;
  if (!patch_name.open_write(write_stream)) {
    express_cat.error()
      << "Patchfile::build() - Failed to open file: " << patch_name << endl;
    return false;
  }

  _last_copy_pos = 0;
  _add_pos = 0;
  _cache_add_data = string();
  _cache_copy_start = 0;
  _cache_copy_length = 0;

  write_header(write_stream, stream_orig, stream_new);

  if (!do_compute_patches(file_orig, file_new,
                          write_stream, 0, 0,
                          stream_orig, stream_new)) {
    return false;
  }

  write_terminator(write_stream);

  if (express_cat.is_debug()) {
    express_cat.debug()
      << "Patch file will generate " << _add_pos << "-byte file.\n";
  }

#ifndef NDEBUG
 {
   // Make sure the resulting file would be the right size.
   stream_new.seekg(0, ios::end);
   streampos result_file_length = stream_new.tellg();
   nassertr(_add_pos == result_file_length, false);
 }
#endif  // NDEBUG

  return (_last_copy_pos != 0);
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::do_compute_patches
//       Access: Private
//  Description: Computes the patches for the indicated A to B files,
//               or subfiles.  Checks for multifiles or tar files
//               before falling back to whole-file patching.
////////////////////////////////////////////////////////////////////
bool Patchfile::
do_compute_patches(const Filename &file_orig, const Filename &file_new,
                   ostream &write_stream,
                   PN_uint32 offset_orig, PN_uint32 offset_new,
                   istream &stream_orig, istream &stream_new) {
  nassertr(_add_pos + _cache_add_data.size() + _cache_copy_length == offset_new, false);

  // Check whether our input files are Panda multifiles or tar files.
  bool is_multifile = false;
#ifdef HAVE_TAR
  bool is_tarfile = false;
  TarDef tar_orig, tar_new;
#endif  // HAVE_TAR

  if (_allow_multifile) {
    if (strstr(file_orig.get_basename().c_str(), ".mf") != NULL ||
        strstr(file_new.get_basename().c_str(), ".mf") != NULL) {
      // Read the first n bytes of both files for the Multifile magic
      // number.
      string magic_number = Multifile::get_magic_number();
      char *buffer = (char *)PANDA_MALLOC_ARRAY(magic_number.size());
      stream_orig.seekg(0, ios::beg);
      stream_orig.read(buffer, magic_number.size());

      if (stream_orig.gcount() == (int)magic_number.size() &&
          memcmp(buffer, magic_number.data(), magic_number.size()) == 0) {
        stream_new.seekg(0, ios::beg);
        stream_new.read(buffer, magic_number.size());
        if (stream_new.gcount() == (int)magic_number.size() &&
            memcmp(buffer, magic_number.data(), magic_number.size()) == 0) {
          is_multifile = true;
        }
      }
      PANDA_FREE_ARRAY(buffer);
    }
#ifdef HAVE_TAR
    if (strstr(file_orig.get_basename().c_str(), ".tar") != NULL ||
        strstr(file_new.get_basename().c_str(), ".tar") != NULL) {
      if (read_tar(tar_orig, stream_orig) &&
          read_tar(tar_new, stream_new)) {
        is_tarfile = true;
      }
    }
#endif  // HAVE_TAR
  }

  if (is_multifile) {
    if (express_cat.is_debug()) {
      express_cat.debug()
        << file_orig.get_basename() << " appears to be a Panda Multifile.\n";
    }
    if (!compute_mf_patches(write_stream, offset_orig, offset_new,
                            stream_orig, stream_new)) {
      return false;
    }
#ifdef HAVE_TAR
  } else if (is_tarfile) {
    if (express_cat.is_debug()) {
      express_cat.debug()
        << file_orig.get_basename() << " appears to be a tar file.\n";
    }
    if (!compute_tar_patches(write_stream, offset_orig, offset_new,
                             stream_orig, stream_new, tar_orig, tar_new)) {
      return false;
    }
#endif  // HAVE_TAR
  } else {
    if (express_cat.is_debug()) {
      express_cat.debug()
        << file_orig.get_basename() << " is not a multifile.\n";
    }
    if (!compute_file_patches(write_stream, offset_orig, offset_new,
                              stream_orig, stream_new)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Patchfile::patch_subfile
//       Access: Private
//  Description: Generates patches for a nested subfile of a Panda
//               Multifile or a tar file.
////////////////////////////////////////////////////////////////////
bool Patchfile::
patch_subfile(ostream &write_stream,
              PN_uint32 offset_orig, PN_uint32 offset_new,
              const Filename &filename,
              IStreamWrapper &stream_orig, streampos orig_start, streampos orig_end,
              IStreamWrapper &stream_new, streampos new_start, streampos new_end) {
  nassertr(_add_pos + _cache_add_data.size() + _cache_copy_length == offset_new + new_start, false);

  size_t new_size = new_end - new_start;
  size_t orig_size = orig_end - orig_start;

  ISubStream subfile_orig(&stream_orig, orig_start, orig_end);
  ISubStream subfile_new(&stream_new, new_start, new_end);

  bool is_unchanged = false;
  if (orig_size == new_size) {
    HashVal hash_orig, hash_new;
    hash_orig.hash_stream(subfile_orig);
    hash_new.hash_stream(subfile_new);

    if (hash_orig == hash_new) {
      // Actually, the subfile is unchanged; just emit it.
      is_unchanged = true;
    }
  }

  if (is_unchanged) {
    if (express_cat.is_debug() && !filename.empty()) {
      express_cat.debug()
        << "Keeping subfile " << filename << "\n";
    }
    cache_add_and_copy(write_stream, 0, NULL,
                       orig_size, offset_orig + orig_start);

  } else {
    if (!filename.empty()) {
      express_cat.info()
        << "Patching subfile " << filename << "\n";
    }

    if (!do_compute_patches(filename, filename, write_stream,
                            offset_orig + orig_start, offset_new + new_start,
                            subfile_orig, subfile_new)) {
      return false;
    }
  }

  return true;
}

#endif // HAVE_OPENSSL
