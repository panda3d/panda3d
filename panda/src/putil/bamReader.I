// Filename: bamReader.I
// Created by:  jason (12Jun00)
//


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
//     Function: BamReader::get_factory
//       Access: Public, Static
//  Description: Returns the global WriteableFactory for generating
//               TypedWriteable objects
////////////////////////////////////////////////////////////////////
INLINE WriteableFactory* BamReader::
get_factory() {
  if (_factory == NullFactory)
    create_factory();
  return _factory;
}

////////////////////////////////////////////////////////////////////
//     Function: BamReader::create_factory
//       Access: Private, Static
//  Description: Creates a new WriteableFactory for generating
//               TypedWriteable objects
////////////////////////////////////////////////////////////////////
INLINE void BamReader::
create_factory() {
  _factory = new WriteableFactory;
}

////////////////////////////////////////////////////////////////////
//     Function: parse_params
//       Access: Private, Static
//  Description: Takes in a FactoryParams, passed from a WriteableFactory
//               into any TypedWriteable's make function, and parses
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
