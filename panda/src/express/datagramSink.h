// Filename: datagramSink.h
// Created by:  jason (07Jun00)
//

#ifndef DATAGRAMSINK_H
#define DATAGRAMSINK_H

#include <pandabase.h>

#include "datagram.h"

////////////////////////////////////////////////////////////////////
// 	 Class : DatagramSink
// Description : This class defines the abstract interface to sending
//               datagrams to any target, whether it be into a file
//               or across the net
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS DatagramSink {
public:
  INLINE DatagramSink(void);
  virtual ~DatagramSink(void);
  
  virtual bool put_datagram(const Datagram& dataBlock) = 0;
};

#include "datagramSink.I"

#endif

