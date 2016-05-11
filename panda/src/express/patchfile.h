/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file patchfile.h
 * @author darren, mike
 * @date 1997-01-09
 */

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


/**
 *
 */
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
  MAKE_PROPERTY(progress, get_progress);

  INLINE void set_allow_multifile(bool allow_multifile);
  INLINE bool get_allow_multifile();
  MAKE_PROPERTY(allow_multifile, get_allow_multifile, set_allow_multifile);

  INLINE void set_footprint_length(int length);
  INLINE int get_footprint_length();
  INLINE void reset_footprint_length();
  MAKE_PROPERTY(footprint_length, get_footprint_length, set_footprint_length);

  INLINE bool has_source_hash() const;
  INLINE const HashVal &get_source_hash() const;
  INLINE const HashVal &get_result_hash() const;
  MAKE_PROPERTY2(source_hash, has_source_hash, get_source_hash);
  MAKE_PROPERTY(result_hash, get_result_hash);

private:
  int internal_read_header(const Filename &patch_file);
  void init(PT(Buffer) buffer);
  void cleanup();

private:
  // stuff for the build operation
  void build_hash_link_tables(const char *buffer_orig, uint32_t length_orig,
    uint32_t *hash_table, uint32_t *link_table);
  uint32_t calc_hash(const char *buffer);
  void find_longest_match(uint32_t new_pos, uint32_t &copy_pos, uint16_t &copy_length,
    uint32_t *hash_table, uint32_t *link_table, const char* buffer_orig,
    uint32_t length_orig, const char* buffer_new, uint32_t length_new);
  uint32_t calc_match_length(const char* buf1, const char* buf2, uint32_t max_length,
    uint32_t min_length);

  void emit_ADD(ostream &write_stream, uint32_t length, const char* buffer);
  void emit_COPY(ostream &write_stream, uint32_t length, uint32_t COPY_pos);
  void emit_add_and_copy(ostream &write_stream,
                         uint32_t add_length, const char *add_buffer,
                         uint32_t copy_length, uint32_t copy_pos);
  void cache_add_and_copy(ostream &write_stream,
                          uint32_t add_length, const char *add_buffer,
                          uint32_t copy_length, uint32_t copy_pos);
  void cache_flush(ostream &write_stream);

  void write_header(ostream &write_stream,
                    istream &stream_orig, istream &stream_new);
  void write_terminator(ostream &write_stream);

  bool compute_file_patches(ostream &write_stream,
                            uint32_t offset_orig, uint32_t offset_new,
                             istream &stream_orig, istream &stream_new);
  bool compute_mf_patches(ostream &write_stream,
                          uint32_t offset_orig, uint32_t offset_new,
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
                           uint32_t offset_orig, uint32_t offset_new,
                           istream &stream_orig, istream &stream_new,
                           TarDef &tar_orig, TarDef &tar_new);

  // Because this is static, we can only call read_tar() one at a time--no
  // threads, please.
  static istream *_tar_istream;

  static int tar_openfunc(const char *filename, int oflags, ...);
  static int tar_closefunc(int fd);
  static ssize_t tar_readfunc(int fd, void *buffer, size_t nbytes);
  static ssize_t tar_writefunc(int fd, const void *buffer, size_t nbytes);
#endif  // HAVE_TAR

  bool do_compute_patches(const Filename &file_orig, const Filename &file_new,
                          ostream &write_stream,
                          uint32_t offset_orig, uint32_t offset_new,
                          istream &stream_orig, istream &stream_new);

  bool patch_subfile(ostream &write_stream,
                     uint32_t offset_orig, uint32_t offset_new,
                     const Filename &filename,
                     IStreamWrapper &stream_orig, streampos orig_start, streampos orig_end,
                     IStreamWrapper &stream_new, streampos new_start, streampos new_end);

  static const uint32_t _HASH_BITS;
  static const uint32_t _HASHTABLESIZE;
  static const uint32_t _DEFAULT_FOOTPRINT_LENGTH;
  static const uint32_t _NULL_VALUE;
  static const uint32_t _MAX_RUN_LENGTH;
  static const uint32_t _HASH_MASK;

  bool _allow_multifile;
  uint32_t _footprint_length;

  uint32_t *_hash_table;

  uint32_t _add_pos;
  uint32_t _last_copy_pos;

  string _cache_add_data;
  uint32_t _cache_copy_start;
  uint32_t _cache_copy_length;

private:
  PT(Buffer) _buffer; // this is the work buffer for apply -- used to prevent virtual memory swapping

  // async patch apply state variables
  bool _initiated;

  uint16_t _version_number;

  HashVal _MD5_ofSource;

  HashVal _MD5_ofResult;

  uint32_t _total_bytes_to_process;
  uint32_t _total_bytes_processed;

  istream *_patch_stream;
  pofstream _write_stream;
  istream *_origfile_stream;

  Filename _patch_file;
  Filename _orig_file;
  Filename _output_file;
  bool _rename_output_to_orig;
  bool _delete_patchfile;

  static const uint32_t _v0_magic_number;
  static const uint32_t _magic_number;
  static const uint16_t _current_version;
};

#include "patchfile.I"

#endif // HAVE_OPENSSL

#endif
