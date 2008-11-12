// Filename: multifile.h
// Created by:  mike (09Jan97)
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

#ifndef MULTIFILE_H
#define MULTIFILE_H

#include "pandabase.h"

#include "config_express.h"
#include "streamWrapper.h"
#include "subStream.h"
#include "filename.h"
#include "ordered_vector.h"
#include "indirectLess.h"
#include "referenceCount.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : Multifile
// Description : A file that contains a set of files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Multifile : public ReferenceCount {
PUBLISHED:
  Multifile();
  ~Multifile();

private:
  Multifile(const Multifile &copy);
  void operator = (const Multifile &copy);

PUBLISHED:
  BLOCKING bool open_read(const Filename &multifile_name);
  BLOCKING bool open_read(IStreamWrapper *multifile_stream, bool owns_pointer = false);
  BLOCKING bool open_write(const Filename &multifile_name);
  BLOCKING bool open_write(ostream *multifile_stream, bool owns_pointer = false);
  BLOCKING bool open_read_write(const Filename &multifile_name);
  BLOCKING bool open_read_write(iostream *multifile_stream, bool owns_pointer = false);
  BLOCKING void close();

  INLINE const Filename &get_multifile_name() const;
  INLINE void set_multifile_name(const Filename &multifile_name);

  INLINE bool is_read_valid() const;
  INLINE bool is_write_valid() const;
  INLINE bool needs_repack() const;

  INLINE time_t get_timestamp() const;

  INLINE void set_record_timestamp(bool record_timestamp);
  INLINE bool get_record_timestamp() const;

  void set_scale_factor(size_t scale_factor);
  INLINE size_t get_scale_factor() const;

  INLINE void set_encryption_flag(bool flag);
  INLINE bool get_encryption_flag() const;
  INLINE void set_encryption_password(const string &password);
  INLINE const string &get_encryption_password() const;

  string add_subfile(const string &subfile_name, const Filename &filename,
                     int compression_level);
  string add_subfile(const string &subfile_name, istream *subfile_data,
                     int compression_level);
  string update_subfile(const string &subfile_name, const Filename &filename,
                        int compression_level);
  BLOCKING bool flush();
  BLOCKING bool repack();

  int get_num_subfiles() const;
  int find_subfile(const string &subfile_name) const;
  bool has_directory(const string &subfile_name) const;
  bool scan_directory(vector_string &contents,
                      const string &subfile_name) const;
  void remove_subfile(int index);
  const string &get_subfile_name(int index) const;
  MAKE_SEQ(get_subfile_names, get_num_subfiles, get_subfile_name);
  size_t get_subfile_length(int index) const;
  time_t get_subfile_timestamp(int index) const;
  bool is_subfile_compressed(int index) const;
  bool is_subfile_encrypted(int index) const;

  streampos get_index_end() const;
  streampos get_subfile_internal_start(int index) const;
  size_t get_subfile_internal_length(int index) const;

  BLOCKING INLINE string read_subfile(int index);
  BLOCKING istream *open_read_subfile(int index);
  BLOCKING bool extract_subfile(int index, const Filename &filename);
  BLOCKING bool extract_subfile_to(int index, ostream &out);
  BLOCKING bool compare_subfile(int index, const Filename &filename);

  void output(ostream &out) const;
  void ls(ostream &out = cout) const;

  static INLINE string get_magic_number();

public:
  bool read_subfile(int index, string &result);
  bool read_subfile(int index, pvector<unsigned char> &result);

private:
  enum SubfileFlags {
    SF_deleted        = 0x0001,
    SF_index_invalid  = 0x0002,
    SF_data_invalid   = 0x0004,
    SF_compressed     = 0x0008,
    SF_encrypted      = 0x0010,
  };

  class Subfile {
  public:
    INLINE Subfile();
    INLINE bool operator < (const Subfile &other) const;
    streampos read_index(istream &read, streampos fpos,
                         Multifile *multfile);
    streampos write_index(ostream &write, streampos fpos,
                          Multifile *multifile);
    streampos write_data(ostream &write, istream *read, streampos fpos,
                         Multifile *multifile);
    void rewrite_index_data_start(ostream &write, Multifile *multifile);
    void rewrite_index_flags(ostream &write);
    INLINE bool is_deleted() const;
    INLINE bool is_index_invalid() const;
    INLINE bool is_data_invalid() const;

    string _name;
    streampos _index_start;
    streampos _data_start;
    size_t _data_length;
    size_t _uncompressed_length;
    time_t _timestamp;
    istream *_source;
    Filename _source_filename;
    int _flags;
    int _compression_level;  // Not preserved on disk.
  };

  INLINE streampos word_to_streampos(size_t word) const;
  INLINE size_t streampos_to_word(streampos fpos) const;
  INLINE streampos normalize_streampos(streampos fpos) const;
  streampos pad_to_streampos(streampos fpos);

  void add_new_subfile(Subfile *subfile, int compression_level);
  string standardize_subfile_name(const string &subfile_name) const;

  void clear_subfiles();
  bool read_index();
  bool write_header();


  typedef ov_set<Subfile *, IndirectLess<Subfile> > Subfiles;
  Subfiles _subfiles;
  typedef pvector<Subfile *> PendingSubfiles;
  PendingSubfiles _new_subfiles;
  PendingSubfiles _removed_subfiles;

  IStreamWrapper *_read;
  ostream *_write;
  bool _owns_stream;
  streampos _next_index;
  streampos _last_index;

  bool _needs_repack;
  time_t _timestamp;
  bool _timestamp_dirty;
  bool _record_timestamp;
  size_t _scale_factor;
  size_t _new_scale_factor;

  bool _encryption_flag;
  string _encryption_password;

  pifstream _read_file;
  IStreamWrapper _read_filew;
  pofstream _write_file;
  pfstream _read_write_file;
  StreamWrapper _read_write_filew;
  Filename _multifile_name;

  int _file_major_ver;
  int _file_minor_ver;

  static const char _header[];
  static const size_t _header_size;
  static const int _current_major_ver;
  static const int _current_minor_ver;

  static const char _encrypt_header[];
  static const size_t _encrypt_header_size;

  friend class Subfile;
};

#include "multifile.I"

#endif
