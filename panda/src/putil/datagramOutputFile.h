// Filename: datagramOutputFile.h
// Created by:  drose (30Oct00)
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
class EXPCL_PANDA DatagramOutputFile : public DatagramSink {
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
  ofstream _out_file;
  ostream *_out;
  bool _owns_out;
};

#include "datagramOutputFile.I"

#endif
