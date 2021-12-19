/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file xFile.cxx
 * @author drose
 * @date 2004-10-03
 */

#include "xFile.h"
#include "xParserDefs.h"
#include "xLexerDefs.h"
#include "xFileTemplate.h"
#include "xFileDataNodeTemplate.h"
#include "config_xfile.h"
#include "standard_templates.h"
#include "zStream.h"
#include "virtualFileSystem.h"
#include "dcast.h"

using std::istream;
using std::istringstream;
using std::ostream;
using std::string;

TypeHandle XFile::_type_handle;
PT(XFile) XFile::_standard_templates;

/**
 *
 */
XFile::
XFile(bool keep_names) : XFileNode(this) {
  _major_version = 3;
  _minor_version = 2;
  _format_type = FT_text;
  _float_size = FS_64;
  _keep_names = keep_names;
}

/**
 *
 */
XFile::
~XFile() {
  clear();
}

/**
 * Removes all of the classes defined within the XFile and prepares it for
 * reading a new file.
 */
void XFile::
clear() {
  XFileNode::clear();

  _nodes_by_guid.clear();
}

/**
 * Opens and reads the indicated .x file by name.  The nodes and templates
 * defined in the file will be appended to the set of nodes already recorded,
 * if any.
 *
 * Returns true if the file is successfully read, false if there was an error
 * (in which case the file might have been partially read).
 */
bool XFile::
read(Filename filename) {
  filename.set_text();
  VirtualFileSystem *vfs = VirtualFileSystem::get_global_ptr();
  istream *in = vfs->open_read_file(filename, true);
  if (in == nullptr) {
    xfile_cat.error()
      << "Cannot open " << filename << " for reading.\n";
    return false;
  }
  bool okflag = read(*in, filename);
  vfs->close_read_file(in);
  return okflag;
}

/**
 * Parses the already-opened input stream for distributed class descriptions.
 * The filename parameter is optional and is only used when reporting errors.
 *
 * The distributed classes defined in the file will be appended to the set of
 * distributed classes already recorded, if any.
 *
 * Returns true if the file is successfully read, false if there was an error
 * (in which case the file might have been partially read).
 */
bool XFile::
read(istream &in, const string &filename) {
  if (!read_header(in)) {
    return false;
  }

  if (_format_type != FT_text) {
    // Does anyone actually use the binary format?  It wouldn't be too hard to
    // support it if there were any reason at all to do so.
    xfile_cat.error()
      << "Cannot read binary .x files at this time.\n";
    return false;
  }

  // We must call this first so the standard templates file will be parsed and
  // available by the time we need it--it's tricky to invoke the parser from
  // within another parser instance.
  get_standard_templates();

  x_init_parser(in, filename, *this);
  xyyparse();
  x_cleanup_parser();

  return (x_error_count() == 0);
}

/**
 * Opens the indicated filename for output and writes a parseable description
 * of all the known distributed classes to the file.
 *
 * Returns true if the description is successfully written, false otherwise.
 */
bool XFile::
write(Filename filename) const {
  std::ofstream out;

  // We actually open the file to write in binary mode, to avoid the MS-DOS
  // newline characters (since Windows seems to do this too).
  filename.set_binary();
  filename.open_write(out);

  if (!out) {
    xfile_cat.error()
      << "Can't open " << filename << " for output.\n";
    return false;
  }

#ifdef HAVE_ZLIB
  if (filename.get_extension() == "pz") {
    // The filename ends in .pz, which means to automatically compress the X
    // file that we write.
    OCompressStream compressor(&out, false);
    return write(compressor);
  }
#endif  // HAVE_ZLIB

  return write(out);
}

/**
 * Writes a parseable description of all the known nodes and templates to the
 * stream.
 *
 * Returns true if the description is successfully written, false otherwise.
 */
bool XFile::
write(ostream &out) const {
  if (!write_header(out)) {
    return false;
  }

  write_text(out, 0);

  return true;
}

/**
 * Returns the template associated with the indicated name, if any, or NULL if
 * none.
 */
XFileTemplate *XFile::
find_template(const string &name) const {
  XFileTemplate *standard = nullptr;
  const XFile *standard_templates = get_standard_templates();
  if (standard_templates != this) {
    standard = standard_templates->find_template(name);
  }

  XFileNode *child = find_child(name);
  if (child != nullptr &&
      child->is_of_type(XFileTemplate::get_class_type())) {
    XFileTemplate *xtemplate = DCAST(XFileTemplate, child);
    if (standard != nullptr && xtemplate->matches(standard)) {
      // If the template matches a standard template, return the standard
      // instead.  The assumption is that code may expect a certain naming
      // scheme for the data elements of the standard template, so we want to
      // be sure to provide it.
      return standard;
    }
    return xtemplate;
  }

  return standard;
}

/**
 * Returns the template associated with the indicated GUID, if any, or NULL if
 * none.
 */
XFileTemplate *XFile::
find_template(const WindowsGuid &guid) const {
  XFileTemplate *standard = nullptr;
  const XFile *standard_templates = get_standard_templates();
  if (standard_templates != this) {
    standard = standard_templates->find_template(guid);
  }

  NodesByGuid::const_iterator gi;
  gi = _nodes_by_guid.find(guid);
  if (gi != _nodes_by_guid.end() &&
      (*gi).second->is_of_type(XFileTemplate::get_class_type())) {
    XFileTemplate *xtemplate = DCAST(XFileTemplate, (*gi).second);
    if (standard != nullptr && xtemplate->matches(standard)) {
      // If the template matches a standard template, return the standard
      // instead.  The assumption is that code may expect a certain naming
      // scheme for the data elements of the standard template, so we want to
      // be sure to provide it.
      return standard;
    }
    return xtemplate;
  }

  return standard;
}

/**
 * Returns the standard template associated with the indicated name, if any,
 * or NULL if none.
 */
XFileTemplate *XFile::
find_standard_template(const string &name) {
  const XFile *standard_templates = get_standard_templates();
  return standard_templates->find_template(name);
}

/**
 * Returns the template associated with the indicated GUID, if any, or NULL if
 * none.
 */
XFileTemplate *XFile::
find_standard_template(const WindowsGuid &guid) {
  const XFile *standard_templates = get_standard_templates();
  return standard_templates->find_template(guid);
}

/**
 * Returns the data object associated with the indicated name, if any, or NULL
 * if none.
 */
XFileDataNodeTemplate *XFile::
find_data_object(const string &name) const {
  XFileNode *child = find_descendent(name);
  if (child != nullptr &&
      child->is_of_type(XFileDataNodeTemplate::get_class_type())) {
    return DCAST(XFileDataNodeTemplate, child);
  }

  return nullptr;
}

/**
 * Returns the data object associated with the indicated GUID, if any, or NULL
 * if none.
 */
XFileDataNodeTemplate *XFile::
find_data_object(const WindowsGuid &guid) const {
  NodesByGuid::const_iterator gi;
  gi = _nodes_by_guid.find(guid);
  if (gi != _nodes_by_guid.end() &&
      (*gi).second->is_of_type(XFileDataNodeTemplate::get_class_type())) {
    return DCAST(XFileDataNodeTemplate, (*gi).second);
  }

  return nullptr;
}

/**
 * Writes a suitable representation of this node to an .x file in text mode.
 */
void XFile::
write_text(ostream &out, int indent_level) const {
  Children::const_iterator ci;
  for (ci = _children.begin(); ci != _children.end(); ++ci) {
    (*ci)->write_text(out, indent_level);
    out << "\n";
  }
}

/**
 * Reads the header and magic number associated with the file.  Returns true
 * on success, false otherwise.
 */
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

/**
 * Writes the header and magic number associated with the file.  Returns true
 * on success, false otherwise.
 */
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

/**
 * Returns a global XFile object that contains the standard list of Direct3D
 * template definitions that may be assumed to be at the head of every file.
 */
const XFile *XFile::
get_standard_templates() {
  if (_standard_templates == nullptr) {
    // The standardTemplates.x file has been compiled into this binary.
    // Extract it out.

    string data((const char *)standard_templates_data, standard_templates_data_len);

#ifdef HAVE_ZLIB
    // The data is stored compressed; decompress it on-the-fly.
    istringstream inz(data);
    IDecompressStream in(&inz, false);

#else
    // The data is stored uncompressed, so just load it.
    istringstream in(data);
#endif  // HAVE_ZLIB

    _standard_templates = new XFile;
    if (!_standard_templates->read(in, "standardTemplates.x")) {
      xfile_cat.error()
        << "Internal error: Unable to parse built-in standardTemplates.x!\n";
    }

    // Now flag all of these templates as "standard".
    for (int i = 0; i < _standard_templates->get_num_children(); i++) {
      XFileNode *child = _standard_templates->get_child(i);
      if (child->is_of_type(XFileTemplate::get_class_type())) {
        XFileTemplate *xtemplate = DCAST(XFileTemplate, child);
        xtemplate->_is_standard = true;
      }
    }
  }

  return _standard_templates;
}
