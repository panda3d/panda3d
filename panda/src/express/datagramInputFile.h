// Filename: datagramInputFile.h
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

#ifndef DATAGRAMINPUTFILE_H
#define DATAGRAMINPUTFILE_H

#include <pandabase.h>

#include "datagramGenerator.h"
#include "filename.h"

////////////////////////////////////////////////////////////////////
//       Class : DatagramInputFile
// Description : This class can be used to read a binary file that
//               consists of an arbitrary header followed by a number
//               of datagrams.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS DatagramInputFile : public DatagramGenerator {
public:
  INLINE DatagramInputFile();

  INLINE bool open(Filename filename);

  bool read_header(string &header, size_t num_bytes);
  virtual bool get_datagram(Datagram &data);
  virtual bool is_eof();
  virtual bool is_error();

  INLINE void close();

private:
  bool _read_first_datagram;
  bool _error;
  ifstream _in;
};

#include "datagramInputFile.I"

#endif
