// Filename: littleEndian.h
// Created by:  drose (09Feb00)
//
////////////////////////////////////////////////////////////////////

#ifndef LITTLEENDIAN_H
#define LITTLEENDIAN_H

#include <pandabase.h>

#include "numeric_types.h"

#ifdef IS_BIG_ENDIAN

////////////////////////////////////////////////////////////////////
// 	 Class : LittleEndian
// Description : LittleEndian is a special string-like class that
//               automatically reverses the byte order when it is
//               assigned from either a char buffer or a true
//               string--but only when compiling for a bigendian
//               architecture.  On littleendian machines, LittleEndian
//               is defined to map directly to string.
//
//               This is a sneaky interface to automatically handle
//               numeric conversions so that network data is always
//               sent littleendian.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS LittleEndian {
public:
  INLINE LittleEndian(const string &string);
  INLINE LittleEndian(const char *data, size_t length);
  INLINE LittleEndian(const LittleEndian &other);

  INLINE void operator =(const string &string);
  INLINE void operator =(const LittleEndian &other);

  INLINE operator const string &() const;
  INLINE const char *data() const;
  INLINE size_t length() const;

private:
  void reverse_assign(const char *data, size_t length);

  string _str;
};

#else  // !IS_BIG_ENDIAN

#define LittleEndian string

#endif // IS_BIG_ENDIAN

#include "littleEndian.I"

#endif







