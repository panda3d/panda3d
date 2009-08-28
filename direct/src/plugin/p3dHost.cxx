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
#include "openssl/md5.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::Constructor
//       Access: Private
//  Description: Use P3DInstanceManager::get_host() to construct a new
//               P3DHost.
////////////////////////////////////////////////////////////////////
P3DHost::
P3DHost(const string &host_url) :
  _host_url(host_url) 
{
  // Ensure that the download URL ends with a slash.
  _host_url_prefix = _host_url;
  if (!_host_url_prefix.empty() && _host_url_prefix[_host_url_prefix.size() - 1] != '/') {
    _host_url_prefix += "/";
  }

  _xcontents = NULL;

  determine_host_dir();
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
//  Description: Reads the contents.xml file in the standard
//               filename, if possible.
//
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool P3DHost::
read_contents_file() {
  string standard_filename = _host_dir + "/contents.xml";
  return read_contents_file(standard_filename);
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

  const char *descriptive_name = _xcontents->Attribute("descriptive_name");
  if (descriptive_name != NULL) {
    _descriptive_name = descriptive_name;
  }

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
                      bool &package_solo,               // out
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
    const char *solo = xpackage->Attribute("solo");
    if (version == NULL) {
      version = "";
    }
    if (name != NULL && platform != NULL &&
        package_name == name && 
        inst_mgr->get_platform() == platform &&
        package_version == version) {
      // Here's the matching package definition.
      desc_file.load_xml(xpackage);
      package_platform = platform;
      package_solo = false;
      if (solo != NULL) {
        package_solo = (atoi(solo) != 0);
      }
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
    const char *solo = xpackage->Attribute("solo");
    if (platform == NULL) {
      platform = "";
    }
    if (version == NULL) {
      version = "";
    }
    if (name != NULL &&
        package_name == name && 
        *platform == '\0' &&
        package_version == version) {
      // Here's the matching package definition.
      desc_file.load_xml(xpackage);
      package_platform = platform;
      package_solo = false;
      if (solo != NULL) {
        package_solo = (atoi(solo) != 0);
      }
      return true;
    }

    xpackage = xpackage->NextSiblingElement("package");
  }

  // Couldn't find the named package.
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::determine_host_dir
//       Access: Private
//  Description: Hashes the host_url into a (mostly) unique directory
//               string, which will be the root of the host's install
//               tree.  Stores the result in _host_dir.
//
//               This code is duplicated in Python, in
//               HostInfo.determineHostDir().
////////////////////////////////////////////////////////////////////
void P3DHost::
determine_host_dir() {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  _host_dir = inst_mgr->get_root_dir();
  _host_dir += "/";

  string hostname;

  // Look for a server name in the URL.  Including this string in the
  // directory name makes it friendlier for people browsing the
  // directory.

  // We can't use URLSpec here, because we don't link with Panda3D.
  // We have to do it by hand.
  size_t p = _host_url.find("://");
  if (p != string::npos) {
    size_t start = p + 3;
    size_t end = _host_url.find("/",  start);
    // Now start .. end is something like "username@host:port".

    size_t at = _host_url.find("@", start);
    if (at < end) {
      start = at + 1;
    }

    size_t colon = _host_url.find(":", start);
    if (colon < end) {
      end = colon;
    }

    // Now start .. end is just the hostname.
    hostname = _host_url.substr(start, end - start);
  }

  // Now build a hash string of the whole URL.  We'll use MD5 to get a
  // pretty good hash, with a minimum chance of collision.  Even if
  // there is a hash collision, though, it's not the end of the world;
  // it just means that both hosts will dump their packages into the
  // same directory, and they'll fight over the toplevel contents.xml
  // file.  Assuming they use different version numbers (which should
  // be safe since they have the same hostname), there will be minimal
  // redownloading.


  static const size_t hash_size = 16;
  unsigned char md[hash_size];

  size_t keep_hash = hash_size;

  if (!hostname.empty()) {
    _host_dir += hostname;
    _host_dir += "_";

    // If we successfully got a hostname, we don't really need the
    // full hash.  We'll keep half of it.
    keep_hash = keep_hash / 2;
  }

  MD5_CTX ctx;
  MD5_Init(&ctx);
  MD5_Update(&ctx, _host_url.data(), _host_url.size());
  MD5_Final(md, &ctx);

  for (size_t i = 0; i < keep_hash; ++i) {
    int high = (md[i] >> 4) & 0xf;
    int low = md[i] & 0xf;
    _host_dir += encode_hexdigit(high);
    _host_dir += encode_hexdigit(low);
  }
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

  unlink(to_filename.c_str());
  if (rename(temp_filename.c_str(), to_filename.c_str()) == 0) {
    return true;
  }

  unlink(temp_filename.c_str());
  return false;
}
