// Filename: ioPtaDatagramInt.h
// Created by:  jason (26Jun00)
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

#ifndef _IO_PTA_DATAGRAM_INT
#define _IO_PTA_DATAGRAM_INT

#include "pandabase.h"

#include "pointerToArray.h"
#include "pta_int.h"

class Datagram;
class DatagramIterator;

///////////////////////////////////////////////////////////////////
//       Class : IoPtaDatagramInt
// Description : This class is used to read and write a PTA_int from a
//               Datagram, in support of Bam.  It's not intended to be
//               constructed; it's just a convenient place to scope
//               these static methods which should be called directly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA IoPtaDatagramInt {
public:
  static void write_datagram(Datagram &dest, CPTA_int array);
  static PTA_int read_datagram(DatagramIterator &source);
};

typedef IoPtaDatagramInt IPD_int;

#endif

