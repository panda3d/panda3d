/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file datagramBuffer.h
 * @author rdb
 * @date 2017-11-07
 */

#ifndef DATAGRAMBUFFER_H
#define DATAGRAMBUFFER_H

#include "pandabase.h"
#include "datagramGenerator.h"
#include "datagramSink.h"
#include "vector_uchar.h"

/**
 * This class can be used to write a series of datagrams into a memory buffer.
 * It acts as both a datagram sink and generator; you can fill it up with
 * datagrams and then read as many datagrams from it.
 *
 * This uses the same format as DatagramInputFile and DatagramOutputFile,
 * meaning that Datagram sizes are always stored little-endian.
 */
class EXPCL_PANDA_PUTIL DatagramBuffer : public DatagramSink, public DatagramGenerator {
PUBLISHED:
  INLINE DatagramBuffer();
  INLINE explicit DatagramBuffer(vector_uchar data);

  INLINE void clear();

public:
  bool write_header(const std::string &header);
  virtual bool put_datagram(const Datagram &data) override;
  virtual void flush() override;

  bool read_header(std::string &header, size_t num_bytes);
  virtual bool get_datagram(Datagram &data) override;
  virtual bool is_eof() override;

  virtual bool is_error() override;

  INLINE const vector_uchar &get_data() const;
  INLINE void set_data(vector_uchar data);
  INLINE void swap_data(vector_uchar &other);

PUBLISHED:
  MAKE_PROPERTY(data, get_data, set_data);

private:
  vector_uchar _data;
  size_t _read_offset;
  bool _wrote_first_datagram;
  bool _read_first_datagram;
};

#include "datagramBuffer.I"

#endif
