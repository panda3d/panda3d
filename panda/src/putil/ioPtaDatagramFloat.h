// Filename: ioPtaDatagramFloat.h
// Created by:  charles (10Jul00)
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

#ifndef _IO_PTA_DATAGRAM_FLOAT
#define _IO_PTA_DATAGRAM_FLOAT

#include "pandabase.h"

#include "pointerToArray.h"
#include "pta_stdfloat.h"

class BamReader;
class BamWriter;
class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : IoPtaDatagramFloat
// Description : This class is used to read and write a PTA_stdfloat
//               from a Datagram, in support of Bam.  It's not
//               intended to be constructed; it's just a convenient
//               place to scope these static methods which should be
//               called directly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PUTIL IoPtaDatagramFloat {
public:
  static void write_datagram(BamWriter *manager, Datagram &dest, CPTA_stdfloat array);
  static PTA_stdfloat read_datagram(BamReader *manager, DatagramIterator &source);
};

typedef IoPtaDatagramFloat IPD_float;

#endif // _IO_PTA_DATAGRAM_FLOAT
