// Filename: xFile.cxx
// Created by:  drose (03Oct04)
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

#include "xFile.h"
#include "xParserDefs.h"
#include "xLexerDefs.h"
#include "config_xfile.h"
#include "config_express.h"
#include "virtualFileSystem.h"

TypeHandle XFile::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: XFile::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFile::
XFile() : XFileNode("xfile") {
  _major_version = 1;
  _minor_version = 1;
  _format_type = FT_text;
  _float_size = FS_64;
}

////////////////////////////////////////////////////////////////////
//     Function: XFile::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
XFile::
~XFile() {
  clear();
}

////////////////////////////////////////////////////////////////////
//     Function: XFile::add_child
//       Access: Public, Virtual
//  Description: Adds the indicated node as a child of this node.
////////////////////////////////////////////////////////////////////
void XFile::
add_child(XFileNode *node) {
  XFileNode::add_child(node);
}

////////////////////////////////////////////////////////////////////
//     Function: XFile::clear
//       Access: Public, Virtual
//  Description: Removes all of the classes defined within the XFile
//               and prepares it for reading a new file.
////////////////////////////////////////////////////////////////////
void XFile::
clear() {
  XFileNode::clear();
}

////////////////////////////////////////////////////////////////////
//     Function: XFile::read
//       Access: Public
//  Description: Opens and reads the indicated .x file by name.  The
//               nodes and templates defined in the file will be
//               appended to the set of nodes already recorded, if
//               any.
//
//               Returns true if the file is successfully read, false
//               if there was an error (in which case the file might
//               have been partially read).
////////////////////////////////////////////////////////////////////
bool XFile::
read(Filename filename) {
  ifstream in;

  filename.set_text();
  if (use_vfs) {
    VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
    istream *in = vfs->open_read_file(filename);
    if (in == (istream *)NULL) {
      xfile_cat.error()
        << "Cannot open " << filename << " for reading.\n";
      return false;
    }
    bool okflag = read(*in, filename);
    delete in;
    return okflag;

  } else {
    filename.open_read(in);

    if (!in) {
      xfile_cat.error()
        << "Cannot open " << filename << " for reading.\n";
      return false;
    }
    
    return read(in, filename);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFile::read
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
bool XFile::
read(istream &in, const string &filename) {
  if (!read_header(in)) {
    return false;
  }

  if (_format_type != FT_text) {
    // Does anyone actually use the binary format?  It wouldn't be too
    // hard to support it if there were any reason at all to do so.
    xfile_cat.error()
      << "Cannot read binary .x files at this time.\n";
    return false;
  }

  x_init_parser(in, filename, *this);
  xyyparse();
  x_cleanup_parser();

  return (x_error_count() == 0);
}

////////////////////////////////////////////////////////////////////
//     Function: XFile::write
//       Access: Public
//  Description: Opens the indicated filename for output and writes a
//               parseable description of all the known distributed
//               classes to the file.
//
//               Returns true if the description is successfully
//               written, false otherwise.
////////////////////////////////////////////////////////////////////
bool XFile::
write(Filename filename) const {
  ofstream out;

  // We actually open the file to write in binary mode, to avoid the
  // MS-DOS newline characters (since Windows seems to do this too).
  filename.set_binary();
  filename.open_write(out);

  if (!out) {
    xfile_cat.error()
      << "Can't open " << filename << " for output.\n";
    return false;
  }

  return write(out);
}

////////////////////////////////////////////////////////////////////
//     function: XFile::write
//       Access: Public
//  Description: Writes a parseable description of all the known
//               nodes and templates to the stream.
//
//               Returns true if the description is successfully
//               written, false otherwise.
////////////////////////////////////////////////////////////////////
bool XFile::
write(ostream &out) const {
  if (!write_header(out)) {
    return false;
  }

  write_text(out, 0);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFile::write_text
//       Access: Public, Virtual
//  Description: Writes a suitable representation of this node to an
//               .x file in text mode.
////////////////////////////////////////////////////////////////////
void XFile::
write_text(ostream &out, int indent_level) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write_text(out, indent_level);
    out << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: XFile::read_header
//       Access: Private
//  Description: Reads the header and magic number associated with the
//               file.  Returns true on success, false otherwise.
////////////////////////////////////////////////////////////////////
bool XFile::
read_header(istream &in) {
  char magic[4];
  if (!in.read(magic, 4)) {
    xfile_cat.error()
      << "Empty file.\n";
    return false;
  }

  if (memcmp(magic, "xof ", 4) != 0) {
    xfile_cat.error()
      << "Not a DirectX file.\n";
    return false;
  }

  char version[4];
  if (!in.read(version, 4)) {
    xfile_cat.error()
      << "Truncated file.\n";
    return false;
  }
  _major_version = (version[0] - '0') * 10 + (version[1] - '0');
  _minor_version = (version[2] - '0') * 10 + (version[3] - '0');

  char format[4];
  if (!in.read(format, 4)) {
    xfile_cat.error()
      << "Truncated file.\n";
    return false;
  }

  if (memcmp(format, "txt ", 4) == 0) {
    _format_type = FT_text;

  } else if (memcmp(format, "bin ", 4) == 0) {
    _format_type = FT_binary;

  } else if (memcmp(format, "com ", 4) == 0) {
    _format_type = FT_compressed;

  } else {
    xfile_cat.error()
      << "Unknown format type: " << string(format, 4) << "\n";
    return false;
  }

  if (_format_type == FT_compressed) {
    // Read and ignore the compression type, since we don't support
    // compression anyway.
    char compression_type[4];
    in.read(compression_type, 4);
  }

  char float_size[4];
  if (!in.read(float_size, 4)) {
    xfile_cat.error()
      << "Truncated file.\n";
    return false;
  }

  if (memcmp(float_size, "0032", 4) == 0) {
    _float_size = FS_32;

  } else if (memcmp(float_size, "0064", 4) == 0) {
    _float_size = FS_64;

  } else {
    xfile_cat.error()
      << "Unknown float size: " << string(float_size, 4) << "\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: XFile::write_header
//       Access: Private
//  Description: Writes the header and magic number associated with the
//               file.  Returns true on success, false otherwise.
////////////////////////////////////////////////////////////////////
bool XFile::
write_header(ostream &out) const {
  out.write("xof ", 4);

  char buffer[128];
  sprintf(buffer, "%02d%02d", _major_version, _minor_version);
  if (strlen(buffer) != 4) {
    xfile_cat.error()
      << "Invalid version: " << _major_version << "." << _minor_version
      << "\n";
    return false;
  }

  out.write(buffer, 4);

  switch (_format_type) {
  case FT_text:
    out.write("txt ", 4);
    break;
 
  case FT_binary:
    out.write("bin ", 4);
    break;
  
  case FT_compressed:
    out.write("cmp ", 4);
    break;

  default:
    xfile_cat.error()
      << "Invalid format type: " << _format_type << "\n";
    return false;
  }

  if (_format_type == FT_compressed) {
    // Write a bogus compression type, just so we have a valid header.
    out.write("xxx ", 4);
  }

  switch (_float_size) {
  case FS_32:
    out.write("0032", 4);
    break;
 
  case FS_64:
    out.write("0064", 4);
    break;

  default:
    xfile_cat.error()
      << "Invalid float size: " << _float_size << "\n";
    return false;
  }

  if (_format_type == FT_text) {
    // If it's a text format, we can now write a newline.
    out << "\n";
  }

  return true;
}
