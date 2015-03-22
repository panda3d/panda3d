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
#include "openSSLWrapper.h"

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
  BLOCKING bool open_read(const Filename &multifile_name, const streampos &offset = 0);
  BLOCKING bool open_read(IStreamWrapper *multifile_stream, bool owns_pointer = false, const streampos &offset = 0);
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
  INLINE void set_encryption_password(const string &encryption_password);
  INLINE const string &get_encryption_password() const;

  INLINE void set_encryption_algorithm(const string &encryption_algorithm);
  INLINE const string &get_encryption_algorithm() const;
  INLINE void set_encryption_key_length(int encryption_key_length);
  INLINE int get_encryption_key_length() const;
  INLINE void set_encryption_iteration_count(int encryption_iteration_count);
  INLINE int get_encryption_iteration_count() const;

  string add_subfile(const string &subfile_name, const Filename &filename,
                     int compression_level);
  string add_subfile(const string &subfile_name, istream *subfile_data,
                     int compression_level);
  string update_subfile(const string &subfile_name, const Filename &filename,
                        int compression_level);

#ifdef HAVE_OPENSSL
  bool add_signature(const Filename &certificate,
                     const Filename &chain,
                     const Filename &pkey,
                     const string &password = "");
  bool add_signature(const Filename &composite,
                     const string &password = "");

  int get_num_signatures() const;
  string get_signature_subject_name(int n) const;
  string get_signature_friendly_name(int n) const;
  string get_signature_public_key(int n) const;
  void print_signature_certificate(int n, ostream &out) const;
  void write_signature_certificate(int n, ostream &out) const;

  int validate_signature_certificate(int n) const;
#endif  // HAVE_OPENSSL

  BLOCKING bool flush();
  BLOCKING bool repack();

  int get_num_subfiles() const;
  int find_subfile(const string &subfile_name) const;
  bool has_directory(const string &subfile_name) const;
  bool scan_directory(vector_string &contents,
                      const string &subfile_name) const;
  void remove_subfile(int index);
  INLINE bool remove_subfile(const string &subfile_name);
  const string &get_subfile_name(int index) const;
  MAKE_SEQ(get_subfile_names, get_num_subfiles, get_subfile_name);
  size_t get_subfile_length(int index) const;
  time_t get_subfile_timestamp(int index) const;
  bool is_subfile_compressed(int index) const;
  bool is_subfile_encrypted(int index) const;
  bool is_subfile_text(int index) const;

  streampos get_index_end() const;
  streampos get_subfile_internal_start(int index) const;
  size_t get_subfile_internal_length(int index) const;

  BLOCKING INLINE string read_subfile(int index);
  BLOCKING istream *open_read_subfile(int index);
  BLOCKING static void close_read_subfile(istream *stream);
  BLOCKING bool extract_subfile(int index, const Filename &filename);
  BLOCKING bool extract_subfile_to(int index, ostream &out);
  BLOCKING bool compare_subfile(int index, const Filename &filename);

  void output(ostream &out) const;
  void ls(ostream &out = cout) const;

  static INLINE string get_magic_number();

  void set_header_prefix(const string &header_prefix);
  INLINE const string &get_header_prefix() const;

public:
#ifdef HAVE_OPENSSL
  class CertRecord {
  public:
    INLINE CertRecord(X509 *cert);
    INLINE CertRecord(const CertRecord &copy);
    INLINE ~CertRecord();
    INLINE void operator = (const CertRecord &other);
    X509 *_cert;
  };
  typedef pvector<CertRecord> CertChain;

  bool add_signature(X509 *certificate, STACK_OF(X509) *chain, EVP_PKEY *pkey);
  bool add_signature(const CertChain &chain, EVP_PKEY *pkey);

  const CertChain &get_signature(int n) const;
#endif  // HAVE_OPENSSL

  bool read_subfile(int index, string &result);
  bool read_subfile(int index, pvector<unsigned char> &result);

private:
  enum SubfileFlags {
    SF_deleted        = 0x0001,
    SF_index_invalid  = 0x0002,
    SF_data_invalid   = 0x0004,
    SF_compressed     = 0x0008,
    SF_encrypted      = 0x0010,
    SF_signature      = 0x0020,
    SF_text           = 0x0040,
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
    INLINE bool is_cert_special() const;
    INLINE streampos get_last_byte_pos() const;

    string _name;
    streampos _index_start;
    size_t _index_length;
    streampos _data_start;
    size_t _data_length;
    size_t _uncompressed_length;
    time_t _timestamp;
    istream *_source;
    Filename _source_filename;
    int _flags;
    int _compression_level;  // Not preserved on disk.
#ifdef HAVE_OPENSSL
    EVP_PKEY *_pkey;         // Not preserved on disk.
#endif
  };

  INLINE streampos word_to_streampos(size_t word) const;
  INLINE size_t streampos_to_word(streampos fpos) const;
  INLINE streampos normalize_streampos(streampos fpos) const;
  streampos pad_to_streampos(streampos fpos);

  void add_new_subfile(Subfile *subfile, int compression_level);
  istream *open_read_subfile(Subfile *subfile);
  string standardize_subfile_name(const string &subfile_name) const;

  void clear_subfiles();
  bool read_index();
  bool write_header();

  void check_signatures();

  static INLINE char tohex(unsigned int nibble);

  typedef ov_set<Subfile *, IndirectLess<Subfile> > Subfiles;
  Subfiles _subfiles;
  typedef pvector<Subfile *> PendingSubfiles;
  PendingSubfiles _new_subfiles;
  PendingSubfiles _removed_subfiles;
  PendingSubfiles _cert_special;

#ifdef HAVE_OPENSSL
  typedef pvector<CertChain> Certificates;
  Certificates _signatures;
#endif

  streampos _offset;
  IStreamWrapper *_read;
  ostream *_write;
  bool _owns_stream;
  streampos _next_index;
  streampos _last_index;
  streampos _last_data_byte;

  bool _needs_repack;
  time_t _timestamp;
  bool _timestamp_dirty;
  bool _record_timestamp;
  size_t _scale_factor;
  size_t _new_scale_factor;

  bool _encryption_flag;
  string _encryption_password;
  string _encryption_algorithm;
  int _encryption_key_length;
  int _encryption_iteration_count;

  pifstream _read_file;
  IStreamWrapper _read_filew;
  pofstream _write_file;
  pfstream _read_write_file;
  StreamWrapper _read_write_filew;
  Filename _multifile_name;
  string _header_prefix;

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
