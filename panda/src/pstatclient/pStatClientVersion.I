// Filename: pStatClientVersion.I
// Created by:  drose (21May01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PStatClientVersion::get_major_version
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int PStatClientVersion::
get_major_version() const {
  return _major_version;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientVersion::get_minor_version
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE int PStatClientVersion::
get_minor_version() const {
  return _minor_version;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientVersion::set_version
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void PStatClientVersion::
set_version(int major, int minor) {
  _major_version = major;
  _minor_version = minor;
}

////////////////////////////////////////////////////////////////////
//     Function: PStatClientVersion::is_at_least
//       Access: Public
//  Description: Returns true if the client version is at least the
//               indicated major/minor version number, false
//               otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool PStatClientVersion::
is_at_least(int major, int minor) const {
  return (_major_version > major ||
          (_major_version == major && _minor_version >= minor));
}
