// Filename: fltRecordWriter.h
// Created by:  drose (24Aug00)
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

#ifndef FLTRECORDWRITER_H
#define FLTRECORDWRITER_H

#include "pandatoolbase.h"

#include "fltOpcode.h"
#include "fltError.h"

#include "datagram.h"
#include "pset.h"

class FltHeader;

////////////////////////////////////////////////////////////////////
//       Class : FltRecordWriter
// Description : This class writes a sequence of FltRecords to an
//               ostream, handling opcode and size counts properly.
////////////////////////////////////////////////////////////////////
class FltRecordWriter {
public:
  FltRecordWriter(ostream &out);
  ~FltRecordWriter();

  void set_opcode(FltOpcode opcode);
  const Datagram &get_datagram() const;
  void set_datagram(const Datagram &datagram);
  Datagram &update_datagram();

  FltError advance();

  FltError write_record(FltOpcode opcode,
                        const Datagram &datagram = Datagram());

  FltError write_instance_def(FltHeader *header, int instance_index);

private:
  ostream &_out;
  Datagram _datagram;
  FltOpcode _opcode;

  typedef pset<int> Instances;
  Instances _instances_written;
};

#endif


