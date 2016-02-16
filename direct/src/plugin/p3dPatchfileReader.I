/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dPatchfileReader.I
 * @author drose
 * @date 2009-09-28
 */

/**
 * Returns true if the patchfile is currently open, false otherwise.
 */
inline bool P3DPatchfileReader::
is_open() const {
  return _is_open;
}

/**
 * Returns the number of bytes written to the output file so far during the
 * patching process.
 */
inline size_t P3DPatchfileReader::
get_bytes_written() const {
  return _bytes_written;
}

/**
 * Returns true if the patching process has completed successfully, false if it
 * has failed or has not yet completed.
 */
inline bool P3DPatchfileReader::
get_success() const {
  return _success;
}

/**
 * Extracts an unsigned short from the patchfile.
 */
inline unsigned int P3DPatchfileReader::
read_uint16() {
  unsigned int a = _patch_in.get();
  unsigned int b = _patch_in.get();
  return (b << 8) | a;
}

/**
 * Extracts an unsigned long from the patchfile.
 */
inline unsigned int P3DPatchfileReader::
read_uint32() {
  unsigned int a = _patch_in.get();
  unsigned int b = _patch_in.get();
  unsigned int c = _patch_in.get();
  unsigned int d = _patch_in.get();
  return (d << 24) | (c << 16) | (b << 8) | a;
}

/**
 * Extracts a signed long from the patchfile.
 */
inline int P3DPatchfileReader::
read_int32() {
  unsigned int a = _patch_in.get();
  unsigned int b = _patch_in.get();
  unsigned int c = _patch_in.get();
  int d = _patch_in.get();
  return (d << 24) | (c << 16) | (b << 8) | a;
}
