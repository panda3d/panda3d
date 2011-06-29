// Filename: datagramSink.h
// Created by:  jason (07Jun00)
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

#ifndef DATAGRAMSINK_H
#define DATAGRAMSINK_H

#include "pandabase.h"

#include "datagram.h"

class SubfileInfo;
class FileReference;
class Filename;

////////////////////////////////////////////////////////////////////
//       Class : DatagramSink
// Description : This class defines the abstract interface to sending
//               datagrams to any target, whether it be into a file
//               or across the net
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS DatagramSink {
PUBLISHED:
  INLINE DatagramSink();
  virtual ~DatagramSink();

  virtual bool put_datagram(const Datagram &data) = 0;
  virtual bool copy_datagram(SubfileInfo &result, const Filename &filename);
  virtual bool copy_datagram(SubfileInfo &result, const SubfileInfo &source);
  virtual bool is_error() = 0;
  virtual void flush() = 0;

  virtual const Filename &get_filename();
  virtual const FileReference *get_file();
  virtual streampos get_file_pos();
};

#include "datagramSink.I"

#endif

