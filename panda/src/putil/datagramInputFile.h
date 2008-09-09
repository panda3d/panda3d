// Filename: datagramInputFile.h
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

#ifndef DATAGRAMINPUTFILE_H
#define DATAGRAMINPUTFILE_H

#include "pandabase.h"

#include "datagramGenerator.h"
#include "filename.h"
#include "virtualFile.h"

////////////////////////////////////////////////////////////////////
//       Class : DatagramInputFile
// Description : This class can be used to read a binary file that
//               consists of an arbitrary header followed by a number
//               of datagrams.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL DatagramInputFile : public DatagramGenerator {
public:
  INLINE DatagramInputFile();

  bool open(Filename filename);
  bool open(istream &in);

  void close();

  bool read_header(string &header, size_t num_bytes);
  virtual bool get_datagram(Datagram &data);
  virtual bool is_eof();
  virtual bool is_error();

  virtual VirtualFile *get_file();
  virtual streampos get_file_pos();

private:
  bool _read_first_datagram;
  bool _error;
  PT(VirtualFile) _vfile;
  pifstream _in_file;
  istream *_in;
  bool _owns_in;
};

#include "datagramInputFile.I"

#endif
