/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltRecordReader.h
 * @author drose
 * @date 2000-08-24
 */

#ifndef FLTRECORDREADER_H
#define FLTRECORDREADER_H

#include "pandatoolbase.h"

#include "fltOpcode.h"
#include "fltError.h"

#include "datagram.h"
#include "datagramIterator.h"

/**
 * This class turns an istream into a sequence of FltRecords by reading a
 * sequence of Datagrams and extracting the opcode from each one.  It
 * remembers where it is in the file and what the current record is.
 */
class FltRecordReader {
public:
  FltRecordReader(std::istream &in);
  ~FltRecordReader();

  FltOpcode get_opcode() const;
  DatagramIterator &get_iterator();
  const Datagram &get_datagram();
  int get_record_length() const;

  FltError advance(bool ok_eof = false);

  bool eof() const;
  bool error() const;

private:
  void read_next_header();

  std::istream &_in;
  Datagram _datagram;
  FltOpcode _opcode;
  int _record_length;
  DatagramIterator *_iterator;

  FltError _next_error;
  FltOpcode _next_opcode;
  int _next_record_length;

  enum State {
    S_begin,
    S_normal,
    S_eof,
    S_error
  };
  State _state;
};

#endif
