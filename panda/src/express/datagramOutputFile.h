// Filename: datagramOutputFile.h
// Created by:  drose (30Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef DATAGRAMOUTPUTFILE_H
#define DATAGRAMOUTPUTFILE_H

#include <pandabase.h>

#include "datagramSink.h"
#include "filename.h"

////////////////////////////////////////////////////////////////////
// 	 Class : DatagramOutputFile
// Description : This class can be used to write a binary file that
//               consists of an arbitrary header followed by a number
//               of datagrams.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDAEXPRESS DatagramOutputFile : public DatagramSink {
public:
  INLINE DatagramOutputFile();

  INLINE bool open(Filename filename);

  bool write_header(const string &header);
  virtual bool put_datagram(const Datagram &data);
  virtual bool is_error();

  INLINE void close();

private:
  bool _wrote_first_datagram;
  bool _error;
  ofstream _out;
};

#include "datagramOutputFile.I"

#endif
