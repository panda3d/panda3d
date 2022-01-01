/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file zipArchive.h
 * @author rdb
 * @date 2019-01-20
 */

#ifndef ZIPARCHIVE_H
#define ZIPARCHIVE_H

#include "pandabase.h"

#include "config_express.h"
#include "streamWrapper.h"
#include "subStream.h"
#include "filename.h"
#include "ordered_vector.h"
#include "indirectLess.h"
#include "referenceCount.h"
#include "pvector.h"
#include "vector_uchar.h"

#ifdef HAVE_OPENSSL
typedef struct x509_st X509;
typedef struct evp_pkey_st EVP_PKEY;
#endif

// Defined by Cocoa, conflicts with the definition below.
#undef verify

/**
 * A file that contains a set of files.
 */
class EXPCL_PANDA_EXPRESS ZipArchive : public ReferenceCount {
PUBLISHED:
  ZipArchive();
  ZipArchive(const ZipArchive &copy) = delete;
  ~ZipArchive();

  ZipArchive &operator = (const ZipArchive &copy) = delete;

PUBLISHED:
  BLOCKING bool open_read(const Filename &filename);
  BLOCKING bool open_read(IStreamWrapper *stream, bool owns_pointer = false);
  BLOCKING bool open_write(const Filename &filename);
  BLOCKING bool open_write(std::ostream *stream, bool owns_pointer = false);
  BLOCKING bool open_read_write(const Filename &filename);
  BLOCKING bool open_read_write(std::iostream *stream, bool owns_pointer = false);
  BLOCKING bool verify();
  BLOCKING void close();

  INLINE const Filename &get_filename() const;
  INLINE void set_filename(const Filename &filename);

  INLINE bool is_read_valid() const;
  INLINE bool is_write_valid() const;
  INLINE bool needs_repack() const;

  INLINE void set_record_timestamp(bool record_timestamp);
  INLINE bool get_record_timestamp() const;

  std::string add_subfile(const std::string &subfile_name, const Filename &filename,
                          int compression_level);
  std::string add_subfile(const std::string &subfile_name, std::istream *subfile_data,
                          int compression_level);
  std::string update_subfile(const std::string &subfile_name, const Filename &filename,
                             int compression_level);

#ifdef HAVE_OPENSSL
  bool add_jar_signature(const Filename &certificate, const Filename &pkey,
                         const std::string &password = "",
                         const std::string &alias = "cert");
#endif

  BLOCKING bool flush();
  BLOCKING bool repack();

  int get_num_subfiles() const;
  int find_subfile(const std::string &subfile_name) const;
  bool has_directory(const std::string &subfile_name) const;
  bool scan_directory(vector_string &contents,
                      const std::string &subfile_name) const;
  void remove_subfile(int index);
  INLINE bool remove_subfile(const std::string &subfile_name);
  const std::string &get_subfile_name(int index) const;
  MAKE_SEQ(get_subfile_names, get_num_subfiles, get_subfile_name);
  size_t get_subfile_length(int index) const;
  time_t get_subfile_timestamp(int index) const;
  bool is_subfile_compressed(int index) const;
  bool is_subfile_encrypted(int index) const;

  std::streampos get_subfile_internal_start(int index) const;
  size_t get_subfile_internal_length(int index) const;

  BLOCKING INLINE vector_uchar read_subfile(int index);
  BLOCKING std::istream *open_read_subfile(int index);
  BLOCKING static void close_read_subfile(std::istream *stream);
  BLOCKING bool extract_subfile(int index, const Filename &filename);
  BLOCKING bool extract_subfile_to(int index, std::ostream &out);
  BLOCKING bool compare_subfile(int index, const Filename &filename);

  void output(std::ostream &out) const;
  void ls(std::ostream &out = std::cout) const;

  void set_comment(const std::string &comment);
  INLINE const std::string &get_comment() const;

public:
#ifdef HAVE_OPENSSL
  bool add_jar_signature(X509 *cert, EVP_PKEY *pkey, const std::string &alias);
#endif  // HAVE_OPENSSL

  bool read_subfile(int index, std::string &result);
  bool read_subfile(int index, vector_uchar &result);

private:
  enum SubfileFlags : uint16_t {
    SF_encrypted         = (1 << 0),
    SF_deflate_best      = (1 << 1),
    SF_deflate_fast      = (1 << 2),
    SF_deflate_fastest   = SF_deflate_best | SF_deflate_fast,
    SF_data_descriptor   = (1 << 3),
    SF_strong_encryption = (1 << 6),
    SF_utf8_encoding     = (1 << 11),
  };

  enum CompressionMethod : uint16_t {
    CM_store = 0,
    CM_shrink = 1,
    CM_reduce1 = 2,
    CM_reduce2 = 3,
    CM_reduce3 = 4,
    CM_reduce4 = 5,
    CM_implode = 6,
    CM_tokenize = 7,
    CM_deflate = 8,
    CM_deflate64 = 9,
    CM_bzip2 = 12,
    CM_lzma = 14,
    CM_terse = 18,
    CM_lz77 = 19,
    CM_wavpack = 97,
    CM_ppmd = 98,
  };

  class Subfile {
  public:
    Subfile() = default;
    Subfile(const std::string &name, int compression_level);

    INLINE bool operator < (const Subfile &other) const;

    bool read_index(std::istream &read);
    bool read_header(std::istream &read);
    bool verify_data(std::istream &read);
    bool write_index(std::ostream &write, std::streampos &fpos);
    bool write_header(std::ostream &write, std::streampos &fpos);
    bool write_data(std::ostream &write, std::istream *read,
                    std::streampos &fpos, int compression_level);
    INLINE bool is_compressed() const;
    INLINE bool is_encrypted() const;
    INLINE std::streampos get_last_byte_pos() const;

    std::string _name;
    uint8_t _system = 0;
    size_t _index_length = 0;
    uint32_t _checksum = 0;
    uint64_t _data_length = 0;
    uint64_t _uncompressed_length = 0;
    time_t _timestamp = 0;
    std::streampos _header_start = 0;
    uint16_t _internal_attribs = 0;
    uint32_t _external_attribs = 0;
    std::string _comment;
    int _flags = SF_data_descriptor;
    CompressionMethod _compression_method = CM_store;
  };

  void add_new_subfile(Subfile *subfile, int compression_level);
  std::istream *open_read_subfile(Subfile *subfile);
  std::string standardize_subfile_name(const std::string &subfile_name) const;

  void clear_subfiles();
  bool read_index();
  bool write_index(std::ostream &write, std::streampos &fpos);

  typedef ov_set<Subfile *, IndirectLess<Subfile> > Subfiles;
  Subfiles _subfiles;
  typedef pvector<Subfile *> PendingSubfiles;
  PendingSubfiles _removed_subfiles;

  std::streampos _offset;
  IStreamWrapper *_read;
  std::ostream *_write;
  bool _owns_stream;
  std::streampos _index_start = 0;
  std::streampos _file_end = 0;

  bool _index_changed;
  bool _needs_repack;
  bool _record_timestamp;

  pifstream _read_file;
  IStreamWrapper _read_filew;
  pofstream _write_file;
  pfstream _read_write_file;
  StreamWrapper _read_write_filew;
  Filename _filename;
  std::string _header_prefix;
  std::string _comment;

  friend class Subfile;
};

#include "zipArchive.I"

#endif
