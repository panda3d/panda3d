// Filename: dcFile.cxx
// Created by:  drose (05Oct00)
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

#include "dcFile.h"
#include "dcParserDefs.h"
#include "dcLexerDefs.h"
#include "hashGenerator.h"

#ifdef WITHIN_PANDA
#include "filename.h"
#include "config_express.h"
#include "virtualFileSystem.h"
#endif


////////////////////////////////////////////////////////////////////
//     Function: DCFile::Constructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DCFile::
DCFile() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::Destructor
//       Access: Published
//  Description:
////////////////////////////////////////////////////////////////////
DCFile::
~DCFile() {
  Classes::iterator ci;
  for (ci = _classes.begin(); ci != _classes.end(); ++ci) {
    delete (*ci);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::read
//       Access: Published
//  Description: Opens and reads the indicated .dc file by name.  The
//               distributed classes defined in the file will be
//               appended to the set of distributed classes already
//               recorded, if any.
//
//               Returns true if the file is successfully read, false
//               if there was an error (in which case the file might
//               have been partially read).
////////////////////////////////////////////////////////////////////
bool DCFile::
read(Filename filename) {
  ifstream in;

#ifdef WITHIN_PANDA
  filename.set_text();
  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    istream *in = vfs->open_read_file(filename);
    if (in == (istream *)NULL) {
      cerr << "Cannot open " << filename << " for reading.\n";
      return false;
    }
    bool okflag = read(*in, filename);
    delete in;
    return okflag;
  }
  filename.open_read(in);
#else
  in.open(filename.c_str());
#endif

  if (!in) {
    cerr << "Cannot open " << filename << " for reading.\n";
    return false;
  }

  return read(in, filename);
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::read
//       Access: Published
//  Description: Parses the already-opened input stream for
//               distributed class descriptions.  The filename
//               parameter is optional and is only used when reporting
//               errors.
//
//               The distributed classes defined in the file will be
//               appended to the set of distributed classes already
//               recorded, if any.
//
//               Returns true if the file is successfully read, false
//               if there was an error (in which case the file might
//               have been partially read).
////////////////////////////////////////////////////////////////////
bool DCFile::
read(istream &in, const string &filename) {
  dc_init_parser(in, filename, *this);
  dcyyparse();
  dc_cleanup_parser();

  return (dc_error_count() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::write
//       Access: Published
//  Description: Opens the indicated filename for output and writes a
//               parseable description of all the known distributed
//               classes to the file.
//
//               Returns true if the description is successfully
//               written, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCFile::
write(Filename filename, bool brief) const {
  ofstream out;

#ifdef WITHIN_PANDA
  filename.set_text();
  filename.open_write(out);
#else
  out.open(filename.c_str());
#endif

  if (!out) {
    cerr << "Can't open " << filename << " for output.\n";
    return false;
  }
  return write(out, brief);
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::write
//       Access: Published
//  Description: Writes a parseable description of all the known
//               distributed classes to the stream.
//
//               Returns true if the description is successfully
//               written, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCFile::
write(ostream &out, bool brief) const {
  Classes::const_iterator ci;
  for (ci = _classes.begin(); ci != _classes.end(); ++ci) {
    (*ci)->write(out, brief, 0);
    out << "\n";
  }

  return !out.fail();
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::get_num_classes
//       Access: Published
//  Description: Returns the number of classes read from the .dc
//               file(s).
////////////////////////////////////////////////////////////////////
int DCFile::
get_num_classes() {
  return _classes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::get_class
//       Access: Published
//  Description: Returns the nth class read from the .dc file(s).
////////////////////////////////////////////////////////////////////
DCClass *DCFile::
get_class(int n) {
  nassertr(n >= 0 && n < (int)_classes.size(), NULL);
  return _classes[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::get_class_by_name
//       Access: Published
//  Description: Returns the class that has the indicated name, or
//               NULL if there is no such class.
////////////////////////////////////////////////////////////////////
DCClass *DCFile::
get_class_by_name(const string &name) {
  ClassesByName::const_iterator ni;
  ni = _classes_by_name.find(name);
  if (ni != _classes_by_name.end()) {
    return (*ni).second;
  }

  return (DCClass *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::get_hash
//       Access: Published
//  Description: Returns a 32-bit hash index associated with this
//               file.  This number is guaranteed to be consistent if
//               the contents of the file have not changed, and it is
//               very likely to be different if the contents of the
//               file do change.
////////////////////////////////////////////////////////////////////
unsigned long DCFile::
get_hash() const {
  HashGenerator hashgen;
  generate_hash(hashgen);
  return hashgen.get_hash();
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::generate_hash
//       Access: Public, Virtual
//  Description: Accumulates the properties of this file into the
//               hash.
////////////////////////////////////////////////////////////////////
void DCFile::
generate_hash(HashGenerator &hashgen) const {
  hashgen.add_int(_classes.size());
  Classes::const_iterator ci;
  for (ci = _classes.begin(); ci != _classes.end(); ++ci) {
    (*ci)->generate_hash(hashgen);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::add_class
//       Access: Public
//  Description: Adds the newly-allocated distributed class definition
//               to the file.  The DCFile becomes the owner of the
//               pointer and will delete it when it destructs.
//               Returns true if the class is successfully added, or
//               false if there was a name conflict.
////////////////////////////////////////////////////////////////////
bool DCFile::
add_class(DCClass *dclass) {
  bool inserted = _classes_by_name.insert
    (ClassesByName::value_type(dclass->_name, dclass)).second;

  if (!inserted) {
    return false;
  }

  dclass->_number = get_num_classes();
  _classes.push_back(dclass);
  return true;
}
