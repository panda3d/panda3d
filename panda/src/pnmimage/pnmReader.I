// Filename: pnmReader.I
// Created by:  drose (16Jun00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: PNMReader::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE PNMReader::
PNMReader(PNMFileType *type, FILE *file, bool owns_file) :
  _type(type),
  _file(file),
  _owns_file(owns_file),
  _is_valid(true)
{
}

////////////////////////////////////////////////////////////////////
//     Function: PNMReader::get_type
//       Access: Public
//  Description: Returns a pointer to the PNMFileType object that
//               created this PNMReader.
////////////////////////////////////////////////////////////////////
INLINE PNMFileType *PNMReader::
get_type() const {
  return _type;
}

////////////////////////////////////////////////////////////////////
//     Function: PNMReader::is_valid
//       Access: Public
//  Description: Returns true if the PNMReader can be used to read
//               data, false if something is wrong.
////////////////////////////////////////////////////////////////////
INLINE bool PNMReader::
is_valid() const {
  return _is_valid;
}
