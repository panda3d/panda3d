// Filename: datagramGenerator.h
// Created by:  jason (07Jun00)
//

#ifndef DATAGRAMGENERATOR_H
#define DATAGRAMGENERATOR_H

#include <pandabase.h>

#include "datagram.h"

////////////////////////////////////////////////////////////////////
// 	 Class : DatagramGenerator
// Description : This class defines the abstract interace to any
//               source of datagrams, whether it be from a file or
//               from the net
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS DatagramGenerator {
public:
  INLINE DatagramGenerator();
  virtual ~DatagramGenerator();
  
  virtual bool get_datagram(Datagram &data) = 0;
  virtual bool is_eof() = 0;
  virtual bool is_error() = 0;
};

#include "datagramGenerator.I"

#endif
