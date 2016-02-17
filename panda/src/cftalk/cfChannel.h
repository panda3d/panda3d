/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file cfChannel.h
 * @author drose
 * @date 2009-03-26
 */

#ifndef CFCHANNEL_H
#define CFCHANNEL_H

#include "pandabase.h"
#include "referenceCount.h"
#include "bamReader.h"
#include "bamWriter.h"
#include "cfCommand.h"

/**
 * Represents an open communication channel in the connected-frame protocol.
 * Commands may be sent and received on this channel.
 */
class EXPCL_CFTALK CFChannel : public ReferenceCount {
public:
  CFChannel(DatagramGenerator *dggen, DatagramSink *dgsink);
  ~CFChannel();

  void send_command(CFCommand *command);
  PT(CFCommand) receive_command();

private:
  DatagramGenerator *_dggen;
  DatagramSink *_dgsink;
  BamReader _reader;
  BamWriter _writer;
};

#include "cfChannel.I"

#endif
