/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file encryptStreamBuf.h
 * @author drose
 * @date 2004-09-01
 */

#ifndef ENCRYPTSTREAMBUF_H
#define ENCRYPTSTREAMBUF_H

#include "dtoolbase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

typedef struct evp_cipher_ctx_st EVP_CIPHER_CTX;

/**
 * The streambuf object that implements IDecompressStream and OCompressStream.
 */
class EXPCL_DTOOL_PRC EncryptStreamBuf : public std::streambuf {
public:
  EncryptStreamBuf();
  virtual ~EncryptStreamBuf();

  void open_read(std::istream *source, bool owns_source, const std::string &password);
  void close_read();

  void open_write(std::ostream *dest, bool owns_dest, const std::string &password);
  void close_write();

  INLINE void set_algorithm(const std::string &algorithm);
  INLINE const std::string &get_algorithm() const;

  INLINE void set_key_length(int key_length);
  INLINE int get_key_length() const;

  INLINE void set_iteration_count(int iteration_count);
  INLINE int get_iteration_count() const;

  INLINE void set_magic_length(size_t length);
  INLINE size_t get_magic_length() const;

  virtual std::streampos seekoff(std::streamoff off, ios_seekdir dir, ios_openmode which);
  virtual std::streampos seekpos(std::streampos pos, ios_openmode which);

protected:
  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  size_t read_chars(char *start, size_t length);
  void write_chars(const char *start, size_t length);

private:
  std::istream *_source;
  bool _owns_source;

  std::ostream *_dest;
  bool _owns_dest;

  std::string _algorithm;
  int _key_length;
  int _iteration_count;

  EVP_CIPHER_CTX *_read_ctx;
  size_t _read_block_size;
  unsigned char *_read_overflow_buffer;
  size_t _in_read_overflow_buffer;

  EVP_CIPHER_CTX *_write_ctx;
  size_t _write_block_size;

  size_t _magic_length = 0;
  bool _finished = false;
};

#include "encryptStreamBuf.I"

#endif  // HAVE_OPENSSL

#endif
