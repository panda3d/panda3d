// Filename: datagramIterator.h
// Created by:  jns (07Feb00)
//

#ifndef DATAGRAMITERATOR_H
#define DATAGRAMITERATOR_H

#include <pandabase.h>

#include "datagram.h"
#include "numeric_types.h"

////////////////////////////////////////////////////////////////////
// 	 Class : DatagramIterator
// Description : A class to retrieve the individual data elements
//               previously stored in a Datagram.  Elements may be
//               retrieved one at a time; it is up to the caller to
//               know the correct type and order of each element.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS DatagramIterator {
PUBLISHED:
  INLINE DatagramIterator();
  INLINE DatagramIterator(const Datagram &datagram, size_t offset = 0);
  INLINE DatagramIterator(const DatagramIterator &copy);
  INLINE void operator = (const DatagramIterator &copy);
  INLINE ~DatagramIterator();

  bool get_bool();
  PN_int8 get_int8();
  PN_uint8 get_uint8();

  PN_int16 get_int16();
  PN_int32 get_int32();
  PN_int64 get_int64();
  PN_uint16 get_uint16();
  PN_uint32 get_uint32();
  PN_uint64 get_uint64();
  float get_float32();
  PN_float64 get_float64();

  PN_int16 get_be_int16();
  PN_int32 get_be_int32();
  PN_int64 get_be_int64();
  PN_uint16 get_be_uint16();
  PN_uint32 get_be_uint32();
  PN_uint64 get_be_uint64();
  float get_be_float32();
  PN_float64 get_be_float64();

  string get_string();
  string get_fixed_string(size_t size);

  void skip_bytes(size_t size);
  string extract_bytes(size_t size);

  string get_remaining_bytes() const;
  int get_remaining_size() const;

  const Datagram &get_datagram() const;
  size_t get_current_index() const;
 
private:
  const Datagram *_datagram;
  size_t _current_index;
};
 
#include "datagramIterator.I"

#endif
