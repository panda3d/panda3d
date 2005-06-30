// Filename: ioPtaDatagramShort.h
// Created by:  jason (26Jun00)
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

#ifndef _IO_PTA_DATAGRAM_SHORT
#define _IO_PTA_DATAGRAM_SHORT

#include "pandabase.h"

#include "pointerToArray.h"
#include "pta_ushort.h"

class BamReader;
class BamWriter;
class Datagram;
class DatagramIterator;

////////////////////////////////////////////////////////////////////
//       Class : IoPtaDatagramShort
// Description : This class is used to read and write a PTA_ushort
//               from a Datagram, in support of Bam.  It's not
//               intended to be constructed; it's just a convenient
//               place to scope these static methods which should be
//               called directly.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA IoPtaDatagramShort {
public:
  static void write_datagram(BamWriter *manager, Datagram &dest, CPTA_ushort array);
  static PTA_ushort read_datagram(BamReader *manager, DatagramIterator &source);
};

typedef IoPtaDatagramShort IPD_ushort;

#endif
