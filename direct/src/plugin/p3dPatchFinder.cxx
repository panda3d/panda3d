// Filename: p3dPatchFinder.cxx
// Created by:  drose (27Sep09)
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

#include "p3dPatchFinder.h"

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::PackageVersion::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPatchFinder::PackageVersion::
PackageVersion(const PackageVersionKey &key) :
  _package_name(key._package_name),
  _platform(key._platform),
  _version(key._version),
  _host_url(key._host_url),
  _file(key._file)
{
  _package_current = NULL;
  _package_base = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::PackageVersion::get_patch_chain
//       Access: Public
//  Description: Fills chain with the list of patches that, when
//               applied in sequence to the indicated PackageVersion
//               object, produces this PackageVersion object.  Returns
//               false if no chain can be found.
////////////////////////////////////////////////////////////////////
bool P3DPatchFinder::PackageVersion::
get_patch_chain(Patchfiles &chain, PackageVersion *start_pv,
                const PackageVersionsList &already_visited_in) {
  chain.clear();
  if (this == start_pv) {
    // We're already here.  A zero-length patch chain is therefore the
    // answer.
    return true;
  }
  if (::find(already_visited_in.begin(), already_visited_in.end(), this) != already_visited_in.end()) {
    // We've already been here; this is a loop.  Avoid infinite
    // recursion.
    return false;
  }

  // Yeah, we make a new copy of this vector at each stage of the
  // recursion.  This could be made much faster with a linked list
  // instead, but I'm working on the assumption that there will be no
  // more than a few dozen patchfiles, in which case this naive
  // approach should be fast enough.
  PackageVersionsList already_visited = already_visited_in;
  already_visited.push_back(this);

  bool found_any = false;
  Patchfiles::iterator pi;
  for (pi = _from_patches.begin(); pi != _from_patches.end(); ++pi) {
    Patchfile *patchfile = (*pi);
    PackageVersion *from_pv = patchfile->_from_pv;
    assert(from_pv != NULL);
    Patchfiles this_chain;
    if (from_pv->get_patch_chain(this_chain, start_pv, already_visited)) {
      // There's a path through this patchfile.
      this_chain.push_back(patchfile);
      if (!found_any || this_chain.size() < chain.size()) {
        found_any = true;
        chain.swap(this_chain);
      }
    }
  }

  // If found_any is true, we've already filled chain with the
  // shortest path found.
  return found_any;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::PackageVersionKey::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPatchFinder::PackageVersionKey::
PackageVersionKey(const string &package_name,
                  const string &platform,
                  const string &version,
                  const string &host_url,
                  const FileSpec &file) :
  _package_name(package_name),
  _platform(platform),
  _version(version),
  _host_url(host_url),
  _file(file)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::PackageVersionKey::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
bool P3DPatchFinder::PackageVersionKey::
operator < (const PackageVersionKey &other) const {
  if (_package_name != other._package_name) {
    return _package_name < other._package_name;
  }
  if (_platform != other._platform) {
    return _platform < other._platform;
  }
  if (_version != other._version) {
    return _version < other._version;
  }
  if (_host_url != other._host_url) {
    return _host_url < other._host_url;
  }
  return _file.compare_hash(other._file) < 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::PackageVersionKey::operator <
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DPatchFinder::PackageVersionKey::
output(ostream &out) const {
  out << "(" << _package_name << ", " << _platform << ", " << _version
      << ", " << _host_url << ", ";
  _file.output_hash(out);
  out << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Patchfile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPatchFinder::Patchfile::
Patchfile(Package *package) :
  _package(package),
  _from_pv(NULL),
  _to_pv(NULL)
{
  _package_name = package->_package_name;
  _platform = package->_platform;
  _version = package->_version;
  _host_url = package->_host_url;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Patchfile::get_source_key
//       Access: Public
//  Description: Returns the key for locating the package that this
//               patchfile can be applied to.
////////////////////////////////////////////////////////////////////
P3DPatchFinder::PackageVersionKey P3DPatchFinder::Patchfile::
get_source_key() const {
  return PackageVersionKey(_package_name, _platform, _version, _host_url, _source_file);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Patchfile::get_target_key
//       Access: Public
//  Description: Returns the key for locating the package that this
//               patchfile will generate.
////////////////////////////////////////////////////////////////////
P3DPatchFinder::PackageVersionKey P3DPatchFinder::Patchfile::
get_target_key() const {
  return PackageVersionKey(_package_name, _platform, _version, _host_url, _target_file);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Patchfile::load_xml
//       Access: Public
//  Description: Reads the data structures from an xml file.
////////////////////////////////////////////////////////////////////
void P3DPatchFinder::Patchfile::
load_xml(TiXmlElement *xpatch) {
  const char *package_name_cstr = xpatch->Attribute("name");
  if (package_name_cstr != NULL && *package_name_cstr) {
    _package_name = package_name_cstr;
  }
  const char *platform_cstr = xpatch->Attribute("platform");
  if (platform_cstr != NULL && *platform_cstr) {
    _platform = platform_cstr;
  }
  const char *version_cstr = xpatch->Attribute("version");
  if (version_cstr != NULL && *version_cstr) {
    _version = version_cstr;
  }
  const char *host_url_cstr = xpatch->Attribute("host");
  if (host_url_cstr != NULL && *host_url_cstr) {
    _host_url = host_url_cstr;
  }

  _file.load_xml(xpatch);

  TiXmlElement *xsource = xpatch->FirstChildElement("source");
  if (xsource != NULL) {
    _source_file.load_xml(xsource);
  }
  TiXmlElement *xtarget = xpatch->FirstChildElement("target");
  if (xtarget != NULL) {
    _target_file.load_xml(xtarget);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Package::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPatchFinder::Package::
Package() {
  _current_pv = NULL;
  _base_pv = NULL;
  _got_base_file = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Package::get_current_key
//       Access: Public
//  Description: Returns the key to locate the current version of this
//               package.
////////////////////////////////////////////////////////////////////
P3DPatchFinder::PackageVersionKey P3DPatchFinder::Package::
get_current_key() const {
  return PackageVersionKey(_package_name, _platform, _version, _host_url, _current_file);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Package::get_base_key
//       Access: Public
//  Description: Returns the key to locate the "base" or oldest
//               version of this package.
////////////////////////////////////////////////////////////////////
P3DPatchFinder::PackageVersionKey P3DPatchFinder::Package::
get_base_key() const {
  return PackageVersionKey(_package_name, _platform, _version, _host_url, _base_file);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Package::get_generic_key
//       Access: Public
//  Description: Returns the key that has the indicated hash.
////////////////////////////////////////////////////////////////////
P3DPatchFinder::PackageVersionKey P3DPatchFinder::Package::
get_generic_key(const FileSpec &file) const {
  return PackageVersionKey(_package_name, _platform, _version, _host_url, file);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Package::read_desc_file
//       Access: Public
//  Description: Reads the package's desc file for the package
//               information.  Returns true on success, false on
//               failure.
////////////////////////////////////////////////////////////////////
bool P3DPatchFinder::Package::
read_desc_file(TiXmlDocument *doc) {
  TiXmlElement *xpackage = doc->FirstChildElement("package");
  if (xpackage == NULL) {
    return false;
  }

  const char *package_name_cstr = xpackage->Attribute("name");
  if (package_name_cstr != NULL && *package_name_cstr) {
    _package_name = package_name_cstr;
  }
  const char *platform_cstr = xpackage->Attribute("platform");
  if (platform_cstr != NULL && *platform_cstr) {
    _platform = platform_cstr;
  }
  const char *version_cstr = xpackage->Attribute("version");
  if (version_cstr != NULL && *version_cstr) {
    _version = version_cstr;
  }
  const char *host_url_cstr = xpackage->Attribute("host");
  if (host_url_cstr != NULL && *host_url_cstr) {
    _host_url = host_url_cstr;
  }

  // Get the current version.
  TiXmlElement *xarchive = xpackage->FirstChildElement("uncompressed_archive");
  if (xarchive != NULL) {
    _current_file.load_xml(xarchive);
  }

  // Get the base_version--the bottom (oldest) of the patch chain.
  xarchive = xpackage->FirstChildElement("base_version");
  if (xarchive != NULL) {
    _base_file.load_xml(xarchive);
    _got_base_file = true;
  }

  _patches.clear();
  TiXmlElement *xpatch = xpackage->FirstChildElement("patch");
  while (xpatch != NULL) {
    Patchfile *patchfile = new Patchfile(this);
    patchfile->load_xml(xpatch);
    _patches.push_back(patchfile);
    xpatch = xpatch->NextSiblingElement("patch");
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPatchFinder::
P3DPatchFinder() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPatchFinder::
~P3DPatchFinder() {
  // TODO.  Cleanup nicely.
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::get_patch_chain_to_current
//       Access: Public
//  Description: Loads the package defined in the indicated desc file,
//               and constructs a patch chain from the version
//               represented by file to the current version of this
//               package, if possible. Returns true if successful,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DPatchFinder::
get_patch_chain_to_current(Patchfiles &chain, TiXmlDocument *doc,
                           const FileSpec &file) {
  chain.clear();
  Package *package = read_package_desc_file(doc);
  if (package == NULL) {
    return false;
  }

  build_patch_chains();
  PackageVersion *from_pv = get_package_version(package->get_generic_key(file));
  PackageVersion *to_pv = package->_current_pv;

  if (to_pv != NULL && from_pv != NULL) {
    return to_pv->get_patch_chain(chain, from_pv, PackageVersionsList());
  }

  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::read_package_desc_file
//       Access: Public
//  Description: Reads a desc file associated with a particular
//               package, and adds the package to
//               _packages.  Returns the Package object, or
//               NULL on failure.
////////////////////////////////////////////////////////////////////
P3DPatchFinder::Package *P3DPatchFinder::
read_package_desc_file(TiXmlDocument *doc) {
  Package *package = new Package;
  if (!package->read_desc_file(doc)) {
    delete package;
    return NULL;
  }

  _packages.push_back(package);
  return package;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::build_patch_chains
//       Access: Public
//  Description: Builds up the chains of PackageVersions and the
//               patchfiles that connect them.
////////////////////////////////////////////////////////////////////
void P3DPatchFinder::
build_patch_chains() {
  Packages::iterator pi;
  for (pi = _packages.begin(); pi != _packages.end(); ++pi) {
    Package *package = (*pi);
    if (!package->_got_base_file) {
      // This package doesn't have any versions yet.
      continue;
    }

    PackageVersion *current_pv = get_package_version(package->get_current_key());
    package->_current_pv = current_pv;
    current_pv->_package_current = package;
    current_pv->_print_name = package->_current_file.get_filename();

    PackageVersion *base_pv = get_package_version(package->get_base_key());
    package->_base_pv = base_pv;
    base_pv->_package_base = package;
    base_pv->_print_name = package->_base_file.get_filename();

    Patchfiles::iterator fi;
    for (fi = package->_patches.begin(); fi != package->_patches.end(); ++fi) {
      record_patchfile(*fi);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::get_package_version
//       Access: Public
//  Description: Returns a shared PackageVersion object for the
//               indicated key.
////////////////////////////////////////////////////////////////////
P3DPatchFinder::PackageVersion *P3DPatchFinder::
get_package_version(const PackageVersionKey &key) {
  assert(!key._package_name.empty());
  PackageVersions::const_iterator vi = _package_versions.find(key);
  if (vi != _package_versions.end()) {
    return (*vi).second;
  }

  PackageVersion *pv = new PackageVersion(key);
  bool inserted = _package_versions.insert(PackageVersions::value_type(key, pv)).second;
  assert(inserted);
  return pv;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPatchFinder::record_patchfile
//       Access: Public
//  Description: Adds the indicated patchfile to the patch chains.
////////////////////////////////////////////////////////////////////
void P3DPatchFinder::
record_patchfile(Patchfile *patchfile) {
  PackageVersion *from_pv = get_package_version(patchfile->get_source_key());
  patchfile->_from_pv = from_pv;
  from_pv->_to_patches.push_back(patchfile);

  PackageVersion *to_pv = get_package_version(patchfile->get_target_key());
  patchfile->_to_pv = to_pv;
  to_pv->_from_patches.push_back(patchfile);
  to_pv->_print_name = patchfile->_file.get_filename();
}
