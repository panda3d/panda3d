// Filename: multifile.h
// Created by:  mike (09Jan97)
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

#ifndef MULTIFILE_H
#define MULTIFILE_H

#include "pandabase.h"

#include "config_express.h"
#include "subStream.h"
#include "filename.h"
#include "ordered_vector.h"
#include "indirectLess.h"
#include "pvector.h"

////////////////////////////////////////////////////////////////////
//       Class : Multifile
// Description : A file that contains a set of files.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS Multifile {
PUBLISHED:
  Multifile();
  ~Multifile();

private:
  Multifile(const Multifile &copy);
  void operator = (const Multifile &copy);

PUBLISHED:
  bool open_read(const Filename &multifile_name);
  bool open_write(const Filename &multifile_name);
  bool open_read_write(const Filename &multifile_name);
  void close();

  INLINE const Filename &get_multifile_name() const;

  INLINE bool is_read_valid() const;
  INLINE bool is_write_valid() const;
  INLINE bool needs_repack() const;

  void set_scale_factor(size_t scale_factor);
  INLINE size_t get_scale_factor() const;

  INLINE void set_encryption_flag(bool flag);
  INLINE bool get_encryption_flag() const;
  INLINE void set_encryption_password(const string &password);
  INLINE const string &get_encryption_password() const;

  string add_subfile(const string &subfile_name, const Filename &filename,
                     int compression_level);
  string update_subfile(const string &subfile_name, const Filename &filename,
                        int compression_level);
  bool flush();
  bool repack();

  int get_num_subfiles() const;
  int find_subfile(const string &subfile_name) const;
  bool has_directory(const string &subfile_name) const;
  bool scan_directory(vector_string &contents,
                      const string &subfile_name) const;
  void remove_subfile(int index);
  const string &get_subfile_name(int index) const;
  size_t get_subfile_length(int index) const;
  bool is_subfile_compressed(int index) const;
  bool is_subfile_encrypted(int index) const;
  size_t get_subfile_internal_length(int index) const;

  INLINE string read_subfile(int index);
  istream *open_read_subfile(int index);
  bool extract_subfile(int index, const Filename &filename);
  bool compare_subfile(int index, const Filename &filename);

  void output(ostream &out) const;
  void ls(ostream &out = cout) const;

public:
  bool read_subfile(int index, string &result);

  // Special interfaces to work with iostreams, not necessarily files.
  bool open_read(istream *multifile_stream);
  bool open_write(ostream *multifile_stream);
  bool open_read_write(iostream *multifile_stream);
  string add_subfile(const string &subfile_name, istream *subfile_data,
                     int compression_level);

  bool extract_subfile_to(int index, ostream &out);

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

  istream *_read;
  ostream *_write;
  streampos _next_index;
  streampos _last_index;

  bool _needs_repack;
  size_t _scale_factor;
  size_t _new_scale_factor;

  bool _encryption_flag;
  string _encryption_password;

  ifstream _read_file;
  ofstream _write_file;
  fstream _read_write_file;
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
