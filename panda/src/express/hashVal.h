// Filename: hashVal.h
// Created by:  drose (14Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef HASHVAL_H
#define HASHVAL_H

#include "pandabase.h"
#include "typedef.h"
#include "notify.h"
#include "ramfile.h"
#include "datagram.h"
#include "datagramIterator.h"
#include "streamWriter.h"
#include "streamReader.h"

////////////////////////////////////////////////////////////////////
//       Class : HashVal
// Description : Stores a 128-bit value that represents the hashed
//               contents (typically MD5) of a file or buffer.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS HashVal {
PUBLISHED:
  INLINE HashVal();
  INLINE HashVal(const HashVal &copy);
  INLINE void operator = (const HashVal &copy);

  INLINE bool operator == (const HashVal &other) const;
  INLINE bool operator != (const HashVal &other) const;
  INLINE bool operator < (const HashVal &other) const;
  INLINE int compare_to(const HashVal &other) const;

  INLINE void merge_with(const HashVal &other);

  INLINE void output_dec(ostream &out) const;
  INLINE void input_dec(istream &in);
  void output_hex(ostream &out) const;
  void input_hex(istream &in);

  INLINE void output(ostream &out) const;

  string as_dec() const;
  bool set_from_dec(const string &text);

  string as_hex() const;
  bool set_from_hex(const string &text);

  INLINE void write_datagram(Datagram &destination) const;
  INLINE void read_datagram(DatagramIterator &source);
  INLINE void write_stream(StreamWriter &destination) const;
  INLINE void read_stream(StreamReader &source);

#ifdef HAVE_SSL
  bool hash_file(const Filename &filename);
  INLINE void hash_ramfile(const Ramfile &ramfile);
  INLINE void hash_string(const string &data);
  void hash_buffer(const char *buffer, int length);
#endif  // HAVE_SSL

private:
  static void encode_hex(unsigned int val, char *buffer);
  static void decode_hex(const char *buffer, unsigned int &val);
  INLINE static char tohex(unsigned int nibble);
  INLINE static unsigned int fromhex(char digit);

  PN_uint32 _hv[4];
};

INLINE ostream &operator << (ostream &out, const HashVal &hv);

#include "hashVal.I"

#endif
