// Filename: datagramSink.h
// Created by:  jason (07Jun00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
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
public:
  INLINE DatagramSink(void);
  virtual ~DatagramSink(void);

  virtual bool put_datagram(const Datagram &data) = 0;
  virtual bool is_error() = 0;
};

#include "datagramSink.I"

#endif

