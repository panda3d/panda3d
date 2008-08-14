// Filename: bamReader.I
// Created by:  jason (12Jun00)
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
//     Function: BamReader::get_filename
//       Access: Public
//  Description: If a BAM is a file, then the BamReader should
//               contain the name of the file.  This enables the
//               reader to interpret pathnames in the BAM as relative
//               to the directory containing the BAM.
////////////////////////////////////////////////////////////////////
INLINE const Filename &BamReader::
get_filename() const {
  return _filename;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_loader_options
//       Access: Public
//  Description: Returns the LoaderOptions passed to the loader when
//               the model was requested, if any.
////////////////////////////////////////////////////////////////////
INLINE const LoaderOptions &BamReader::
get_loader_options() const {
  return _loader_options;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::set_loader_options
//       Access: Public
//  Description: Specifies the LoaderOptions for this BamReader.
////////////////////////////////////////////////////////////////////
INLINE void BamReader::
set_loader_options(const LoaderOptions &options) {
  _loader_options = options;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::is_eof
//       Access: Public
//  Description: Returns true if the reader has reached end-of-file,
//               false otherwise.  This call is only valid after a
//               call to read_object().
////////////////////////////////////////////////////////////////////
INLINE bool BamReader::
is_eof() const {
  return _source->is_eof();
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_major_ver
//       Access: Public
//  Description: Returns the major version number of the Bam file
//               currently being read.
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_file_major_ver() const {
  return _file_major;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_minor_ver
//       Access: Public
//  Description: Returns the minor version number of the Bam file
//               currently being read.
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_file_minor_ver() const {
  return _file_minor;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_endian
//       Access: Public
//  Description: Returns the endian preference indicated by the Bam
//               file currently being read.  This does not imply that
//               every number is stored using the indicated
//               convention, but individual objects may choose to
//               respect this flag when recording data.
////////////////////////////////////////////////////////////////////
INLINE BamEndian BamReader::
get_file_endian() const {
  return _file_endian;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_current_major_ver
//       Access: Public
//  Description: Returns the major version number of Bam files
//               supported by the current code base.  This must match
//               get_file_major_ver() in order to successfully read a
//               file.
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_current_major_ver() const {
  return _cur_major;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_current_minor_ver
//       Access: Public
//  Description: Returns the minor version number of Bam files
//               supported by the current code base.  This must match
//               or exceed get_file_minor_ver() in order to
//               successfully read a file.
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_current_minor_ver() const {
  return _cur_minor;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file
//       Access: Public
//  Description: Returns the VirtualFile that provides the source for
//               these datagrams, if any, or NULL if the datagrams do
//               not originate from a VirtualFile.
////////////////////////////////////////////////////////////////////
INLINE VirtualFile *BamReader::
get_file() {
  return _source->get_file();
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_pos
//       Access: Public
//  Description: Returns the current file position within the data
//               stream, if any, or 0 if the file position is not
//               meaningful or cannot be determined.
//
//               For BamReaders that return a meaningful file
//               position, this will be pointing to the first byte
//               following the datagram returned after a call to
//               get_datagram().
////////////////////////////////////////////////////////////////////
INLINE streampos BamReader::
get_file_pos() {
  return _source->get_file_pos();
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_factory
//       Access: Public, Static
//  Description: Returns the global WritableFactory for generating
//               TypedWritable objects
////////////////////////////////////////////////////////////////////
INLINE WritableFactory *BamReader::
get_factory() {
  if (_factory == (WritableFactory *)NULL) {
    create_factory();
  }
  return _factory;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::create_factory
//       Access: Private, Static
//  Description: Creates a new WritableFactory for generating
//               TypedWritable objects
////////////////////////////////////////////////////////////////////
INLINE void BamReader::
create_factory() {
  _factory = new WritableFactory;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_datagram
//       Access: Private
//  Description: Reads a single datagram from the stream.  Returns
//               true on success, false on failure.
////////////////////////////////////////////////////////////////////
INLINE bool BamReader::
get_datagram(Datagram &datagram) {
  if (_source->is_error()) {
    return false;
  }

  return _source->get_datagram(datagram);
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::AuxData::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BamReader::AuxData::
AuxData() {
}

////////////////////////////////////////////////////////////////////
//     Function: parse_params
//       Access: Private, Static
//  Description: Takes in a FactoryParams, passed from a WritableFactory
//               into any TypedWritable's make function, and parses
//               out the datagram that contatins the data it needs to
//               construct itself, and parses out the pointer to the
//               managing BamReader object
////////////////////////////////////////////////////////////////////
INLINE void
parse_params(const FactoryParams &params,
             DatagramIterator &scan, BamReader *&manager) {
  BamReaderParam *param = DCAST(BamReaderParam, params.get_param(0));

  scan = param->get_iterator();
  manager = param->get_manager();
}
