// Filename: encryptStreamBuf.h
// Created by:  drose (01Sep04)
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

#ifndef ENCRYPTSTREAMBUF_H
#define ENCRYPTSTREAMBUF_H

#include "dtoolbase.h"

// This module is not compiled if OpenSSL is not available.
#ifdef HAVE_OPENSSL

#include "openssl/evp.h"

////////////////////////////////////////////////////////////////////
//       Class : EncryptStreamBuf
// Description : The streambuf object that implements
//               IDecompressStream and OCompressStream.
////////////////////////////////////////////////////////////////////
class EXPCL_DTOOLCONFIG EncryptStreamBuf : public streambuf {
public:
  EncryptStreamBuf();
  virtual ~EncryptStreamBuf();

  void open_read(istream *source, bool owns_source, const string &password);
  void close_read();

  void open_write(ostream *dest, bool owns_dest, const string &password);
  void close_write();

  INLINE void set_algorithm(const string &algorithm);
  INLINE const string &get_algorithm() const;

  INLINE void set_key_length(int key_length);
  INLINE int get_key_length() const;

  INLINE void set_iteration_count(int iteration_count);
  INLINE int get_iteration_count() const;

protected:
  virtual int overflow(int c);
  virtual int sync();
  virtual int underflow();

private:
  size_t read_chars(char *start, size_t length);
  void write_chars(const char *start, size_t length);

private:
  istream *_source;
  bool _owns_source;

  ostream *_dest;
  bool _owns_dest;

  string _algorithm;
  int _key_length;
  int _iteration_count;
  
  bool _read_valid;
  EVP_CIPHER_CTX _read_ctx;
  size_t _read_block_size;
  unsigned char *_read_overflow_buffer;
  size_t _in_read_overflow_buffer;

  bool _write_valid;
  EVP_CIPHER_CTX _write_ctx;
  size_t _write_block_size;
};

#include "encryptStreamBuf.I"

#endif  // HAVE_OPENSSL

#endif
