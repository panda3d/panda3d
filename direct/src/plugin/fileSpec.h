// Filename: fileSpec.h
// Created by:  drose (29Jun09)
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

#ifndef FILESPEC_H
#define FILESPEC_H

#include "get_tinyxml.h"
#include <string>
using namespace std;

////////////////////////////////////////////////////////////////////
//       Class : FileSpec
// Description : This simple class is used both within the core API in
//               this module, as well as within the plugin_npapi
//               plugin implementation, to represent a file on disk
//               that may need to be verified or (re)downloaded.
////////////////////////////////////////////////////////////////////
class FileSpec {
public:
  FileSpec();
  FileSpec(const FileSpec &copy);
  void operator = (const FileSpec &copy);
  ~FileSpec();

  void load_xml(TiXmlElement *xelement);
  void store_xml(TiXmlElement *xelement);

  inline const string &get_filename() const;
  inline void set_filename(const string &filename);
  inline string get_pathname(const string &package_dir) const;
  inline size_t get_size() const;
  inline time_t get_timestamp() const;
  inline bool has_hash() const;
  
  bool quick_verify(const string &package_dir);
  bool quick_verify_pathname(const string &pathname);
  bool full_verify(const string &package_dir);
  inline const FileSpec *get_actual_file() const;
  const FileSpec *force_get_actual_file(const string &pathname);
  
  bool check_hash(const string &pathname) const;
  bool read_hash(const string &pathname);
  bool read_hash_stream(istream &in);
  int compare_hash(const FileSpec &other) const;

  void write(ostream &out) const;
  void output_hash(ostream &out) const;

private:
  bool priv_check_hash(const string &pathname, void *stp);
  static inline int decode_hexdigit(char c);
  static inline char encode_hexdigit(int c);

  static bool decode_hex(unsigned char *dest, const char *source, size_t size);
  static void encode_hex(char *dest, const unsigned char *source, size_t size);
  static void stream_hex(ostream &out, const unsigned char *source, size_t size);

  enum { hash_size = 16 };

  string _filename;
  size_t _size;
  time_t _timestamp;
  unsigned char _hash[hash_size];
  bool _got_hash;

  FileSpec *_actual_file;
};

#include "fileSpec.I"

#endif
