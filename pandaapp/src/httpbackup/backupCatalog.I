// Filename: backupCatalog.I
// Created by:  drose (29Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::Entry::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE BackupCatalog::Entry::
Entry() {
}

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::Entry::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE bool BackupCatalog::Entry::
operator < (const BackupCatalog::Entry &other) const {
  return get_date() < other.get_date();
}

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::Entry::get_date
//       Access: Public
//  Description: Returns the date associated with the entry.  This is
//               the last-modified date provided by the server, if we
//               have such a thing; or the downloaded date if that's
//               all we have.
////////////////////////////////////////////////////////////////////
INLINE const HTTPDate &BackupCatalog::Entry::
get_date() const {
  return _document_spec.has_date() ? _document_spec.get_date() : _download_date;
}


INLINE istream &
operator >> (istream &in, BackupCatalog::Entry &entry) {
  entry.input(in);
  return in;
}

INLINE ostream &
operator << (ostream &out, const BackupCatalog::Entry &entry) {
  entry.output(out);
  return out;
}
