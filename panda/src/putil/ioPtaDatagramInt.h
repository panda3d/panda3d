// Filename: ioPtaDatagramInt.h
// Created by:  jason (26Jun00)
// 
////////////////////////////////////////////////////////////////////

#ifndef _IO_PTA_DATAGRAM_INT
#define _IO_PTA_DATAGRAM_INT

#include <pandabase.h>

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
class IoPtaDatagramInt {
public:
  static void write_datagram(Datagram &dest, CPTA_int array);
  static PTA_int read_datagram(DatagramIterator &source);
};

typedef IoPtaDatagramInt IPD_int;

#endif

