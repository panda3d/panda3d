// Filename: datagramInputFile.cxx
// Created by:  drose (30Oct00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#if defined(WIN32_VC) && !defined(NO_PCH)
#include "express_headers.h"
#endif

#pragma hdrstop

#if !defined(WIN32_VC) || defined(NO_PCH)
#include "numeric_types.h"
#include "datagramIterator.h"
#include "profileTimer.h"
#endif

#include "datagramInputFile.h"

//#define SKYLER_TIMER 1
#ifdef SKYLER_TIMER //[
  EXPCL_PANDAEXPRESS ProfileTimer Skyler_timer_file;
#endif //]

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::read_header
//       Access: Public
//  Description: Reads a sequence of bytes from the beginning of the
//               datagram file.  This may be called any number of
//               times after the file has been opened and before the
//               first datagram is read.  It may not be called once
//               the first datagram has been read.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
read_header(string &header, size_t num_bytes) {
  nassertr(!_read_first_datagram, false);

  char *buffer = (char *)alloca(num_bytes);
  nassertr(buffer != (char *)NULL, false);

  _in.read(buffer, num_bytes);
  if (_in.fail() || _in.eof()) {
    return false;
  }

  header = string(buffer, num_bytes);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::get_datagram
//       Access: Public, Virtual
//  Description: Reads the next datagram from the file.  Returns true
//               on success, false if there is an error or end of
//               file.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
get_datagram(Datagram &data) {
  #ifdef SKYLER_TIMER //[
    Skyler_timer_file.on();
  #endif //]
  _read_first_datagram = true;

  // First, get the size of the upcoming datagram.  We do this with
  // the help of a second datagram.
  char sizebuf[sizeof(PN_uint32)];
  _in.read(sizebuf, sizeof(PN_uint32));
  if (_in.fail() || _in.eof()) {
    #ifdef SKYLER_TIMER //[
      Skyler_timer_file.off("DatagramInputFile::get_datagram");
    #endif //]
    return false;
  }

  Datagram size(sizebuf, sizeof(PN_uint32));
  DatagramIterator di(size);
  PN_uint32 num_bytes = di.get_uint32();

  // Now, read the datagram itself.
  char *buffer = new char[num_bytes];
  nassertr(buffer != (char *)NULL, false);

  _in.read(buffer, num_bytes);
  if (_in.fail() || _in.eof()) {
    _error = true;
    delete[] buffer;
    #ifdef SKYLER_TIMER //[
      Skyler_timer_file.off("DatagramInputFile::get_datagram");
    #endif //]
    return false;
  }

  data = Datagram(buffer, num_bytes);
  delete[] buffer;
  #ifdef SKYLER_TIMER //[
    Skyler_timer_file.off("DatagramInputFile::get_datagram");
  #endif //]
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::is_eof
//       Access: Public, Virtual
//  Description: Returns true if the file has reached the end-of-file.
//               This test may only be made after a call to
//               read_header() or get_datagram() has failed.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
is_eof() {
  return _in.eof();
}

////////////////////////////////////////////////////////////////////
//     Function: DatagramInputFile::is_error
//       Access: Public, Virtual
//  Description: Returns true if the file has reached an error
//               condition.
////////////////////////////////////////////////////////////////////
bool DatagramInputFile::
is_error() {
  if (_in.fail()) {
    _error = true;
  }
  return _error;
}
