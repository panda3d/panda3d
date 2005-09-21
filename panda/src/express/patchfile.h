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
#include "notify.h"
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

  void emit_ADD(ofstream &write_stream, PN_uint16 length, const char* buffer,
                PN_uint32 ADD_pos);
  void emit_COPY(ofstream &write_stream, PN_uint16 length, 
                 PN_uint32 COPY_pos, PN_uint32 last_copy_pos,
                 PN_uint32 ADD_pos);

  static const PN_uint32 _HASH_BITS;
  static const PN_uint32 _HASHTABLESIZE;
  static const PN_uint32 _DEFAULT_FOOTPRINT_LENGTH;
  static const PN_uint32 _NULL_VALUE;
  static const PN_uint32 _MAX_RUN_LENGTH;
  static const PN_uint32 _HASH_MASK;

  PN_uint32 _footprint_length;

protected:
  PT(Buffer) _buffer; // this is the work buffer for apply -- used to prevent virtual memory swapping

  // async patch apply state variables
  bool _initiated;

  PN_uint16 _version_number;

  HashVal _MD5_ofSource;  
  PN_uint32 _source_file_length;

  HashVal _MD5_ofResult;  
  PN_uint32 _result_file_length;
  int _total_bytes_processed;

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
