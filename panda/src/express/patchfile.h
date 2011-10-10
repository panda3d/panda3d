// Filename: patchfile.h
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
#include "ordered_vector.h"
#include "streamWrapper.h"

#include <algorithm>


////////////////////////////////////////////////////////////////////
//       Class : Patchfile
// Description :
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Patchfile {
PUBLISHED:
  Patchfile();
  Patchfile(PT(Buffer) buffer);
  ~Patchfile();

  bool build(Filename file_orig, Filename file_new, Filename patch_name);
  int read_header(const Filename &patch_file);

  int initiate(const Filename &patch_file, const Filename &file);
  int initiate(const Filename &patch_file, const Filename &orig_file,
               const Filename &target_file);
  int run();

  bool apply(Filename &patch_file, Filename &file);
  bool apply(Filename &patch_file, Filename &orig_file, 
             const Filename &target_file);

  INLINE PN_stdfloat get_progress() const;

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

  bool compute_file_patches(ostream &write_stream, 
                            PN_uint32 offset_orig, PN_uint32 offset_new,
                             istream &stream_orig, istream &stream_new);
  bool compute_mf_patches(ostream &write_stream, 
                          PN_uint32 offset_orig, PN_uint32 offset_new,
                          istream &stream_orig, istream &stream_new);
#ifdef HAVE_TAR
  class TarSubfile {
  public:
    inline bool operator < (const TarSubfile &other) const {
      return _name < other._name;
    }
    string _name;
    streampos _header_start;
    streampos _data_start;
    streampos _data_end;
    streampos _end;
  };
  typedef ov_set<TarSubfile> TarDef;

  bool read_tar(TarDef &tar, istream &stream);
  bool compute_tar_patches(ostream &write_stream, 
                           PN_uint32 offset_orig, PN_uint32 offset_new,
                           istream &stream_orig, istream &stream_new,
                           TarDef &tar_orig, TarDef &tar_new);

  // Because this is static, we can only call read_tar() one at a
  // time--no threads, please.
  static istream *_tar_istream;
  
  static int tar_openfunc(const char *filename, int oflags, ...);
  static int tar_closefunc(int fd);
  static ssize_t tar_readfunc(int fd, void *buffer, size_t nbytes);
  static ssize_t tar_writefunc(int fd, const void *buffer, size_t nbytes);
#endif  // HAVE_TAR

  bool do_compute_patches(const Filename &file_orig, const Filename &file_new,
                          ostream &write_stream, 
                          PN_uint32 offset_orig, PN_uint32 offset_new,
                          istream &stream_orig, istream &stream_new);
  
  bool patch_subfile(ostream &write_stream, 
                     PN_uint32 offset_orig, PN_uint32 offset_new,
                     const Filename &filename,
                     IStreamWrapper &stream_orig, streampos orig_start, streampos orig_end,
                     IStreamWrapper &stream_new, streampos new_start, streampos new_end);

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

private:
  PT(Buffer) _buffer; // this is the work buffer for apply -- used to prevent virtual memory swapping

  // async patch apply state variables
  bool _initiated;

  PN_uint16 _version_number;

  HashVal _MD5_ofSource;  

  HashVal _MD5_ofResult;  

  PN_uint32 _total_bytes_to_process;
  PN_uint32 _total_bytes_processed;

  istream *_patch_stream;
  pofstream _write_stream;
  istream *_origfile_stream;

  Filename _patch_file;
  Filename _orig_file;
  Filename _output_file;
  bool _rename_output_to_orig;
  bool _delete_patchfile;

  static const PN_uint32 _v0_magic_number;
  static const PN_uint32 _magic_number;
  static const PN_uint16 _current_version;
};

#include "patchfile.I"

#endif // HAVE_OPENSSL

#endif
