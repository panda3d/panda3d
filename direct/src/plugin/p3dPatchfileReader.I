// Filename: p3dPatchfileReader.I
// Created by:  drose (28Sep09)
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
//     Function: P3DPatchfileReader::is_open
//       Access: Public
//  Description: Returns true if the patchfile is currently open,
//               false otherwise.
////////////////////////////////////////////////////////////////////
inline bool P3DPatchfileReader::
is_open() const {
  return _is_open;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::get_bytes_written
//       Access: Public
//  Description: Returns the number of bytes written to the output
//               file so far during the patching process.
////////////////////////////////////////////////////////////////////
inline size_t P3DPatchfileReader::
get_bytes_written() const {
  return _bytes_written;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::get_success
//       Access: Public
//  Description: Returns true if the patching process has completed
//               successfully, false if it has failed or has not yet
//               completed.
////////////////////////////////////////////////////////////////////
inline bool P3DPatchfileReader::
get_success() const {
  return _success;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::read_uint16
//       Access: Private
//  Description: Extracts an unsigned short from the patchfile.
////////////////////////////////////////////////////////////////////
inline unsigned int P3DPatchfileReader::
read_uint16() {
  unsigned int a = _patch_in.get();
  unsigned int b = _patch_in.get();
  return (b << 8) | a;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::read_uint32
//       Access: Private
//  Description: Extracts an unsigned long from the patchfile.
////////////////////////////////////////////////////////////////////
inline unsigned int P3DPatchfileReader::
read_uint32() {
  unsigned int a = _patch_in.get();
  unsigned int b = _patch_in.get();
  unsigned int c = _patch_in.get();
  unsigned int d = _patch_in.get();
  return (d << 24) | (c << 16) | (b << 8) | a;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchfileReader::read_int32
//       Access: Private
//  Description: Extracts a signed long from the patchfile.
////////////////////////////////////////////////////////////////////
inline int P3DPatchfileReader::
read_int32() {
  unsigned int a = _patch_in.get();
  unsigned int b = _patch_in.get();
  unsigned int c = _patch_in.get();
  int d = _patch_in.get();
  return (d << 24) | (c << 16) | (b << 8) | a;
}
