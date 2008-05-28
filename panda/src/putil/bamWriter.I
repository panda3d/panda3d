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
//     Function: BamWriter::get_filename
//       Access: Public
//  Description: If a BAM is a file, then the BamWriter should
//               contain the name of the file.  This enables the
//               writer to convert pathnames in the BAM to relative
//               to the directory containing the BAM.
////////////////////////////////////////////////////////////////////
INLINE const Filename &BamWriter::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::get_file_endian
//       Access: Public
//  Description: Returns the endian preference indicated by the Bam
//               file currently being written.  This does not imply
//               that every number is stored using the indicated
//               convention, but individual objects may choose to
//               respect this flag when recording data.
////////////////////////////////////////////////////////////////////
INLINE BamEndian BamWriter::
get_file_endian() const {
  return _file_endian;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::get_file_texture_mode
//       Access: Public
//  Description: Returns the BamTextureMode preference indicated by
//               the Bam file currently being written.  Texture
//               objects written to this Bam file will be encoded
//               according to the specified mode.
////////////////////////////////////////////////////////////////////
INLINE BamTextureMode BamWriter::
get_file_texture_mode() const {
  return _file_texture_mode;
}

////////////////////////////////////////////////////////////////////
//     Function: BamWriter::set_file_texture_mode
//       Access: Public
//  Description: Changes the BamTextureMode preference for
//               the Bam file currently being written.  Texture
//               objects written to this Bam file will be encoded
//               according to the specified mode.
////////////////////////////////////////////////////////////////////
INLINE void BamWriter::
set_file_texture_mode(BamTextureMode file_texture_mode) {
  _file_texture_mode = file_texture_mode;
}
