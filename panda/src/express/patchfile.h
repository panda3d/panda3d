// Filename: patchfile.h
// Created by:  darren, mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////
#ifndef PATCHFILE_H
#define PATCHFILE_H

#include "pandabase.h"

#ifdef HAVE_OPENSSL

#include "typedef.h"
#include "pnotify.h"
#include "filename.h"
#include "plist.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "buffer.h"
#include "pointerTo.h"
#include "hashVal.h" // MD5 stuff

#include <algorithm>


////////////////////////////////////////////////////////////////////
//       Class : Patchfile
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Patchfile {
PUBLISHED:
  /*
  enum PatchfileStatus {
    PF_ok = 2,
    PF_success = 1,
    PF_error = -1,
    PF_error_filenotfound = -2,
    PF_error_invalidpatchfile = -3,
    PF_error_wrongpatchfile = -4,
    PF_error_renamingtempfile = -5,
  };
  */

  Patchfile();
  Patchfile(PT(Buffer) buffer);
  ~Patchfile();

  bool build(Filename file_orig, Filename file_new, Filename patch_name);
  int read_header(const Filename &patch_file);

  int initiate(const Filename &patch_file, const Filename &file);
  int run();

  bool apply(Filename &patch_file, Filename &file);

  INLINE float get_progress() const;

  INLINE void set_allow_multifile(bool allow_multifile);
  INLINE bool get_allow_multifile();

  INLINE void set_footprint_length(int length);
  INLINE int get_footprint_length();
  INLINE void reset_footprint_length();

  INLINE bool has_source_hash() const;
  INLINE const HashVal &get_source_hash() const;
  INLINE const HashVal &get_result_hash() const;

private:
  int internal_read_header(const Filename &patch_file);
  void init(PT(Buffer) buffer);
  void cleanup();

private:
  // stuff for the build operation
  void build_hash_link_tables(const char *buffer_orig, PN_uint32 length_orig,
    PN_uint32 *hash_table, PN_uint32 *link_table);
  PN_uint32 calc_hash(const char *buffer);
  void find_longest_match(PN_uint32 new_pos, PN_uint32 &copy_pos, PN_uint16 &copy_length,
    PN_uint32 *hash_table, PN_uint32 *link_table, const char* buffer_orig,
    PN_uint32 length_orig, const char* buffer_new, PN_uint32 length_new);
  PN_uint32 calc_match_length(const char* buf1, const char* buf2, PN_uint32 max_length,
    PN_uint32 min_length);

  void emit_ADD(ostream &write_stream, PN_uint32 length, const char* buffer);
  void emit_COPY(ostream &write_stream, PN_uint32 length, PN_uint32 COPY_pos);
  void emit_add_and_copy(ostream &write_stream, 
                         PN_uint32 add_length, const char *add_buffer,
                         PN_uint32 copy_length, PN_uint32 copy_pos);
  void cache_add_and_copy(ostream &write_stream, 
                          PN_uint32 add_length, const char *add_buffer,
                          PN_uint32 copy_length, PN_uint32 copy_pos);
  void cache_flush(ostream &write_stream);

  void write_header(ostream &write_stream, 
                    istream &stream_orig, istream &stream_new);
  void write_terminator(ostream &write_stream);

  bool compute_patches(ostream &write_stream, PN_uint32 copy_offset,
                       istream &stream_orig, istream &stream_new);
  bool compute_mf_patches(ostream &write_stream, 
                          istream &stream_orig, istream &stream_new);

  static const PN_uint32 _HASH_BITS;
  static const PN_uint32 _HASHTABLESIZE;
  static const PN_uint32 _DEFAULT_FOOTPRINT_LENGTH;
  static const PN_uint32 _NULL_VALUE;
  static const PN_uint32 _MAX_RUN_LENGTH;
  static const PN_uint32 _HASH_MASK;

  bool _allow_multifile;
  PN_uint32 _footprint_length;

  PN_uint32 *_hash_table;

  PN_uint32 _add_pos;
  PN_uint32 _last_copy_pos;

  string _cache_add_data;
  PN_uint32 _cache_copy_start;
  PN_uint32 _cache_copy_length;

protected:
  PT(Buffer) _buffer; // this is the work buffer for apply -- used to prevent virtual memory swapping

  // async patch apply state variables
  bool _initiated;

  PN_uint16 _version_number;

  HashVal _MD5_ofSource;  

  HashVal _MD5_ofResult;  

  PN_uint32 _total_bytes_to_process;
  PN_uint32 _total_bytes_processed;

  ifstream _patch_stream;
  ofstream _write_stream;
  ifstream _origfile_stream;

  Filename _patch_file;
  Filename _orig_file;
  Filename _temp_file;

  static const PN_uint32 _v0_magic_number;
  static const PN_uint32 _magic_number;
  static const PN_uint16 _current_version;
};

#include "patchfile.I"

#endif // HAVE_OPENSSL

#endif
