// Filename: dcFile.cxx
// Created by:  drose (05Oct00)
// 
////////////////////////////////////////////////////////////////////

#include "dcFile.h"
#include "dcParserDefs.h"
#include "dcLexerDefs.h"

#ifdef WITHIN_PANDA
#include <filename.h>
#endif


////////////////////////////////////////////////////////////////////
//     Function: DCFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
DCFile::
DCFile() {
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::Destructor
//       Access: Public
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
//       Access: Public
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
//       Access: Public
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
//       Access: Public
//  Description: Opens the indicated filename for output and writes a
//               parseable description of all the known distributed
//               classes to the file.
//
//               Returns true if the description is successfully
//               written, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCFile::
write(Filename filename) const {
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
  return write(out, filename);
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::write
//       Access: Public
//  Description: Writes a parseable description of all the known
//               distributed classes to the file.  The filename
//               parameter is optional and is only used when reporting
//               errors.
//
//               Returns true if the description is successfully
//               written, false otherwise.
////////////////////////////////////////////////////////////////////
bool DCFile::
write(ostream &out, const string &filename) const {
  Classes::const_iterator ci;
  for (ci = _classes.begin(); ci != _classes.end(); ++ci) {
    (*ci)->write(out);
    out << "\n";
  }

  if (out.fail()) {
    cerr << "I/O error writing " << filename << ".\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::get_num_classes
//       Access: Public
//  Description: Returns the number of classes read from the .dc
//               file(s).
////////////////////////////////////////////////////////////////////
int DCFile::
get_num_classes() {
  return _classes.size();
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::get_class
//       Access: Public
//  Description: Returns the nth class read from the .dc file(s).
////////////////////////////////////////////////////////////////////
DCClass *DCFile::
get_class(int n) {
  nassertr(n >= 0 && n < (int)_classes.size(), NULL);
  return _classes[n];
}

////////////////////////////////////////////////////////////////////
//     Function: DCFile::get_class_by_name
//       Access: Public
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
