// Filename: p3dHost.cxx
// Created by:  drose (21Aug09)
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

#include "p3dHost.h"
#include "p3dInstanceManager.h"
#include "p3dPackage.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::Constructor
//       Access: Private
//  Description: Use P3DInstanceManager::get_host() to construct a new
//               P3DHost.
////////////////////////////////////////////////////////////////////
P3DHost::
P3DHost(P3DInstanceManager *inst_mgr, const string &host_url) :
  _host_url(host_url) 
{
  _host_dir = inst_mgr->get_root_dir();
  _host_dir += "/host";  // TODO.

  // Ensure that the download URL ends with a slash.
  _host_url_prefix = _host_url;
  if (!_host_url_prefix.empty() && _host_url_prefix[_host_url_prefix.size() - 1] != '/') {
    _host_url_prefix += "/";
  }

  _xcontents = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::Destructor
//       Access: Private
//  Description: 
////////////////////////////////////////////////////////////////////
P3DHost::
~P3DHost() {
  if (_xcontents != NULL) {
    delete _xcontents;
  }

  Packages::iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    delete (*pi).second;
  }
  _packages.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::read_contents_file
//       Access: Public
//  Description: Reads the contents.xml file in the indicated
//               filename.  On success, copies the contents.xml file
//               into the standard location (if it's not there
//               already).
//
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DHost::
read_contents_file(const string &contents_filename) {
  TiXmlDocument doc(contents_filename.c_str());
  if (!doc.LoadFile()) {
    return false;
  }

  TiXmlElement *xcontents = doc.FirstChildElement("contents");
  if (xcontents == NULL) {
    return false;
  }

  if (_xcontents != NULL) {
    delete _xcontents;
  }
  _xcontents = (TiXmlElement *)xcontents->Clone();

  string standard_filename = _host_dir + "/contents.xml";
  if (standardize_filename(standard_filename) != 
      standardize_filename(contents_filename)) {
    copy_file(contents_filename, standard_filename);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::get_package
//       Access: Public
//  Description: Returns a (possibly shared) pointer to the indicated
//               package.
////////////////////////////////////////////////////////////////////
P3DPackage *P3DHost::
get_package(const string &package_name, const string &package_version) {
  string key = package_name + "_" + package_version;
  Packages::iterator pi = _packages.find(key);
  if (pi != _packages.end()) {
    return (*pi).second;
  }

  P3DPackage *package = 
    new P3DPackage(this, package_name, package_version);
  bool inserted = _packages.insert(Packages::value_type(key, package)).second;
  assert(inserted);

  return package;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::get_package_desc_file
//       Access: Public
//  Description: Fills the indicated FileSpec with the hash
//               information for the package's desc file, and also
//               determines the package's platform.  Returns true if
//               successful, false if the package is unknown.  This
//               requires has_contents_file() to return true in order
//               to be successful.
////////////////////////////////////////////////////////////////////
bool P3DHost::
get_package_desc_file(FileSpec &desc_file,              // out
                      string &package_platform,         // out
                      const string &package_name,       // in
                      const string &package_version) {  // in
  if (_xcontents == NULL) {
    return false;
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  // Scan the contents data for the indicated package.  First, we look
  // for a platform-specific version.
  TiXmlElement *xpackage = _xcontents->FirstChildElement("package");
  while (xpackage != NULL) {
    const char *name = xpackage->Attribute("name");
    const char *platform = xpackage->Attribute("platform");
    const char *version = xpackage->Attribute("version");
    if (name != NULL && platform != NULL && version != NULL &&
        package_name == name && 
        inst_mgr->get_platform() == platform &&
        package_version == version) {
      // Here's the matching package definition.
      desc_file.load_xml(xpackage);
      package_platform = platform;
      return true;
    }

    xpackage = xpackage->NextSiblingElement("package");
  }

  // Look again, this time looking for a non-platform-specific version.
  xpackage = _xcontents->FirstChildElement("package");
  while (xpackage != NULL) {
    const char *name = xpackage->Attribute("name");
    const char *platform = xpackage->Attribute("platform");
    const char *version = xpackage->Attribute("version");
    if (platform == NULL) {
      platform = "";
    }
    if (name != NULL && version != NULL &&
        package_name == name && 
        *platform == '\0' &&
        package_version == version) {
      // Here's the matching package definition.
      desc_file.load_xml(xpackage);
      package_platform = platform;
      return true;
    }

    xpackage = xpackage->NextSiblingElement("package");
  }

  // Couldn't find the named package.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::standardize_filename
//       Access: Private, Static
//  Description: Attempts to change the filename into some standard
//               form for comparison with other filenames.  On a
//               case-insensitive filesystem, this converts the
//               filename to lowercase.  On Windows, it further
//               replaces forward slashes with backslashes.
////////////////////////////////////////////////////////////////////
string P3DHost::
standardize_filename(const string &filename) {
#if defined(_WIN32) || defined(__APPLE__)
  string new_filename;
  for (string::const_iterator si = filename.begin();
       si != filename.end();
       ++si) {
    char ch = *si;
#ifdef _WIN32
    if (ch == '/') {
      ch = '\\';
    }
#endif  // _WIN32
    new_filename += tolower(ch);
  }
  return new_filename;
#else  // _WIN32 || __APPLE__
  return filename;
#endif  // _WIN32 || __APPLE__
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::copy_file
//       Access: Private, Static
//  Description: Copies the data in the file named by from_filename
//               into the file named by to_filename.
////////////////////////////////////////////////////////////////////
bool P3DHost::
copy_file(const string &from_filename, const string &to_filename) {
  ifstream in(from_filename.c_str(), ios::in | ios::binary);

  // Copy to a temporary file first, in case (a) the filenames
  // actually refer to the same file, or (b) in case we have different
  // processes writing to the same file, and (c) to prevent
  // partially overwriting the file should something go wrong.
  ostringstream strm;
  strm << to_filename << ".t";
#ifdef _WIN32
  strm << GetCurrentProcessId() << "_" << GetCurrentThreadId();
#else
  strm << getpid();
#endif
  string temp_filename = strm.str();
  ofstream out(temp_filename.c_str(), ios::out | ios::binary);
        
  static const size_t buffer_size = 4096;
  char buffer[buffer_size];
  
  in.read(buffer, buffer_size);
  size_t count = in.gcount();
  while (count != 0) {
    out.write(buffer, count);
    if (out.fail()) {
      unlink(temp_filename.c_str());
      return false;
    }
    in.read(buffer, buffer_size);
    count = in.gcount();
  }
  out.close();

  if (!in.eof()) {
    unlink(temp_filename.c_str());
    return false;
  }

  if (rename(temp_filename.c_str(), to_filename.c_str()) == 0) {
    return true;
  }

  unlink(temp_filename.c_str());
  return false;
}
