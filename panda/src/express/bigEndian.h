// Filename: bigEndian.h
// Created by:  drose (23Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef BIGENDIAN_H
#define BIGENDIAN_H

#include <pandabase.h>

#include "numeric_types.h"

#ifdef IS_LITTLE_ENDIAN

////////////////////////////////////////////////////////////////////
// 	 Class : BigEndian
// Description : BigEndian is a special string-like class that
//               automatically reverses the byte order when it is
//               assigned from either a char buffer or a true
//               string--but only when compiling for a littleendian
//               architecture.  On bigendian machines, BigEndian
//               is defined to map directly to string.
//
//               See also LittleEndian.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS BigEndian {
public:
  INLINE BigEndian(const string &string);
  INLINE BigEndian(const char *data, size_t length);
  INLINE BigEndian(const BigEndian &other);

  INLINE void operator =(const string &string);
  INLINE void operator =(const BigEndian &other);

  INLINE operator const string &() const;
  INLINE const char *data() const;
  INLINE size_t length() const;

private:
  void reverse_assign(const char *data, size_t length);

  string _str;
};

#else  // !IS_LITTLE_ENDIAN

#define BigEndian string

#endif // IS_LITTLE_ENDIAN

#include "bigEndian.I"

#endif







