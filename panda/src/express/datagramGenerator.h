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
  INLINE DatagramGenerator(void);
  virtual ~DatagramGenerator(void);
  
  virtual bool get_datagram(Datagram& dataBlock) = 0;
  virtual bool empty(void) = 0;
  virtual bool is_valid(void) = 0;
};

#include "datagramGenerator.I"

#endif
