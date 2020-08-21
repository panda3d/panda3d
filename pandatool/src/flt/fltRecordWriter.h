/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file fltRecordWriter.h
 * @author drose
 * @date 2000-08-24
 */

#ifndef FLTRECORDWRITER_H
#define FLTRECORDWRITER_H

#include "pandatoolbase.h"

#include "fltOpcode.h"
#include "fltError.h"

#include "datagram.h"
#include "pset.h"

class FltHeader;

/**
 * This class writes a sequence of FltRecords to an ostream, handling opcode
 * and size counts properly.
 */
class FltRecordWriter {
public:
  FltRecordWriter(std::ostream &out);
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
  std::ostream &_out;
  Datagram _datagram;
  FltOpcode _opcode;

  typedef pset<int> Instances;
  Instances _instances_written;
};

#endif
