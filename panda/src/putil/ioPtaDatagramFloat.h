// Filename: ioPtaDatagramFloat.h
// Created by:  charles (10Jul00)
// 
////////////////////////////////////////////////////////////////////

#ifndef _IO_PTA_DATAGRAM_FLOAT
#define _IO_PTA_DATAGRAM_FLOAT

#include <pandabase.h>

#include "pointerToArray.h"
#include "pta_float.h"

class Datagram;
class DatagramIterator;

///////////////////////////////////////////////////////////////////
//       Class : IoPtaDatagramFloat
// Description : This class is used to read and write a PTA_float
//               from a Datagram, in support of Bam.  It's not
//               intended to be constructed; it's just a convenient
//               place to scope these static methods which should be
//               called directly.
////////////////////////////////////////////////////////////////////
class IoPtaDatagramFloat {
public:
  static void write_datagram(Datagram &dest, CPTA_float array);
  static PTA_float read_datagram(DatagramIterator &source);
};

typedef IoPtaDatagramFloat IPD_float;

#endif // _IO_PTA_DATAGRAM_FLOAT
