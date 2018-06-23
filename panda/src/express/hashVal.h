/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file hashVal.h
 * @author drose
 * @date 2000-11-14
 */

#ifndef HASHVAL_H
#define HASHVAL_H

#include "pandabase.h"
#include "typedef.h"
#include "pnotify.h"
#include "ramfile.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "streamWriter.h"
#include "streamReader.h"
#include "vector_uchar.h"

/**
 * Stores a 128-bit value that represents the hashed contents (typically MD5)
 * of a file or buffer.
 */
class EXPCL_PANDA_EXPRESS HashVal {
PUBLISHED:
  INLINE HashVal();
  INLINE HashVal(const HashVal &copy);
  INLINE void operator = (const HashVal &copy);

  INLINE bool operator == (const HashVal &other) const;
  INLINE bool operator != (const HashVal &other) const;
  INLINE bool operator < (const HashVal &other) const;
  INLINE int compare_to(const HashVal &other) const;

  INLINE void merge_with(const HashVal &other);

  INLINE void output_dec(std::ostream &out) const;
  INLINE void input_dec(std::istream &in);
  void output_hex(std::ostream &out) const;
  void input_hex(std::istream &in);
  void output_binary(std::ostream &out) const;
  void input_binary(std::istream &in);

  INLINE void output(std::ostream &out) const;

  std::string as_dec() const;
  bool set_from_dec(const std::string &text);

  std::string as_hex() const;
  bool set_from_hex(const std::string &text);

  vector_uchar as_bin() const;
  bool set_from_bin(const vector_uchar &text);

  INLINE void write_datagram(Datagram &destination) const;
  INLINE void read_datagram(DatagramIterator &source);
  INLINE void write_stream(StreamWriter &destination) const;
  INLINE void read_stream(StreamReader &source);

#ifdef HAVE_OPENSSL
  bool hash_file(const Filename &filename);
  bool hash_stream(std::istream &stream);
  INLINE void hash_ramfile(const Ramfile &ramfile);
  INLINE void hash_string(const std::string &data);
  INLINE void hash_bytes(const vector_uchar &data);
  void hash_buffer(const char *buffer, int length);
#endif  // HAVE_OPENSSL

private:
  static void encode_hex(uint32_t val, char *buffer);
  static void decode_hex(const char *buffer, uint32_t &val);
  INLINE static char tohex(unsigned int nibble);
  INLINE static unsigned int fromhex(char digit);

  uint32_t _hv[4];
};

INLINE std::ostream &operator << (std::ostream &out, const HashVal &hv);

#include "hashVal.I"

#endif
