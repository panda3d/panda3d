/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file nativeNumericData.I
 * @author drose
 * @date 2001-05-09
 */

/**
 * This constructor accepts the address of a numeric variable, and its sizeof.
 */
INLINE NativeNumericData::
NativeNumericData(const void *data, size_t) :
  _source(data)
{
}

/**
 * This constructor accepts a pointer to a data array containing a packed
 * numeric value, the offset within the array at which the numeric value
 * starts, and the size of the numeric value.
 *
 * It is essential that the array not be destructed or modified as long as the
 * NumericData object remains; it may just store a pointer into that string's
 * internal buffer.
 */
INLINE NativeNumericData::
NativeNumericData(const void *data, size_t start, size_t) {
  _source = (void *)((const char *)data + start);
}

/**
 * Copies the data, with byte reversal if appropriate, into the indicated
 * numeric variable, whose address and sizeof are given.
 */
INLINE void NativeNumericData::
store_value(void *dest, size_t length) const {
  memcpy(dest, _source, length);
}

/**
 * Returns the pointer to the first byte of the data, either reversed or
 * nonreversed, as appropriate.
 */
INLINE const void *NativeNumericData::
get_data() const {
  return _source;
}

// this is for a intel compile .. it is native format and it is readable off
// word boundries
inline void TS_SetVal1(const int8_t * src, int8_t *dst)
{
  *dst = *src;
}
inline void TS_SetVal2(const char * src, char *dst)
{
  *(reinterpret_cast<int16_t *>(dst)) = *(reinterpret_cast  <const int16_t *>(src));
}

inline void TS_SetVal4(const char * src, char *dst)
{
  *(reinterpret_cast <int32_t *>(dst)) = *(reinterpret_cast <const int32_t *>(src));
}


inline void TS_SetVal8(const char * src, char *dst)
{
  *(reinterpret_cast<int64_t *>(dst)) = *(reinterpret_cast<const int64_t *>(src));
}

template<class type> inline type  TS_GetInteger(type &val,const  char * _src)
{
  val = *(reinterpret_cast <const type *>(_src));
  return val;
}

template<class type> inline type  TS_GetIntegerIncPtr(type &val,char *& _src)
{
  val = *(reinterpret_cast <const type *>(_src));
  _src+= sizeof(type);
  return val;
}

template<class type> inline void TS_AddIntegerIncPtr(type val, char *& _dst)
{
  *(reinterpret_cast <type *>(_dst)) = val;
  _dst+= sizeof(type);
}

template<class type> inline void TS_AddInteger(type val, char * _dst)
{
  *(reinterpret_cast <type *>(_dst)) = val;
}

#define TS_GetDirect(TT,SS)  *((TT *)(SS))
#define TS_GetDirectIncPtr(TT,SS) {  _ptr += sizeof(TT); return *((TT *)(SS -sizeof(TT))); }
