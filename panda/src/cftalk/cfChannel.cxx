// Filename: cfChannel.cxx
// Created by:  drose (26Mar09)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "cfChannel.h"

////////////////////////////////////////////////////////////////////
//     Function: CFChannel::Constructor
//       Access: Public
//  Description: The DatagramGenerator and DatagramSink should be
//               newly created on the free store (via the new
//               operator).  The CFChannel will take ownership of
//               these pointers, and will delete them when it
//               destructs.
////////////////////////////////////////////////////////////////////
CFChannel::
CFChannel(DatagramGenerator *dggen, DatagramSink *dgsink) :
  _dggen(dggen),
  _dgsink(dgsink),
  _reader(dggen),
  _writer(dgsink)
{
  bool ok1 = _reader.init();
  bool ok2 = _writer.init();
  nassertv(ok1 && ok2);
}

////////////////////////////////////////////////////////////////////
//     Function: CFChannel::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
CFChannel::
~CFChannel() {
  delete _dggen;
  delete _dgsink;
}

////////////////////////////////////////////////////////////////////
//     Function: CFChannel::send_command
//       Access: Public
//  Description: Delivers a single command to the process at the other
//               end of the channel.
////////////////////////////////////////////////////////////////////
void CFChannel::
send_command(CFCommand *command) {
  bool ok = _writer.write_object(command);
  nassertv(ok);
}

////////////////////////////////////////////////////////////////////
//     Function: CFChannel::receive_command
//       Access: Public
//  Description: Receives a single command from the process at the other
//               end of the channel.  If no command is ready, the
//               thread will block until one is.  Returns NULL when
//               the connection has been closed.
////////////////////////////////////////////////////////////////////
PT(CFCommand) CFChannel::
receive_command() {
  TypedWritable *obj = _reader.read_object();
  CFCommand *command;
  DCAST_INTO_R(command, obj, NULL);
  return command;
}
