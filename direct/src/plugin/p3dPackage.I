/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file p3dPackage.I
 * @author drose
 * @date 2009-06-12
 */

/**
 * Returns true if the package file information has been downloaded and
 * verified and is ready to be consulted, false if it is not yet available.
 */
inline bool P3DPackage::
get_info_ready() const {
  return _info_ready;
}

/**
 * Returns the number of bytes that will need to be downloaded, when this
 * package is downloaded.  This is intended to be used to estimate the
 * download time for this package relative to other packages, for instance to
 * update a progress bar sensibly.
 */
inline size_t P3DPackage::
get_download_size() const {
  return _compressed_archive.get_size();
}

/**
 * Returns true if the package has been downloaded and verified and is ready
 * to be used, false if it has not.
 */
inline bool P3DPackage::
get_ready() const {
  return _ready;
}

/**
 * Returns true if the package cannot be made ready, for instance because the
 * download server is down.
 */
inline bool P3DPackage::
get_failed() const {
  return _failed;
}

/**
 * Returns the host server which offers this package for download.
 */
inline P3DHost *P3DPackage::
get_host() const {
  return _host;
}

/**
 * Returns the directory into which this package is installed.
 */
inline const string &P3DPackage::
get_package_dir() const {
  return _package_dir;
}

/**
 * Returns the name of this package.  This is an internal name, used to
 * generate filenames and the like; it will generally be all-lowercase and
 * will not contain spaces.  See also get_package_display_name() for a name
 * suitable for displaying to the user.
 */
inline const string &P3DPackage::
get_package_name() const {
  return _package_name;
}

/**
 * Returns the version string of this package.
 */
inline const string &P3DPackage::
get_package_version() const {
  return _package_version;
}

/**
 * Returns the platform string of this package.
 */
inline const string &P3DPackage::
get_package_platform() const {
  return _package_platform;
}

/**
 * Returns the display_name name of this package, as set in the desc file.
 */
inline const string &P3DPackage::
get_package_display_name() const {
  return _package_display_name;
}

/**
 * Returns the <config> entry of the package desc file, if any, or NULL if it
 * was not present.
 */
inline const TiXmlElement *P3DPackage::
get_xconfig() const {
  return _xconfig;
}

/**
 * Returns the full path to the package's desc file.  If this is a "solo" type
 * package, the desc file itself represents the entire contents of the
 * package.
 */
inline const string &P3DPackage::
get_desc_file_pathname() const {
  return _desc_file_pathname;
}

/**
 * Returns the relative path, on the host, of the directory that contains the
 * desc file (and to which all of the paths in the desc file are relative).
 */
inline const string &P3DPackage::
get_desc_file_dirname() const {
  return _desc_file_dirname;
}

/**
 * Returns the full path to the package's uncompressed archive file.  This is
 * only valid if get_ready() is true and the package is not a "solo" package.
 */
inline string P3DPackage::
get_archive_file_pathname() const {
  return _uncompressed_archive.get_pathname(_package_dir);
}


/**
 * Returns the relative amount of effort of this step.
 */
inline double P3DPackage::InstallStep::
get_effort() const {
  return _bytes_needed * _bytes_factor;
}

/**
 * Returns the progress of this step, in the range 0..1.
 */
inline double P3DPackage::InstallStep::
get_progress() const {
  if (_bytes_needed == 0) {
    return 1.0;
  }
  return min((double)_bytes_done / (double)_bytes_needed, 1.0);
}

/**
 * Notifies the Package that progress has been made on this particular step.
 */
inline void P3DPackage::InstallStep::
report_step_progress() {
  _package->report_progress(this);
}

/**
 *
 */
inline P3DPackage::RequiredPackage::
RequiredPackage(const string &package_name, const string &package_version,
                const string &package_seq, P3DHost *host) :
  _package_name(package_name),
  _package_version(package_version),
  _package_seq(package_seq),
  _host(host)
{
}
