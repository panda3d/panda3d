// Filename: backupCatalog.cxx
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

#include "backupCatalog.h"
#include "indirectLess.h"
#include <algorithm>

// These are the characters that are not escaped when we write a
// filename out to the catalog.  Really, the only reason we use
// URLSpec::quote() to protect the filenames is to escape spaces.
static const char * const acceptable_chars = "~/:";

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BackupCatalog::
BackupCatalog() {
  _dirty = false;
}

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
BackupCatalog::
~BackupCatalog() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::read
//       Access: Public
//  Description: Reads the catalog from the named file.  Returns true
//               on success, false on failure.  On a false return, the
//               catalog may have been partially read in.
////////////////////////////////////////////////////////////////////
bool BackupCatalog::
read(const Filename &filename) {
  clear();

  ifstream file;
  if (!filename.open_read(file)) {
    nout << "Unable to read: " << filename << "\n";
    return false;
  }

  Entry *entry = new Entry;
  file >> (*entry);
  while (!file.fail() && !file.eof()) {
    _table[entry->_document_name].push_back(entry);
    _filenames.insert(entry->_filename);
    entry = new Entry;
    file >> (*entry);
  }

  // Delete the last Entry that we didn't use.
  delete entry;

  if (!file.eof()) {
    // Oops, we had an error on one of the entries.
    return false;
  }

  // Now sort all of the entries by date.
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    Entries &entries = (*ti).second;
    sort(entries.begin(), entries.end(), IndirectLess<Entry>());
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::write
//       Access: Public
//  Description: Rewrites the catalog to the named file.  Returns true
//               on success, false on failure.
////////////////////////////////////////////////////////////////////
bool BackupCatalog::
write(const Filename &filename) {
  ofstream file;
  if (!filename.open_write(file)) {
    nout << "Unable to write: " << filename << "\n";
    return false;
  }

  Table::const_iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    const Entries &entries = (*ti).second;
    Entries::const_iterator ei;
    for (ei = entries.begin(); ei != entries.end(); ++ei) {
      (*ei)->write(file);
      file << "\n";
    }
  }

  if (file) {
    _dirty = false;
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::clear
//       Access: Public
//  Description: Completely empties the contents of the catalog.
////////////////////////////////////////////////////////////////////
void BackupCatalog::
clear() {
  Table::iterator ti;
  for (ti = _table.begin(); ti != _table.end(); ++ti) {
    Entries &entries = (*ti).second;
    Entries::iterator ei;
    for (ei = entries.begin(); ei != entries.end(); ++ei) {
      delete (*ei);
    }
  }
  _dirty = false;
}

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::Entry::delete_file
//       Access: Public
//  Description: Deletes the file named by the entry, echoing the
//               indicated reason to nout.
////////////////////////////////////////////////////////////////////
void BackupCatalog::Entry::
delete_file(const Filename &dirname, const string &reason) {
  Filename pathname(dirname, _filename);
  if (pathname.exists()) {
    nout << "Deleting " << _filename << " (" << reason << ").\n";
    if (!pathname.unlink()) {
      nout << "unable to delete.\n";
    }

  } else {
    nout << "Tried to delete " << _filename << " but it's already gone!\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::Entry::input
//       Access: Public
//  Description: Can be used to read in the catalog entry from a
//               stream generated by either output() or write().
////////////////////////////////////////////////////////////////////
void BackupCatalog::Entry::
input(istream &in) {
  in >> _document_name >> _filename >> _document_spec >> _download_date;
  _document_name = URLSpec::unquote(_document_name);
  _filename = URLSpec::unquote(_filename);
}

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::Entry::output
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BackupCatalog::Entry::
output(ostream &out) const {
  out << URLSpec::quote(_document_name, acceptable_chars) << " " 
      << URLSpec::quote(_filename, acceptable_chars) << " " 
      << _document_spec << " " 
      << _download_date;
}

////////////////////////////////////////////////////////////////////
//     Function: BackupCatalog::Entry::write
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void BackupCatalog::Entry::
write(ostream &out) const {
  out << URLSpec::quote(_document_name, acceptable_chars) << " " 
      << URLSpec::quote(_filename, acceptable_chars) << "\n";
  _document_spec.write(out, 2);
  out << "  " << _download_date << "\n";
}
