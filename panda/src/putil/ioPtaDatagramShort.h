// Filename: ioPtaDatagramShort.h
// Created by:  jason (26Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef _IO_PTA_DATAGRAM_SHORT
#define _IO_PTA_DATAGRAM_SHORT

#include <pandabase.h>

#include "pointerToArray.h"
#include "pta_ushort.h"

class Datagram;
class DatagramIterator;

///////////////////////////////////////////////////////////////////
// 	 Class : IoPtaDatagramShort
// Description : This class is used to read and write a PTA_ushort
//               from a Datagram, in support of Bam.  It's not
//               intended to be constructed; it's just a convenient
//               place to scope these static methods which should be
//               called directly.
////////////////////////////////////////////////////////////////////
class IoPtaDatagramShort {
public:
  static void write_datagram(Datagram &dest, CPTA_ushort array);
  static PTA_ushort read_datagram(DatagramIterator &source);
};

typedef IoPtaDatagramShort IPD_ushort;

#endif
