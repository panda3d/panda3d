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
#include "mkdir_complete.h"
#include "openssl/md5.h"

#include <algorithm>

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
  _download_url_prefix = _host_url_prefix;

  _descriptive_name = _host_url;

  _xcontents = NULL;
  _contents_seq = 0;
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

  Packages::iterator mi;
  for (mi = _packages.begin(); mi != _packages.end(); ++mi) {
    PackageMap &package_map = (*mi).second;
    PackageMap::iterator pi;
    for (pi = package_map.begin(); pi != package_map.end(); ++pi) {
      delete (*pi).second;
    }
  }
  _packages.clear();

  FailedPackages::iterator pi;
  for (pi = _failed_packages.begin(); pi != _failed_packages.end(); ++pi) {
    delete (*pi);
  }
  _failed_packages.clear();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::get_alt_host
//       Access: Public
//  Description: Returns the pre-defined alternate host with the
//               indicated token, if one is defined for this token, or
//               the original host if there is no alternate host
//               defined for this token.
//
//               This is intended to implement test versions and the
//               like, for instance in which a particular p3d file may
//               reference a package on one particular host, but there
//               is an alternate version to be tested on a different
//               host.  The HTML code that embeds the p3d file can
//               choose to set the alt_host token to redirect the p3d
//               file to the alternate host.
//
//               The actual URL for the alternate host is embedded
//               within the host's contents.xml as a security measure,
//               to prevent people from tricking a p3d file into
//               running untrusted code by redirecting it to an
//               arbitrary URL.
////////////////////////////////////////////////////////////////////
P3DHost *P3DHost::
get_alt_host(const string &alt_host) {
  assert(_xcontents != NULL);

  AltHosts::iterator hi;
  hi = _alt_hosts.find(alt_host);
  if (hi != _alt_hosts.end() && (*hi).second != _host_url) {
    P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
    return inst_mgr->get_host((*hi).second);
  }
  return this;
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
  if (_host_dir.empty()) {
    // If we haven't got a host_dir yet, we can't read the contents.
    return false;
  }

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
  ++_contents_seq;
  _contents_spec.read_hash(contents_filename);

  TiXmlElement *xhost = _xcontents->FirstChildElement("host");
  if (xhost != NULL) {
    const char *url = xhost->Attribute("url");
    if (url != NULL && _host_url == string(url)) {
      // We're the primary host.  This is the normal case.
      read_xhost(xhost);

      // Build up the list of alternate hosts.
      TiXmlElement *xalthost = xhost->FirstChildElement("alt_host");
      while (xalthost != NULL) {
        const char *keyword = xalthost->Attribute("keyword");
        const char *url = xalthost->Attribute("url");
        if (keyword != NULL && url != NULL) {
          _alt_hosts[keyword] = url;
        }
        xalthost = xalthost->NextSiblingElement("alt_host");
      }

    } else {
      // We're not the primary host; perhaps we're an alternate host.
      TiXmlElement *xalthost = xhost->FirstChildElement("alt_host");
      while (xalthost != NULL) {
        const char *url = xalthost->Attribute("url");
        if (url != NULL && _host_url == string(url)) {
          // Yep, we're this alternate host.
          read_xhost(xalthost);
          break;
        }
        xalthost = xalthost->NextSiblingElement("alt_host");
      }
    }
  }

  if (_host_dir.empty()) {
    determine_host_dir("");
  }
  assert(!_host_dir.empty());
  mkdir_complete(_host_dir, nout);

  string standard_filename = _host_dir + "/contents.xml";
  if (standardize_filename(standard_filename) != 
      standardize_filename(contents_filename)) {
    if (!copy_file(contents_filename, standard_filename)) {
      nout << "Couldn't copy to " << standard_filename << "\n";
    }
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (_host_url == inst_mgr->get_host_url()) {
    // If this is also the plugin host, then copy the contents.xml
    // file into the root Panda directory as well, for the next plugin
    // iteration.
    string top_filename = inst_mgr->get_root_dir() + "/contents.xml";
    if (standardize_filename(top_filename) != 
        standardize_filename(contents_filename)) {
      if (!copy_file(contents_filename, top_filename)) {
        nout << "Couldn't copy to " << top_filename << "\n";
      }
    }
  }


  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::read_xhost
//       Access: Public
//  Description: Reads the host data from the <host> (or <alt_host>)
//               entry in the contents.xml file, or from a
//               p3d_info.xml file.
////////////////////////////////////////////////////////////////////
void P3DHost::
read_xhost(TiXmlElement *xhost) {
  const char *descriptive_name = xhost->Attribute("descriptive_name");
  if (descriptive_name != NULL && _descriptive_name.empty()) {
    _descriptive_name = descriptive_name;
  }

  const char *host_dir_basename = xhost->Attribute("host_dir");
  if (host_dir_basename == NULL) {
    host_dir_basename = "";
  }
  determine_host_dir(host_dir_basename);

  // Get the "download" URL, which is the source from which we
  // download everything other than the contents.xml file.
  const char *download_url = xhost->Attribute("download_url");
  if (download_url != NULL) {
    _download_url_prefix = download_url;
  }
  if (!_download_url_prefix.empty()) {
    if (_download_url_prefix[_download_url_prefix.size() - 1] != '/') {
      _download_url_prefix += "/";
    }
  } else {
    _download_url_prefix = _host_url_prefix;
  }
        
  TiXmlElement *xmirror = xhost->FirstChildElement("mirror");
  while (xmirror != NULL) {
    const char *url = xmirror->Attribute("url");
    if (url != NULL) {
      add_mirror(url);
    }
    xmirror = xmirror->NextSiblingElement("mirror");
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::get_package
//       Access: Public
//  Description: Returns a (possibly shared) pointer to the indicated
//               package.
////////////////////////////////////////////////////////////////////
P3DPackage *P3DHost::
get_package(const string &package_name, const string &package_version,
            const string &alt_host) {
  if (!alt_host.empty()) {
    if (_xcontents != NULL) {
      // If we're asking for an alt host and we've already read our
      // contents.xml file, then we already know all of our hosts, and
      // we can start the package off with the correct host immediately.
      P3DHost *new_host = get_alt_host(alt_host);
      return new_host->get_package(package_name, package_version);
    }

    // If we haven't read contents.xml yet, we need to create the
    // package first, then let it be responsible for downloading our
    // contents.xml, and it can migrate to its alt_host after that.
  }

  PackageMap &package_map = _packages[alt_host];

  string key = package_name + "_" + package_version;
  PackageMap::iterator pi = package_map.find(key);
  if (pi != package_map.end()) {
    P3DPackage *package = (*pi).second;
    if (!package->get_failed()) {
      return package;
    }

    // If the package has previously failed, move it aside and try
    // again (maybe it just failed because the user interrupted it).
    nout << "Package " << key << " has previously failed; trying again.\n";
    _failed_packages.push_back(package);
    (*pi).second = NULL;
  }

  P3DPackage *package = 
    new P3DPackage(this, package_name, package_version, alt_host);
  package_map[key] = package;

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
//     Function: P3DHost::migrate_package
//       Access: Public
//  Description: This is called by P3DPackage when it migrates from
//               this host to its final alt_host, after downloading
//               the contents.xml file for this file and learning the
//               true URL for its target alt_host.
////////////////////////////////////////////////////////////////////
void P3DHost::
migrate_package(P3DPackage *package, const string &alt_host, P3DHost *new_host) {
  assert(new_host != this);
  assert(new_host == get_alt_host(alt_host));

  string key = package->get_package_name() + "_" + package->get_package_version();

  PackageMap &package_map = _packages[alt_host];
  PackageMap::iterator pi = package_map.find(key);
  assert(pi != package_map.end());
  package_map.erase(pi);

  PackageMap &new_package_map = new_host->_packages[""];
  bool inserted = new_package_map.insert(PackageMap::value_type(key, package)).second;
  assert(inserted);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::choose_random_mirrors
//       Access: Public
//  Description: Selects num_mirrors elements, chosen at random, from
//               the _mirrors list.  Adds the selected mirrors to
//               result.  If there are fewer than num_mirrors elements
//               in the list, adds only as many mirrors as we can get.
////////////////////////////////////////////////////////////////////
void P3DHost::
choose_random_mirrors(vector<string> &result, int num_mirrors) {
  vector<size_t> selected;

  size_t num_to_select = min(_mirrors.size(), (size_t)num_mirrors);
  while (num_to_select > 0) {
    size_t i = (size_t)(((double)rand() / (double)RAND_MAX) * _mirrors.size());
    while (find(selected.begin(), selected.end(), i) != selected.end()) {
      // Already found this i, find a new one.
      i = (size_t)(((double)rand() / (double)RAND_MAX) * _mirrors.size());
    }
    selected.push_back(i);
    result.push_back(_mirrors[i]);
    --num_to_select;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::add_mirror
//       Access: Public
//  Description: Adds a new URL to serve as a mirror for this host.
//               The mirrors will be consulted first, before
//               consulting the host directly.
////////////////////////////////////////////////////////////////////
void P3DHost::
add_mirror(string mirror_url) {
  // Ensure the URL ends in a slash.
  if (!mirror_url.empty() && mirror_url[mirror_url.size() - 1] != '/') {
    mirror_url += '/';
  }
  
  // Add it to the _mirrors list, but only if it's not already
  // there.
  if (find(_mirrors.begin(), _mirrors.end(), mirror_url) == _mirrors.end()) {
    _mirrors.push_back(mirror_url);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DHost::uninstall
//       Access: Public
//  Description: Removes the host directory and all its contents
//               from the user's hard disk.
////////////////////////////////////////////////////////////////////
void P3DHost::
uninstall() {
  if (_host_dir.empty()) {
    nout << "Cannot uninstall " << _descriptive_name << ": host directory not yet known.\n";
    return;
  }

  // First, explicitly uninstall each of our packages.
  Packages::iterator mi;
  for (mi = _packages.begin(); mi != _packages.end(); ++mi) {
    PackageMap &package_map = (*mi).second;
    PackageMap::iterator pi;
    for (pi = package_map.begin(); pi != package_map.end(); ++pi) {
      P3DPackage *package = (*pi).second;
      package->uninstall();
    }
  }

  // Then, uninstall the host itself.
  nout << "Uninstalling " << _descriptive_name << " from " << _host_dir << "\n";

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->delete_directory_recursively(_host_dir);
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
determine_host_dir(const string &host_dir_basename) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  _host_dir = inst_mgr->get_root_dir();
  _host_dir += "/hosts";

  if (!host_dir_basename.empty()) {
    // If the contents.xml specified a host_dir parameter, use it.
    inst_mgr->append_safe_dir(_host_dir, host_dir_basename);
    return;
  }

  // If we didn't get a host_dir parameter, we have to make one up.
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
    _host_dir += P3DInstanceManager::encode_hexdigit(high);
    _host_dir += P3DInstanceManager::encode_hexdigit(low);
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
