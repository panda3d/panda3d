// Filename: bamWriter.I
// Created by:  jason (08Jun00)
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
//     Function: BamWriter::get_target
//       Access: Published
//  Description: Returns the current target of the BamWriter as set by
//               set_target() or the constructor.
////////////////////////////////////////////////////////////////////
INLINE DatagramSink *BamWriter::
get_target() {
  return _target;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::get_filename
//       Access: Published
//  Description: If a BAM is a file, then the BamWriter should
//               contain the name of the file.  This enables the
//               writer to convert pathnames in the BAM to relative
//               to the directory containing the BAM.
////////////////////////////////////////////////////////////////////
INLINE const Filename &BamWriter::
get_filename() const {
  if (_target != (DatagramSink *)NULL) {
    return _target->get_filename();
  }
  static const Filename empty_filename;
  return empty_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::get_file_endian
//       Access: Published
//  Description: Returns the endian preference indicated by the Bam
//               file currently being written.  This does not imply
//               that every number is stored using the indicated
//               convention, but individual objects may choose to
//               respect this flag when recording data.
////////////////////////////////////////////////////////////////////
INLINE BamWriter::BamEndian BamWriter::
get_file_endian() const {
  return _file_endian;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::get_file_stdfloat_double
//       Access: Published
//  Description: Returns true if the file will store all "standard"
//               floats as 64-bit doubles, or false if they are 32-bit
//               floats.  This isn't runtime settable; it's based on
//               the compilation flags of the version of Panda that
//               generated this file.
////////////////////////////////////////////////////////////////////
INLINE bool BamWriter::
get_file_stdfloat_double() const {
  return _file_stdfloat_double;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::get_file_texture_mode
//       Access: Published
//  Description: Returns the BamTextureMode preference indicated by
//               the Bam file currently being written.  Texture
//               objects written to this Bam file will be encoded
//               according to the specified mode.
////////////////////////////////////////////////////////////////////
INLINE BamWriter::BamTextureMode BamWriter::
get_file_texture_mode() const {
  return _file_texture_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::set_file_texture_mode
//       Access: Published
//  Description: Changes the BamTextureMode preference for
//               the Bam file currently being written.  Texture
//               objects written to this Bam file will be encoded
//               according to the specified mode.
////////////////////////////////////////////////////////////////////
INLINE void BamWriter::
set_file_texture_mode(BamTextureMode file_texture_mode) {
  _file_texture_mode = file_texture_mode;
}
