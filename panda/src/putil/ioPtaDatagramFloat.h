// Filename: ioPtaDatagramFloat.h
// Created by:  charles (10Jul00)
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

#ifndef _IO_PTA_DATAGRAM_FLOAT
#define _IO_PTA_DATAGRAM_FLOAT

#include "pandabase.h"

#include "pointerToArray.h"
#include "pta_float.h"

class BamReader;
class BamWriter;
class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : IoPtaDatagramFloat
// Description : This class is used to read and write a PTA_float
//               from a Datagram, in support of Bam.  It's not
//               intended to be constructed; it's just a convenient
//               place to scope these static methods which should be
//               called directly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA IoPtaDatagramFloat {
public:
  static void write_datagram(BamWriter *manager, Datagram &dest, CPTA_float array);
  static PTA_float read_datagram(BamReader *manager, DatagramIterator &source);
};

typedef IoPtaDatagramFloat IPD_float;

#endif // _IO_PTA_DATAGRAM_FLOAT
