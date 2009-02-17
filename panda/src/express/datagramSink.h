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
  virtual bool is_error() = 0;
};

#include "datagramSink.I"

#endif

