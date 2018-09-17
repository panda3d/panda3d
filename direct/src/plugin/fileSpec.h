/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fileSpec.h
 * @author drose
 * @date 2009-06-29
 */

#ifndef FILESPEC_H
#define FILESPEC_H

#include "get_tinyxml.h"
#include <string>

/**
 * This simple class is used both within the core API in this module, as well
 * as within the plugin_npapi plugin implementation, to represent a file on
 * disk that may need to be verified or (re)downloaded.
 */
class FileSpec {
public:
  FileSpec();
  FileSpec(const FileSpec &copy);
  void operator = (const FileSpec &copy);
  ~FileSpec();

  void load_xml(TiXmlElement *xelement);
  void store_xml(TiXmlElement *xelement);

  inline const std::string &get_filename() const;
  inline void set_filename(const std::string &filename);
  inline std::string get_pathname(const std::string &package_dir) const;
  inline size_t get_size() const;
  inline time_t get_timestamp() const;
  inline bool has_hash() const;

  bool quick_verify(const std::string &package_dir);
  bool quick_verify_pathname(const std::string &pathname);
  bool full_verify(const std::string &package_dir);
  inline const FileSpec *get_actual_file() const;
  const FileSpec *force_get_actual_file(const std::string &pathname);

  bool check_hash(const std::string &pathname) const;
  bool read_hash(const std::string &pathname);
  bool read_hash_stream(std::istream &in);
  int compare_hash(const FileSpec &other) const;

  void write(std::ostream &out) const;
  void output_hash(std::ostream &out) const;

private:
  bool priv_check_hash(const std::string &pathname, void *stp);
  static inline int decode_hexdigit(char c);
  static inline char encode_hexdigit(int c);

  static bool decode_hex(unsigned char *dest, const char *source, size_t size);
  static void encode_hex(char *dest, const unsigned char *source, size_t size);
  static void stream_hex(std::ostream &out, const unsigned char *source, size_t size);

  enum { hash_size = 16 };

  std::string _filename;
  size_t _size;
  time_t _timestamp;
  unsigned char _hash[hash_size];
  bool _got_hash;

  FileSpec *_actual_file;
};

#include "fileSpec.I"

#endif
