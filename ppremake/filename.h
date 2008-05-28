// Filename: filename.h
// Created by:  drose (18Jan99)
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

#ifndef FILENAME_H
#define FILENAME_H

#include "ppremake.h"

#include "vector_string.h"

#include <assert.h>

class DSearchPath;

////////////////////////////////////////////////////////////////////
//       Class : Filename
// Description : The name of a file, such as a texture file or an Egg
//               file.  Stores the full pathname, and includes
//               functions for extracting out the directory prefix
//               part and the file extension and stuff.
//
//               A Filename is also aware of the mapping between the
//               Unix-like filename convention we use internally, and
//               the local OS's specific filename convention, and it
//               knows how to perform basic OS-specific I/O, like
//               testing for file existence and searching a
//               searchpath, as well as the best way to open an
//               fstream for reading or writing.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOL Filename {
PUBLISHED:
  enum Type {
    // These type values must fit within the bits allocated for
    // F_type, below.
    T_general    = 0x00,
    T_dso        = 0x01,
    T_executable = 0x02,
    // Perhaps other types will be added later.
  };

public:
  enum Flags {
    F_type            = 0x0f,
    F_binary          = 0x10,
    F_text            = 0x20,
    F_pattern         = 0x40,
  };

PUBLISHED:
  INLINE Filename(const string &filename = "");
  INLINE Filename(const char *filename);
  INLINE Filename(const Filename &copy);
  Filename(const Filename &dirname, const Filename &basename);
  INLINE ~Filename();

  // Static constructors to explicitly create a filename that refers
  // to a text or binary file.  This is in lieu of calling set_text()
  // or set_binary() or set_type().
  INLINE static Filename text_filename(const string &filename);
  INLINE static Filename binary_filename(const string &filename);
  INLINE static Filename dso_filename(const string &filename);
  INLINE static Filename executable_filename(const string &filename);

  INLINE static Filename pattern_filename(const string &filename);

  static Filename from_os_specific(const string &os_specific,
                                   Type type = T_general);
  static Filename expand_from(const string &user_string, 
                              Type type = T_general);
  static Filename temporary(const string &dirname, const string &prefix,
                            const string &suffix = string(),
                            Type type = T_general);

  // Assignment is via the = operator.
  INLINE Filename &operator = (const string &filename);
  INLINE Filename &operator = (const char *filename);
  INLINE Filename &operator = (const Filename &copy);

  // And retrieval is by any of the classic string operations.
  INLINE operator const string & () const;
  INLINE const char *c_str() const;
  INLINE bool empty() const;
  INLINE size_t length() const;
  INLINE char operator [] (int n) const;

  INLINE string substr(size_t begin, size_t end = string::npos) const;

  // Or, you can use any of these.
  INLINE string get_fullpath() const;
  INLINE string get_dirname() const;
  INLINE string get_basename() const;
  INLINE string get_fullpath_wo_extension() const;
  INLINE string get_basename_wo_extension() const;
  INLINE string get_extension() const;

  // You can also use any of these to reassign pieces of the filename.
  void set_fullpath(const string &s);
  void set_dirname(const string &s);
  void set_basename(const string &s);
  void set_fullpath_wo_extension(const string &s);
  void set_basename_wo_extension(const string &s);
  void set_extension(const string &s);

  // Setting these flags appropriately is helpful when opening or
  // searching for a file; it helps the Filename resolve OS-specific
  // conventions (for instance, that dynamic library names should
  // perhaps be changed from .so to .dll).
  INLINE void set_binary();
  INLINE void set_text();
  INLINE bool is_binary() const;
  INLINE bool is_text() const;

  INLINE void set_type(Type type);
  INLINE Type get_type() const;

  INLINE void set_pattern(bool pattern);
  INLINE bool get_pattern() const;

  INLINE bool has_hash() const;
  Filename get_filename_index(int index) const;

  INLINE string get_hash_to_end() const;
  void set_hash_to_end(const string &s);

  void extract_components(vector_string &components) const;
  void standardize();

  // The following functions deal with the outside world.

  INLINE bool is_local() const;
  INLINE bool is_fully_qualified() const;
  void make_absolute();
  void make_absolute(const Filename &start_directory);

  bool make_canonical();
  bool make_true_case();

  string to_os_specific() const;
  string to_os_generic() const;
  string to_os_short_name() const;
  string to_os_long_name() const;

  bool exists() const;
  bool is_regular_file() const;
  bool is_directory() const;
  bool is_executable() const;
  int compare_timestamps(const Filename &other,
                         bool this_missing_is_old = true,
                         bool other_missing_is_old = true) const;
  time_t get_timestamp() const;
  time_t get_access_timestamp() const;
  off_t get_file_size() const;

  bool resolve_filename(const DSearchPath &searchpath,
                        const string &default_extension = string());
  bool make_relative_to(Filename directory, bool allow_backups = true);
  int find_on_searchpath(const DSearchPath &searchpath);
  
  bool scan_directory(vector_string &contents) const;

  bool open_read(ifstream &stream) const;
  bool open_write(ofstream &stream, bool truncate = true) const;
  bool open_append(ofstream &stream) const;
  bool open_read_write(fstream &stream) const;

  bool chdir() const;
  bool touch() const;
  bool unlink() const;
  bool rename_to(const Filename &other) const;

  bool make_dir() const;

  // Comparison operators are handy.
  INLINE bool operator == (const string &other) const;
  INLINE bool operator != (const string &other) const;
  INLINE bool operator < (const string &other) const;
  INLINE int compare_to(const Filename &other) const;

  INLINE void output(ostream &out) const;

public:
  bool atomic_compare_and_exchange_contents(string &orig_contents, const string &old_contents, const string &new_contents) const;
  bool atomic_read_contents(string &contents) const;

protected:
  void locate_basename();
  void locate_extension();
  void locate_hash();
  size_t get_common_prefix(const string &other) const;
  static int count_slashes(const string &str);
  bool r_make_canonical(const Filename &cwd);

  string _filename;
  // We'll make these size_t instead of string::size_type to help out
  // cppParser.
  size_t _dirname_end;
  size_t _basename_start;
  size_t _basename_end;
  size_t _extension_start;
  size_t _hash_start;
  size_t _hash_end;

  int _flags;
};

INLINE ostream &operator << (ostream &out, const Filename &n) {
  n.output(out);
  return out;
}

#include "filename.I"

#endif



