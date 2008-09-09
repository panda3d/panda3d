// Filename: datagramOutputFile.h
// Created by:  drose (30Oct00)
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

#ifndef DATAGRAMOUTPUTFILE_H
#define DATAGRAMOUTPUTFILE_H

#include "pandabase.h"

#include "datagramSink.h"
#include "filename.h"

////////////////////////////////////////////////////////////////////
//       Class : DatagramOutputFile
// Description : This class can be used to write a binary file that
//               consists of an arbitrary header followed by a number
//               of datagrams.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL DatagramOutputFile : public DatagramSink {
public:
  INLINE DatagramOutputFile();

  bool open(Filename filename);
  bool open(ostream &out);

  void close();

  bool write_header(const string &header);
  virtual bool put_datagram(const Datagram &data);
  virtual bool is_error();

private:
  bool _wrote_first_datagram;
  bool _error;
  pofstream _out_file;
  ostream *_out;
  bool _owns_out;
};

#include "datagramOutputFile.I"

#endif
