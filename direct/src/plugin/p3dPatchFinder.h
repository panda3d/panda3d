// Filename: p3dPatchFinder.h
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

#ifndef P3DPATCHFINDER_H
#define P3DPATCHFINDER_H

#include "p3d_plugin_common.h"
#include "fileSpec.h"
#include "get_tinyxml.h"
#include <vector>
#include <map>

////////////////////////////////////////////////////////////////////
//       Class : P3DPatchFinder
// Description : This class is used to reconstruct the patch
//               chain--the chain of patch files needed to generate a
//               file--for downloading a package via patches, rather
//               than downloading the entire file.
//
//               It is similar to PatchMaker.py, except it only reads
//               patches, it does not generate them.
////////////////////////////////////////////////////////////////////
class P3DPatchFinder {
public:
  class Package;
  class Patchfile;
  class PackageVersion;

  typedef vector<Patchfile *> Patchfiles;
  typedef vector<PackageVersion *> PackageVersionsList;

  // This class is used to index into a map to locate PackageVersion
  // objects, below.
  class PackageVersionKey {
  public:
    PackageVersionKey(const string &package_name,
                      const string &platform,
                      const string &version,
                      const string &host_url,
                      const FileSpec &file);
    bool operator < (const PackageVersionKey &other) const;
    void output(ostream &out) const;

  public:
    string _package_name;
    string _platform;
    string _version;
    string _host_url;
    FileSpec _file;
  };

  // A specific version of a package.  This is not just a package's
  // "version" string; it also corresponds to the particular patch
  // version, which increments independently of the "version".
  class PackageVersion {
  public:
    PackageVersion(const PackageVersionKey &key);

    bool get_patch_chain(Patchfiles &chain, PackageVersion *start_pv,
                         const PackageVersionsList &already_visited_in);

  public:
    string _package_name;
    string _platform;
    string _version;
    string _host_url;
    FileSpec _file;
    string _print_name;

    // The Package object that produces this version if this is the
    // current form or the base form, respectively.
    Package *_package_current;
    Package *_package_base;

    // A list of patchfiles that can produce this version.
    Patchfiles _from_patches;

    // A list of patchfiles that can start from this version.
    Patchfiles _to_patches;
  };

  // A single patchfile for a package.
  class Patchfile {
  public:
    Patchfile(Package *package);

    PackageVersionKey get_source_key() const;
    PackageVersionKey get_target_key() const;
    void load_xml(TiXmlElement *xpatch);

  public:
    Package *_package;
    string _package_name;
    string _platform;
    string _version;
    string _host_url;

    // The patchfile itself
    FileSpec _file;

    // The package file that the patch is applied to
    FileSpec _source_file;

    // The package file that the patch generates
    FileSpec _target_file;

    // The PackageVersion corresponding to our source_file
    PackageVersion *_from_pv;

    // The PackageVersion corresponding to our target_file
    PackageVersion *_to_pv;
  };

  // This is a particular package.  This contains all of the
  // information extracted from the package's desc file.
  class Package {
  public:
    Package();

    PackageVersionKey get_current_key() const;
    PackageVersionKey get_base_key() const;
    PackageVersionKey get_generic_key(const FileSpec &file) const;

    bool read_desc_file(TiXmlDocument *doc);

  public:
    string _package_name;
    string _platform;
    string _version;
    string _host_url;

    PackageVersion *_current_pv;
    PackageVersion *_base_pv;

    FileSpec _current_file;
    FileSpec _base_file;
    bool _got_base_file;

    Patchfiles _patches;
  };

public:
  P3DPatchFinder();
  ~P3DPatchFinder();

  bool get_patch_chain_to_current(Patchfiles &chain, TiXmlDocument *doc,
                                  const FileSpec &file);

  Package *read_package_desc_file(TiXmlDocument *doc);
  void build_patch_chains();
  PackageVersion *get_package_version(const PackageVersionKey &key);

private:
  void record_patchfile(Patchfile *patchfile);

private:
  typedef map<PackageVersionKey, PackageVersion *> PackageVersions;
  PackageVersions _package_versions;

  typedef vector<Package *> Packages;
  Packages _packages;
};

#include "p3dPatchFinder.I"

inline ostream &operator << (ostream &out, const P3DPatchFinder::PackageVersionKey &key) {
  key.output(out);
  return out;
}

#endif

