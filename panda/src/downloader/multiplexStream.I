// Filename: multiplexStream.I
// Created by:  drose (27Nov00)
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

////////////////////////////////////////////////////////////////////
//     Function: MultiplexStream::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE MultiplexStream::
MultiplexStream() : ostream(&_msb) {
  setf(ios::unitbuf);
}

////////////////////////////////////////////////////////////////////
//     Function: MultiplexStream::add_ostream
//       Access: Public
//  Description: Adds the indicated generic ostream to the multiplex
//               output.  The ostream will receive whatever data is
//               sent to the pipe.
////////////////////////////////////////////////////////////////////
INLINE void MultiplexStream::
add_ostream(ostream *out, bool delete_later) {
  _msb.add_output(MultiplexStreamBuf::BT_none,
                  MultiplexStreamBuf::OT_ostream,
                  out, NULL, delete_later);
}

////////////////////////////////////////////////////////////////////
//     Function: MultiplexStream::add_stdio_file
//       Access: Public
//  Description: Adds the given file, previously opened using the C
//               stdio library, to the multiplex output.
////////////////////////////////////////////////////////////////////
INLINE bool MultiplexStream::
add_stdio_file(FILE *fout, bool close_when_done) {
  _msb.add_output(MultiplexStreamBuf::BT_line,
                  MultiplexStreamBuf::OT_ostream,
                  NULL, fout, close_when_done);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiplexStream::add_standard_output
//       Access: Public
//  Description: Adds the standard output channel.
////////////////////////////////////////////////////////////////////
INLINE void MultiplexStream::
add_standard_output() {
  _msb.add_output(MultiplexStreamBuf::BT_none,
                  MultiplexStreamBuf::OT_ostream,
                  &cout, NULL, false);
}

////////////////////////////////////////////////////////////////////
//     Function: MultiplexStream::add_file
//       Access: Public
//  Description: Adds the given file to the multiplex output.  The
//               file is opened in append mode with line buffering.
//               Returns false if the file cannot be opened.
////////////////////////////////////////////////////////////////////
INLINE bool MultiplexStream::
add_file(Filename file) {
  file.set_text();
  pofstream *out = new pofstream;
  if (!file.open_append(*out)) {
    delete out;
    return false;
  }
  out->setf(ios::unitbuf);

  _msb.add_output(MultiplexStreamBuf::BT_line,
                  MultiplexStreamBuf::OT_ostream,
                  out, NULL, true);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: MultiplexStream::add_system_debug
//       Access: Public
//  Description: Adds the system debug output the the multiplex
//               output.  This may map to a syslog or some such
//               os-specific output system.  It may do nothing on a
//               particular system.
//
//               Presently, this maps only to OutputDebugString() on
//               Windows.
////////////////////////////////////////////////////////////////////
INLINE void MultiplexStream::
add_system_debug() {
  _msb.add_output(MultiplexStreamBuf::BT_line,
                  MultiplexStreamBuf::OT_system_debug);
}

////////////////////////////////////////////////////////////////////
//     Function: MultiplexStream::flush
//       Access: Public
//  Description: Forces out all output that hasn't yet been written.
////////////////////////////////////////////////////////////////////
INLINE void MultiplexStream::
flush() {
  _msb.flush();
}
