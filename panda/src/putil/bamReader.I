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
//     Function: BamReaderAuxData::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BamReaderAuxData::
BamReaderAuxData() {
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_source
//       Access: Published
//  Description: Returns the current source of the BamReader as set by
//               set_source() or the constructor.
////////////////////////////////////////////////////////////////////
INLINE DatagramGenerator *BamReader::
get_source() {
  return _source;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_filename
//       Access: Published
//  Description: If a BAM is a file, then the BamReader should
//               contain the name of the file.  This enables the
//               reader to interpret pathnames in the BAM as relative
//               to the directory containing the BAM.
////////////////////////////////////////////////////////////////////
INLINE const Filename &BamReader::
get_filename() const {
  if (_source != (DatagramGenerator *)NULL) {
    return _source->get_filename();
  }
  static const Filename empty_filename;
  return empty_filename;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_loader_options
//       Access: Published
//  Description: Returns the LoaderOptions passed to the loader when
//               the model was requested, if any.
////////////////////////////////////////////////////////////////////
INLINE const LoaderOptions &BamReader::
get_loader_options() const {
  return _loader_options;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::set_loader_options
//       Access: Published
//  Description: Specifies the LoaderOptions for this BamReader.
////////////////////////////////////////////////////////////////////
INLINE void BamReader::
set_loader_options(const LoaderOptions &options) {
  _loader_options = options;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::is_eof
//       Access: Published
//  Description: Returns true if the reader has reached end-of-file,
//               false otherwise.  This call is only valid after a
//               call to read_object().
////////////////////////////////////////////////////////////////////
INLINE bool BamReader::
is_eof() const {
  nassertr(_source != NULL, true);
  return _source->is_eof();
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_major_ver
//       Access: Published
//  Description: Returns the major version number of the Bam file
//               currently being read.
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_file_major_ver() const {
  return _file_major;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_minor_ver
//       Access: Published
//  Description: Returns the minor version number of the Bam file
//               currently being read.
////////////////////////////////////////////////////////////////////
INLINE int BamReader::
get_file_minor_ver() const {
  return _file_minor;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_endian
//       Access: Published
//  Description: Returns the endian preference indicated by the Bam
//               file currently being read.  This does not imply that
//               every number is stored using the indicated
//               convention, but individual objects may choose to
//               respect this flag when recording data.
////////////////////////////////////////////////////////////////////
INLINE BamReader::BamEndian BamReader::
get_file_endian() const {
  return _file_endian;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_file_stdfloat_double
//       Access: Published
//  Description: Returns true if the file stores all "standard"
//               floats as 64-bit doubles, or false if they are 32-bit
//               floats.  This is determined by the compilation flags
//               of the version of Panda that generated this file.
////////////////////////////////////////////////////////////////////
INLINE bool BamReader::
get_file_stdfloat_double() const {
  return _file_stdfloat_double;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_current_major_ver
//       Access: Published
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
//       Access: Published
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
//  Description: Returns the FileReference that provides the source for
//               these datagrams, if any, or NULL if the datagrams do
//               not originate from a file on disk.
////////////////////////////////////////////////////////////////////
INLINE const FileReference *BamReader::
get_file() {
  nassertr(_source != NULL, NULL);
  return _source->get_file();
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::get_vfile
//       Access: Public
//  Description: Returns the VirtualFile that provides the source for
//               these datagrams, if any, or NULL if the datagrams do
//               not originate from a VirtualFile.
////////////////////////////////////////////////////////////////////
INLINE VirtualFile *BamReader::
get_vfile() {
  nassertr(_source != NULL, NULL);
  return _source->get_vfile();
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
  nassertr(_source != NULL, 0);
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
  nassertr(_source != NULL, false);
  if (_source->is_error()) {
    return false;
  }

  if (!_source->get_datagram(datagram)) {
    return false;
  }

  datagram.set_stdfloat_double(_file_stdfloat_double);
  return true;
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
//     Function: BamReader::CreatedObj::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BamReader::CreatedObj::
CreatedObj() :
  _created(false),
  _ptr(NULL),
  _ref_ptr(NULL),
  _change_this(NULL),
  _change_this_ref(NULL)
{
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::CreatedObj::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BamReader::CreatedObj::
~CreatedObj() {
  set_ptr(NULL, NULL);
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::CreatedObj::set_ptr
//       Access: Public
//  Description: Replaces the pointer to the created object.  There
//               are actually two pointers to the same object in
//               different forms: a generic TypedWritable pointer, and
//               an untyped ReferenceCount pointer.  We need both
//               pointers because some objects (like PandaNode)
//               inherit from TypedWritable and ReferenceCount
//               independently.
//
//               Managing a typed pointer and an untyped
//               ReferenceCount pointer to the same object takes just
//               a bit of extra care.
////////////////////////////////////////////////////////////////////
INLINE void BamReader::CreatedObj::
set_ptr(TypedWritable *ptr, ReferenceCount *ref_ptr) {
  if (_ptr != ptr) {
    if (_ref_ptr != NULL) {
      nassertv(_ref_ptr != ref_ptr);
      unref_delete(_ref_ptr);
    }
    
    _ptr = ptr;
    _ref_ptr = ref_ptr;
    
    if (_ref_ptr != NULL) {
      _ref_ptr->ref();
    }
  } else {
    nassertv(_ref_ptr == ref_ptr);
  }
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

