// Filename: p3dPackage.cxx
// Created by:  drose (12Jun09)
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

#include "p3dPackage.h"
#include "p3dInstanceManager.h"
#include "p3dInstance.h"
#include "p3dMultifileReader.h"
#include "p3dTemporaryFile.h"
#include "p3dPatchFinder.h"
#include "mkdir_complete.h"
#include "wstring_encode.h"

#include "zlib.h"

#include <algorithm>
#include <fstream>

#ifdef _WIN32
#include <io.h>    // chmod()
#endif

// Weight factors for computing download progress.  This attempts to
// reflect the relative time-per-byte of each of these operations.
const double P3DPackage::_download_factor = 1.0;
const double P3DPackage::_uncompress_factor = 0.01;
const double P3DPackage::_unpack_factor = 0.01;
const double P3DPackage::_patch_factor = 0.01;

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::
P3DPackage(P3DHost *host, const string &package_name,
           const string &package_version, const string &package_platform,
           const string &alt_host) :
  _host(host),
  _package_name(package_name),
  _package_version(package_version),
  _package_platform(package_platform),
  _alt_host(alt_host)
{
  set_fullname();
  _per_platform = false;
  _patch_version = 0;

  // This is set true if the package is a "solo", i.e. a single
  // file, instead of an xml file and a multifile to unpack.
  _package_solo = false;

  _host_contents_iseq = 0;

  _xconfig = NULL;
  _temp_contents_file = NULL;

  _computed_plan_size = false;
  _info_ready = false;
  _allow_data_download = false;
  _ready = false;
  _failed = false;
  _active_download = NULL;
  _saved_download = NULL;
  _updated = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::
~P3DPackage() {
  // Tell any pending callbacks that we're no good any more.
  if (!_ready && !_failed) {
    report_done(false);
  }

  // Ditto the outstanding instances.
  Instances::iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    (*ii)->remove_package(this);
  }
  _instances.clear();

  if (_xconfig != NULL) {
    delete _xconfig;
    _xconfig = NULL;
  }

  // Cancel any pending download.
  if (_active_download != NULL) {
    _active_download->cancel();
    set_active_download(NULL);
  }
  if (_saved_download != NULL) {
    _saved_download->cancel();
    set_saved_download(NULL);
  }

  if (_temp_contents_file != NULL) {
    delete _temp_contents_file;
    _temp_contents_file = NULL;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::activate_download
//       Access: Public
//  Description: Authorizes the package to begin downloading and
//               unpacking the meat of its data.  Until this is
//               called, the package will download its file
//               information only, and then wait.
////////////////////////////////////////////////////////////////////
void P3DPackage::
activate_download() {
  _allow_data_download = true;

  if (_ready) {
    // If we've already been downloaded, we can report that now.
    Instances::iterator ii;
    for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
      (*ii)->report_package_done(this, true);
    }

  } else {
    // Otherwise, if we've already got the desc file, then start the
    // download.
    if (_info_ready) {
      follow_install_plans(true, false);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::get_formatted_name
//       Access: Public
//  Description: Returns the name of this package, for output to the
//               user.  This will be the "public" name of the package,
//               as formatted for user consumption; it will include
//               capital letters and spaces where appropriate.
////////////////////////////////////////////////////////////////////
string P3DPackage::
get_formatted_name() const {
  ostringstream strm;

  if (!_package_display_name.empty()) {
    strm << _package_display_name;
  } else {
    strm << _package_name;
    if (!_package_version.empty()) {
      strm << " " << _package_version;
    }
  }

  if (_patch_version != 0) {
    strm << " rev " << _patch_version;
  }

  return strm.str();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::add_instance
//       Access: Public
//  Description: Specifies an instance that that will be using this
//               package, and may be responsible for downloading it.
////////////////////////////////////////////////////////////////////
void P3DPackage::
add_instance(P3DInstance *inst) {
  _instances.push_back(inst);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (!_host->has_current_contents_file(inst_mgr)) {
    // If the host needs to update its contents file, we're no longer
    // sure that we're current.
    _info_ready = false;
    _ready = false;
    _failed = false;
    _allow_data_download = false;
    nout << "No longer current: " << get_package_name() << "\n";
  }
  
  begin_info_download();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::remove_instance
//       Access: Public
//  Description: Indicates that the given instance is no longer
//               interested in this package and will not be
//               responsible for downloading it.
////////////////////////////////////////////////////////////////////
void P3DPackage::
remove_instance(P3DInstance *inst) {
  assert(!_instances.empty());

  if (inst == _instances[0]) {
    // This was the primary instance.  Cancel any pending download and
    // move to the next instance.
    if (_active_download != NULL) {
      _active_download->cancel();
      set_active_download(NULL);
    }
  }

  Instances::iterator ii = find(_instances.begin(), _instances.end(), inst);
  assert(ii != _instances.end());
  _instances.erase(ii);

  begin_info_download();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::mark_used
//       Access: Public
//  Description: Marks this package as having been "used", for
//               accounting purposes.
////////////////////////////////////////////////////////////////////
void P3DPackage::
mark_used() {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (inst_mgr->get_verify_contents() == P3D_VC_never) {
    // We're not allowed to create any files in the package directory.
    return;
  }

  // Unlike the Python variant of this function, we don't mess around
  // with updating the disk space or anything.
  string filename = get_package_dir() + "/usage.xml";
  TiXmlDocument doc(filename);
  if (!doc.LoadFile()) {
    TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "utf-8", "");
    doc.LinkEndChild(decl);
  }

  TiXmlElement *xusage = doc.FirstChildElement("usage");
  if (xusage == NULL) {
    xusage = new TiXmlElement("usage");
    doc.LinkEndChild(xusage);
  }

  time_t now = time(NULL);
  int count = 0;
  xusage->Attribute("count_runtime", &count);
  if (count == 0) {
    xusage->SetAttribute("first_use", (int)now);
  }

  ++count;
  xusage->SetAttribute("count_runtime", count);
  xusage->SetAttribute("last_use", (int)now);

  if (_updated) {
    // If we've updated the package, we're no longer sure what its
    // disk space is.  Remove that from the XML file, so that the
    // Python code can recompute it later.
    xusage->RemoveAttribute("disk_space");
    xusage->SetAttribute("last_update", (int)now);
  }

  // Write the file to a temporary filename, then atomically move it
  // to its actual filename, to avoid race conditions.
  ostringstream strm;
  strm << get_package_dir() << "/usage_";
#ifdef _WIN32
  strm << GetCurrentProcessId();
#else
  strm << getpid();
#endif
  strm << ".xml";
  string tfile = strm.str();

  unlink(tfile.c_str());
  if (doc.SaveFile(tfile)) {
    if (rename(tfile.c_str(), filename.c_str()) != 0) {
      // If rename failed, remove the original file first.
      unlink(filename.c_str());
      rename(tfile.c_str(), filename.c_str());
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::uninstall
//       Access: Public
//  Description: Removes the package directory and all its contents
//               from the user's hard disk.
////////////////////////////////////////////////////////////////////
void P3DPackage::
uninstall() {
  if (_package_dir.empty()) {
    nout << "Cannot uninstall " << _package_name << ": package directory not yet known.\n";
    return;
  }

  nout << "Uninstalling package " << _package_name << " from " << _package_dir << "\n";

  // First, make sure that all instances that are sharing this package
  // are stopped, so there will be no access conflicts preventing us
  // from removing the files.  This is particularly important on
  // Windows.
  Instances::iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    P3DInstance *inst = (*ii);
    P3DSession *session = inst->get_session();
    if (session != NULL) {
      nout << "Stopping session " << session << "\n";
      session->shutdown();
    }
    inst->set_failed();
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  inst_mgr->delete_directory_recursively(_package_dir);

  _info_ready = false;
  _ready = false;
  _failed = false;
  _allow_data_download = false;

  // Make sure the host forgets us too.
  _host->forget_package(this);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::make_xml
//       Access: Public
//  Description: Returns a newly-allocated XML structure that
//               corresponds to the package data within this
//               instance.
////////////////////////////////////////////////////////////////////
TiXmlElement *P3DPackage::
make_xml() {
  TiXmlElement *xpackage = new TiXmlElement("package");

  xpackage->SetAttribute("name", _package_name);
  if (!_package_platform.empty()) {
    xpackage->SetAttribute("platform", _package_platform);
  }
  if (!_package_version.empty()) {
    xpackage->SetAttribute("version", _package_version);
  }
  xpackage->SetAttribute("host", _host->get_host_url());
  xpackage->SetAttribute("host_dir", _host->get_host_dir());

  return xpackage;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::begin_info_download
//       Access: Private
//  Description: Begins downloading and installing the information
//               about the package, including its file size and
//               download source and such, if needed.  This is
//               generally a very small download.
////////////////////////////////////////////////////////////////////
void P3DPackage::
begin_info_download() {  
  if (_instances.empty()) {
    // Can't download without any instances.
    return;
  }

  if (_info_ready) {
    // Already downloaded.
    report_info_ready();
    return;
  }

  if (_active_download != NULL) {
    // In the middle of downloading.
    return;
  }

  download_contents_file();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::download_contents_file
//       Access: Private
//  Description: Starts downloading the root-level contents.xml file.
//               This is only done for the first package downloaded
//               from a particular host, and only if the host doesn't
//               have the file already.
////////////////////////////////////////////////////////////////////
void P3DPackage::
download_contents_file() {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (!_host->has_contents_file() && inst_mgr->get_verify_contents() != P3D_VC_force) {
    // First, read whatever contents file is already on disk.  Maybe
    // it's current enough.
    _host->read_contents_file();
  }

  if (_host->has_current_contents_file(inst_mgr)) {
    // We've already got a contents.xml file; go straight to the
    // package desc file.
    host_got_contents_file();
    return;
  }

  // Don't download it if we're not allowed to.
  if (inst_mgr->get_verify_contents() == P3D_VC_never) {
    contents_file_download_finished(false);
    return;
  }

  // Download contents.xml to a temporary filename first, in case
  // multiple packages are downloading it simultaneously.
  if (_temp_contents_file != NULL) {
    delete _temp_contents_file;
    _temp_contents_file = NULL;
  }
  _temp_contents_file = new P3DTemporaryFile(".xml");

  start_download(DT_contents_file, "contents.xml", 
                 _temp_contents_file->get_filename(), FileSpec());
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::contents_file_download_finished
//       Access: Private
//  Description: Called when the contents.xml file has been fully
//               downloaded.
////////////////////////////////////////////////////////////////////
void P3DPackage::
contents_file_download_finished(bool success) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (!_host->has_current_contents_file(inst_mgr)) {
    if (!success || _temp_contents_file == NULL ||
      !_host->read_contents_file(_temp_contents_file->get_filename(), true)) {
      
      if (_temp_contents_file) {
        nout << "Couldn't read " << *_temp_contents_file << "\n";
      }

      // Maybe we can read an already-downloaded contents.xml file.
      bool success = false;
      if (_host->has_host_dir()) {
        string standard_filename = _host->get_host_dir() + "/contents.xml";
        if (_host->read_contents_file(standard_filename, false)) {
          success = true;
        } else {
          nout << "Couldn't read " << standard_filename << "\n";
        }
      } else {
        nout << "No host_dir available for " << _host->get_host_url()
             << "\n";
      }
      if (!success) {
        // Couldn't read an already-downloaded file either.  Fail.
        report_done(false);
        if (_temp_contents_file) {
          delete _temp_contents_file;
          _temp_contents_file = NULL;
        }
        return;
      }
    }
  }
    
  // The file is correctly installed by now; we can remove the
  // temporary file.
  if (_temp_contents_file) {
    delete _temp_contents_file;
    _temp_contents_file = NULL;
  }

  host_got_contents_file();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::redownload_contents_file
//       Access: Private
//  Description: Starts a new download attempt of contents.xml, to
//               check to see whether our local copy is stale.  This
//               is called only from download_desc_file(), or from
//               Download::download_finished().  If the former, the
//               download pointer will be NULL.
//
//               If it turns out a new version can be downloaded, the
//               indicated Download object (and the current install
//               plan) is discarded, and the package download is
//               restarted from the beginning.
//
//               If there is no new version available, calls
//               resume_download_finished() on the indicated Download
//               object, to carry on as if nothing had happened.
////////////////////////////////////////////////////////////////////
void P3DPackage::
redownload_contents_file(P3DPackage::Download *download) {
  assert(_active_download == NULL);
  assert(_saved_download == NULL);
  
  if (_host->get_contents_iseq() != _host_contents_iseq) {
    // If the contents_iseq number has changed, we don't even need to
    // download anything--just go restart the download.
    host_got_contents_file();
    return;
  }
  
  // Don't download it if we're not allowed to.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (inst_mgr->get_verify_contents() == P3D_VC_never) {
    return;
  }
  
  set_saved_download(download);

  // Download contents.xml to a temporary filename first.
  if (_temp_contents_file != NULL) {
    delete _temp_contents_file;
    _temp_contents_file = NULL;
  }
  _temp_contents_file = new P3DTemporaryFile(".xml");

  start_download(DT_redownload_contents_file, "contents.xml", 
                 _temp_contents_file->get_filename(), FileSpec());
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::contents_file_redownload_finished
//       Access: Private
//  Description: Called when the redownload attempt on contents.xml
//               has finished.
////////////////////////////////////////////////////////////////////
void P3DPackage::
contents_file_redownload_finished(bool success) {
  bool contents_changed = false;
  
  if (_host->get_contents_iseq() != _host_contents_iseq) {
    // If the contents_iseq number has changed, we don't even need to
    // bother reading what we just downloaded.
    contents_changed = true;
  }

  if (!contents_changed && success) {
    // If we successfully downloaded something, see if it's different
    // from what we had before.
    if (!_host->check_contents_hash(_temp_contents_file->get_filename())) {
      // It changed!  Now see if we can read the new contents.
      if (!_host->read_contents_file(_temp_contents_file->get_filename(), true)) {
        // Huh, appears to have changed to something bad.  Never mind.
        nout << "Couldn't read " << *_temp_contents_file << "\n";

      } else {
        // The new contents file is read and in place.
        contents_changed = true;
      }
    }
  }
    
  // We no longer need the temporary file.
  if (_temp_contents_file) {
    delete _temp_contents_file;
    _temp_contents_file = NULL;
  }

  if (contents_changed) {
    // OK, the contents.xml has changed; this means we have to restart
    // the whole download process from the beginning.
    nout << "Redownloading contents.xml made a difference.\n";
    set_saved_download(NULL);
    host_got_contents_file();

  } else {
    // Nothing's changed.  This was just a useless diversion.  We now
    // return you to our regularly scheduled download.
    nout << "Redownloading contents.xml didn't help.\n";
    Download *download = _saved_download;
    _saved_download = NULL;
    if (download == NULL) {
      // But, if _saved_download was NULL (meaning NULL was passed to
      // redownload_contents_file(), above), it means that we were
      // called from download_desc_file(), and there's nothing more to
      // do.  We're just hosed.
      report_done(false);
      
    } else {
      download->resume_download_finished(false);
      p3d_unref_delete(download);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::host_got_contents_file
//       Access: Private
//  Description: We come here when we've successfully downloaded and
//               read the host's contents.xml file.  This begins the
//               rest of the download process.
////////////////////////////////////////////////////////////////////
void P3DPackage::
host_got_contents_file() {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  if (!_alt_host.empty()) {
    // If we have an alt host specification, maybe we need to change
    // the host now.
    P3DHost *new_host = _host->get_alt_host(_alt_host);
    nout << "Migrating " << get_package_name() << " to alt_host " 
         << _alt_host << ": " << new_host->get_host_url() << "\n";
    if (new_host != _host) {
      _host->migrate_package_host(this, _alt_host, new_host);
      _host = new_host;
    }

    // Clear the alt_host string now that we're migrated to our final
    // host.
    _alt_host.clear();

    if (!_host->has_current_contents_file(inst_mgr)) {
      // Now go back and get the contents.xml file for the new host.
      download_contents_file();
      return;
    }
  }

  // Record this now, so we'll know later whether the host has been
  // reloaded (e.g. due to some other package, from some other
  // instance, reloading it).
  _host_contents_iseq = _host->get_contents_iseq();

  // Now adjust the platform based on the available platforms
  // provided.
  assert(_alt_host.empty());
  string new_platform;
  if (_host->choose_suitable_platform(new_platform, _per_platform,
                                      _package_name, _package_version, _package_platform)) {
    if (new_platform != _package_platform) {
      nout << "Migrating " << get_package_name() << " from platform \""
           << _package_platform << "\" to platform \"" 
           << new_platform << "\"\n";
      _package_platform = new_platform;
      set_fullname();
    }
  } else {
    nout << "Couldn't find a platform for " << get_package_name()
         << " matching \"" << inst_mgr->get_platform() << "\".\n";
  }

  nout << "_per_platform for " << get_package_name() << " = " << _per_platform << "\n";

  // Now that we have a valid host and platform, we can define the
  // _package_dir.
  _package_dir = _host->get_host_dir() + string("/") + _package_name;
  if (!_package_version.empty()) {
    _package_dir += string("/") + _package_version;
  }
  if (_per_platform && !_package_platform.empty()) {
    _package_dir += string("/") + _package_platform;
  }

  // Ensure the package directory exists; create it if it does not.
  if (inst_mgr->get_verify_contents() != P3D_VC_never) {
    mkdir_complete(_package_dir, nout);
  }
  download_desc_file();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::download_desc_file
//       Access: Private
//  Description: Starts downloading the desc file for the package, if
//               it's needed; or read the local version if it's fresh
//               enough.
////////////////////////////////////////////////////////////////////
void P3DPackage::
download_desc_file() {
  assert(!_package_dir.empty());

  // Attempt to check the desc file for freshness.  If it already
  // exists, and is consistent with the server contents file, we don't
  // need to re-download it.
  string package_seq;
  if (!_host->get_package_desc_file(_desc_file, package_seq, _package_solo,
                                    _package_name, _package_version,
                                    _package_platform)) {
    nout << "Couldn't find package " << _package_fullname
         << ", platform \"" << _package_platform
         << "\" in contents file.\n";
    redownload_contents_file(NULL);
    return;
  }

  string url_filename = _desc_file.get_filename();

  _desc_file_dirname = "";
  _desc_file_basename = url_filename;
  size_t slash = _desc_file_basename.rfind('/');
  if (slash != string::npos) {
    _desc_file_dirname = _desc_file_basename.substr(0, slash);
    _desc_file_basename = _desc_file_basename.substr(slash + 1);
  }

  // The desc file might have a different path on the host server than
  // it has locally, because we might strip out the platform directory
  // locally (according to _per_platform).
  FileSpec local_desc_file = _desc_file;
  local_desc_file.set_filename(_desc_file_basename);
  _desc_file_pathname = local_desc_file.get_pathname(_package_dir);

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (!local_desc_file.full_verify(_package_dir) && inst_mgr->get_verify_contents() != P3D_VC_never) {
    nout << _desc_file_pathname << " is stale.\n";

  } else {
    // The desc file is current.  Attempt to read it.
    if (_package_solo) {
      // No need to load it: the desc file *is* the package.
      report_done(true);
      return;
    } else {
      TiXmlDocument doc(_desc_file_pathname.c_str());
      if (doc.LoadFile()) {
        got_desc_file(&doc, false);
        return;
      }
    }
  }

  // Don't download it if we're not allowed to.
  if (inst_mgr->get_verify_contents() == P3D_VC_never) {
    nout << "Couldn't read " << _desc_file_pathname << "\n";
    report_done(false);
    return;
  }

  // The desc file is not current.  Go download it.
  start_download(DT_desc_file, _desc_file.get_filename(), 
                 _desc_file_pathname, local_desc_file);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::desc_file_download_finished
//       Access: Private
//  Description: Called when the desc file has been fully downloaded.
////////////////////////////////////////////////////////////////////
void P3DPackage::
desc_file_download_finished(bool success) {
  if (!success) {
    report_done(false);
    return;
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (inst_mgr->get_verify_contents() != P3D_VC_never) {
    // Now that we've downloaded the desc file, make it read-only.
    chmod(_desc_file_pathname.c_str(), 0444);
  }

  if (_package_solo) {
    // No need to load it: the desc file *is* the package.
    report_done(true);
    return;

  } else {
    TiXmlDocument doc(_desc_file_pathname.c_str());
    if (!doc.LoadFile()) {
      nout << "Couldn't read " << _desc_file_pathname << "\n";
      report_done(false);
      return;
    }
    
    got_desc_file(&doc, true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::got_desc_file
//       Access: Private
//  Description: Reads the desc file and begins verifying the files.
////////////////////////////////////////////////////////////////////
void P3DPackage::
got_desc_file(TiXmlDocument *doc, bool freshly_downloaded) {
  TiXmlElement *xpackage = doc->FirstChildElement("package");
  if (xpackage == NULL) {
    nout << _package_name << " desc file contains no <package>\n";
    if (!freshly_downloaded) {
      download_desc_file();
      return;
    }
    report_done(false);
    return;
  }

  bool per_platform = parse_bool_attrib(xpackage, "per_platform", false);
  if (per_platform != _per_platform) {
    nout << "Warning! per_platform disagreement for " << get_package_name()
         << "!\n";
    // We don't do anything with this warning--the original value for
    // _per_platform we got from the contents.xml file has to apply,
    // because we're already committed to the _package_dir we're
    // using.
  }
  
  xpackage->Attribute("patch_version", &_patch_version);
  
  TiXmlElement *xconfig = xpackage->FirstChildElement("config");
  if (xconfig != NULL) {
    const char *display_name_cstr = xconfig->Attribute("display_name");
    if (display_name_cstr != NULL) {
      _package_display_name = display_name_cstr;
    }
    
    // Save the config entry within this class for others to query.
    _xconfig = (TiXmlElement *)xconfig->Clone();
  }

  TiXmlElement *xuncompressed_archive = 
    xpackage->FirstChildElement("uncompressed_archive");
  TiXmlElement *xcompressed_archive = 
    xpackage->FirstChildElement("compressed_archive");

  if (xuncompressed_archive == NULL || xcompressed_archive == NULL) {
    // The desc file didn't include the archive file itself, weird.
    if (!freshly_downloaded) {
      download_desc_file();
      return;
    }
    report_done(false);
    return;
  }

  _uncompressed_archive.load_xml(xuncompressed_archive);
  _compressed_archive.load_xml(xcompressed_archive);

  // Now get all the extractable components.
  _unpack_size = 0;
  _extracts.clear();
  TiXmlElement *xextract = xpackage->FirstChildElement("extract");
  while (xextract != NULL) {
    FileSpec file;
    file.load_xml(xextract);
    _extracts.push_back(file);
    _unpack_size += file.get_size();
    xextract = xextract->NextSiblingElement("extract");
  }

  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  // Get the required packages.
  _requires.clear();
  TiXmlElement *xrequires = xpackage->FirstChildElement("requires");
  while (xrequires != NULL) {
    const char *package_name = xrequires->Attribute("name");
    const char *host_url = xrequires->Attribute("host");
    if (package_name != NULL && host_url != NULL) {
      const char *version = xrequires->Attribute("version");
      if (version == NULL) {
        version = "";
      }
      const char *seq = xrequires->Attribute("seq");
      if (seq == NULL) {
        seq = "";
      }
      P3DHost *host = inst_mgr->get_host(host_url);
      _requires.push_back(RequiredPackage(package_name, version, seq, host));
    }

    xrequires = xrequires->NextSiblingElement("requires");
  }

  if (inst_mgr->get_verify_contents() == P3D_VC_never) {
    // This means we'll just leave it at this
    // and assume that we're finished.
    report_done(true);
    return;
  }

  // Get a list of all of the files in the directory, so we can remove
  // files that don't belong.
  vector<string> contents, dirname_contents;
  inst_mgr->scan_directory_recursively(_package_dir, contents, dirname_contents);

  inst_mgr->remove_file_from_list(contents, _desc_file_basename);
  inst_mgr->remove_file_from_list(contents, _uncompressed_archive.get_filename());
  inst_mgr->remove_file_from_list(contents, "usage.xml");
  Extracts::iterator ei;
  for (ei = _extracts.begin(); ei != _extracts.end(); ++ei) {
    inst_mgr->remove_file_from_list(contents, (*ei).get_filename());
  }

  // Now, any files that are still in the contents list don't belong.
  // It's important to remove these files before we start verifying
  // the files that we expect to find here, in case there is a problem
  // with ambiguous filenames or something (e.g. case insensitivity).
  vector<string>::iterator ci;
  for (ci = contents.begin(); ci != contents.end(); ++ci) {
    string filename = (*ci);
    nout << "Removing " << filename << "\n";
    string pathname = _package_dir + "/" + filename;

#ifdef _WIN32
    // Windows can't delete a file if it's read-only.
    chmod(pathname.c_str(), 0644);
#endif
    unlink(pathname.c_str());
    _updated = true;
  }

  // Verify the uncompressed archive.
  bool all_extracts_ok = true;
  if (!_uncompressed_archive.quick_verify(_package_dir)) {
    nout << "File is incorrect: " << _uncompressed_archive.get_filename() << "\n";
    all_extracts_ok = false;
  }

  // Verify all of the extracts.
  for (ei = _extracts.begin(); ei != _extracts.end() && all_extracts_ok; ++ei) {
    if (!(*ei).quick_verify(_package_dir)) {
      nout << "File is incorrect: " << (*ei).get_filename() << "\n";
      all_extracts_ok = false;
    }

    // Make sure all extracts are still marked executable.
    chmod((*ei).get_pathname(_package_dir).c_str(), 0555);
  }

  if (all_extracts_ok) {
    // Great, we're ready to begin.
    nout << "All " << _extracts.size() << " extracts of " << _package_name
         << " seem good.\n";

    report_done(true);

  } else {
    // We need to get the file data still, but at least we know all
    // about it by this point.
    build_install_plans(doc);

    if (!_allow_data_download) {
      // Not authorized to start downloading yet; just report that
      // we're ready.
      report_info_ready();
    } else {
      // We've already been authorized to start downloading, so do it.
      follow_install_plans(true, false);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::clear_install_plans
//       Access: Private
//  Description: Empties _install_plans cleanly.
////////////////////////////////////////////////////////////////////
void P3DPackage::
clear_install_plans() {
  InstallPlans::iterator pi;
  for (pi = _install_plans.begin(); pi != _install_plans.end(); ++pi) {
    InstallPlan &plan = (*pi);
    InstallPlan::iterator si;
    for (si = plan.begin(); si != plan.end(); ++si) {
      InstallStep *step = (*si);
      delete step;
    }
  }

  _install_plans.clear();
  _computed_plan_size = false;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::build_install_plans
//       Access: Private
//  Description: Sets up _install_plans, a list of one or more "plans"
//               to download and install the package.
////////////////////////////////////////////////////////////////////
void P3DPackage::
build_install_plans(TiXmlDocument *doc) {
  clear_install_plans();

  if (_instances.empty()) {
    // Can't download without any instances.
    return;
  }

  if (_ready) {
    // Already downloaded.
    return;
  }

  _install_plans.push_front(InstallPlan());
  InstallPlan &plan = _install_plans.front();
  _computed_plan_size = false;

  bool needs_redownload = false;
  
  InstallStep *step;
  if (!_uncompressed_archive.quick_verify(_package_dir)) {
    // The uncompressed archive is no good.

    if (!_compressed_archive.quick_verify(_package_dir)) {
      // The compressed archive is no good either.  Download a new
      // compressed archive.
      needs_redownload = true;
      step = new InstallStepDownloadFile(this, _compressed_archive);
      plan.push_back(step);
    }

    // Uncompress the compressed archive to generate the uncompressed
    // archive.
    step = new InstallStepUncompressFile
      (this, _compressed_archive, _uncompressed_archive, true);
    plan.push_back(step);
  }

  // Unpack the uncompressed archive.
  step = new InstallStepUnpackArchive(this, _unpack_size);
  plan.push_back(step);

  if (needs_redownload) {
    // Since we need to do some downloading, try to build a plan that
    // involves downloading patches instead of downloading the whole
    // file.  This will be our first choice, plan A, if we can do it.

    // We'll need the md5 hash of the uncompressed archive currently
    // on disk.

    // Maybe we've already read the md5 hash and we have it stored here.
    const FileSpec *on_disk_ptr = _uncompressed_archive.get_actual_file();
    FileSpec on_disk;
    if (on_disk_ptr == NULL) {
      // If not, we have to go read it now.
      if (on_disk.read_hash(_uncompressed_archive.get_pathname(_package_dir))) {
        on_disk_ptr = &on_disk;
      }
    }

    if (on_disk_ptr != NULL) {
      P3DPatchFinder patch_finder;
      P3DPatchFinder::Patchfiles chain;
      if (patch_finder.get_patch_chain_to_current(chain, doc, *on_disk_ptr)) {
        nout << "Got patch chain of length " << chain.size() << "\n";

        // OK, we can create a plan to download and apply the patches.
        _install_plans.push_front(InstallPlan());
        InstallPlan &plan = _install_plans.front();

        P3DPatchFinder::Patchfiles::iterator pi;
        for (pi = chain.begin(); pi != chain.end(); ++pi) {
          P3DPatchFinder::Patchfile *patchfile = (*pi);

          // Download the patchfile
          step = new InstallStepDownloadFile(this, patchfile->_file);
          plan.push_back(step);

          // Uncompress it
          FileSpec new_file = patchfile->_file;
          string new_filename = new_file.get_filename();
          size_t dot = new_filename.rfind('.');
          assert(new_filename.substr(dot) == ".pz");
          new_filename = new_filename.substr(0, dot);
          new_file.set_filename(new_filename);
          step = new InstallStepUncompressFile
            (this, patchfile->_file, new_file, false);
          plan.push_back(step);

          // And apply it
          FileSpec source_file = patchfile->_source_file;
          FileSpec target_file = patchfile->_target_file;
          source_file.set_filename(_uncompressed_archive.get_filename());
          target_file.set_filename(_uncompressed_archive.get_filename());
          step = new InstallStepApplyPatch
            (this, new_file, source_file, target_file);
          plan.push_back(step);
        }

        // Unpack the uncompressed archive.
        step = new InstallStepUnpackArchive(this, _unpack_size);
        plan.push_back(step);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::follow_install_plans
//       Access: Private
//  Description: Performs the next step in the current install plan.
//
//               If download_finished is false, there is a pending
//               download that has not fully completed yet; otherwise,
//               download_finished should be set true.
//
//               If plan_failed is false, it means that the
//               top-of-stack plan is still good; if true, the
//               top-of-stack plan has failed and should be removed.
////////////////////////////////////////////////////////////////////
void P3DPackage::
follow_install_plans(bool download_finished, bool plan_failed) {
  if (!_allow_data_download || _failed) {
    // Not authorized yet, or something went wrong.
    return;
  }

  while (!_install_plans.empty()) {
    // Pull the next step off the current plan.

    InstallPlan &plan = _install_plans.front();

    if (!_computed_plan_size) {
      _total_plan_size = 0.0;
      _total_plan_completed = 0.0;
      InstallPlan::iterator si;
      for (si = plan.begin(); si != plan.end(); ++si) {
        double step_effort = (*si)->get_effort();
        _total_plan_size += step_effort;
        _total_plan_completed += (*si)->get_progress() * step_effort;
      }
      
      _download_progress = 0.0;
      if (_total_plan_size > 0.0) {
        _download_progress = _total_plan_completed / _total_plan_size;
      }
      _computed_plan_size = true;
      nout << "Selected install plan for " << get_package_name()
           << ": " << _total_plan_completed << " of "
           << _total_plan_size << "\n";
    }

    while (!plan.empty() && !plan_failed) {
      InstallStep *step = plan.front();
      _current_step_effort = step->get_effort();

      InstallToken token = step->do_step(download_finished);
      switch (token) {
      case IT_step_failed:
        // This plan has failed.
        plan_failed = true;
        break;

      case IT_terminate:
        // All plans have failed.
        _install_plans.clear();
        report_done(false);
        return;

      case IT_continue:
        // A callback hook has been attached; we'll come back later.
        return;

      case IT_needs_callback:
        // We need to install a callback hook and come back later.
        request_callback();
        return;

      case IT_step_complete:
        // So far, so good.  Go on to the next step.
        _total_plan_completed += _current_step_effort;
        delete step;
        plan.pop_front();
        break;
      }
    }

    if (!plan_failed) {
      // We've finished the plan successfully.
      clear_install_plans();
      report_done(true);
      return;
    }

    // That plan failed.  Go on to the next plan.
    nout << "Plan failed.\n";
    _install_plans.pop_front();
    _computed_plan_size = false;

    // The next plan is (so far as we know) still good.
    plan_failed = false;
  }

  // All plans failed.  Too bad for us.
  report_done(false);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::st_callback
//       Access: Private, Static
//  Description: This function is registered as the callback hook when
//               a package is in the middle of processing in a
//               sub-thread.
////////////////////////////////////////////////////////////////////
void P3DPackage::
st_callback(void *self) {
  ((P3DPackage *)self)->follow_install_plans(false, false);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::request_callback
//       Access: Private
//  Description: Requests that follow_install_plans() will be called
//               again in the future.
////////////////////////////////////////////////////////////////////
void P3DPackage::
request_callback() {
  Instances::iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    (*ii)->request_callback(&st_callback, this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::report_progress
//       Access: Private
//  Description: Reports the current install progress to all
//               interested instances.
////////////////////////////////////////////////////////////////////
void P3DPackage::
report_progress(P3DPackage::InstallStep *step) {
  if (_computed_plan_size) {
    double size = _total_plan_completed + _current_step_effort * step->get_progress();
    _download_progress = min(size / _total_plan_size, 1.0);
  
    Instances::iterator ii;
    for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
      (*ii)->report_package_progress(this, _download_progress);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::report_info_ready
//       Access: Private
//  Description: Called when the package information has been
//               successfully downloaded but activate_download() has
//               not yet been called, and the package is now idle,
//               waiting for activate_download() to be called.
////////////////////////////////////////////////////////////////////
void P3DPackage::
report_info_ready() {
  _info_ready = true;

  Instances::iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    (*ii)->report_package_info_ready(this);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::report_done
//       Access: Private
//  Description: Transitions the package to "ready" or "failure"
//               state, and reports this change to all the interested
//               instances.
////////////////////////////////////////////////////////////////////
void P3DPackage::
report_done(bool success) {
  // Don't call report_done() twice.
  if (_ready || _failed) {
    nout << get_package_name() << ": report_done() called twice\n";
    _failed = true;
    return;
  }

  if (success) {
    _info_ready = true;
    _ready = true;
    _failed = false;
  } else {
    _ready = false;
    _failed = true;
  }

  if (!_allow_data_download && success) {
    // If we haven't been authorized to start downloading yet, just
    // report that we're ready to start.
    Instances::iterator ii;
    for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
      (*ii)->report_package_info_ready(this);
    }

  } else {
    // Otherwise, we can report that we're fully downloaded.
    Instances::iterator ii;
    for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
      (*ii)->report_package_done(this, success);
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::start_download
//       Access: Private
//  Description: Initiates a download of the indicated file.  Returns
//               the new Download object.
////////////////////////////////////////////////////////////////////
P3DPackage::Download *P3DPackage::
start_download(P3DPackage::DownloadType dtype, const string &urlbase, 
               const string &pathname, const FileSpec &file_spec) {
  // Only one download should be active at a time
  assert(_active_download == NULL);
  // This can't happen! If verify_contents is set to P3D_VC_never, we're
  // not allowed to download anything, so we shouldn't get here
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  assert(inst_mgr->get_verify_contents() != P3D_VC_never);

  // We can't explicitly support partial downloads here, because
  // Mozilla provides no interface to ask for one.  We have to trust
  // that Mozilla's use of the browser cache handles partial downloads
  // for us automatically.

  // Delete the target file before we begin.
#ifdef _WIN32
  // Windows can't delete a file if it's read-only.
  chmod(pathname.c_str(), 0644);
#endif
  unlink(pathname.c_str());
    
  Download *download = new Download(this, dtype, file_spec);

  // Fill up the _try_urls vector for URL's to try getting this file
  // from, in reverse order.
  bool is_contents_file = (dtype == DT_contents_file || dtype == DT_redownload_contents_file);

  // The last thing we try is the actual authoritative host, with a
  // cache-busting query string.
  ostringstream strm;
  if (is_contents_file) {
    strm << _host->get_host_url_prefix();
  } else {
    strm << _host->get_download_url_prefix();
  }
  strm << urlbase << "?" << time(NULL);
  string url = strm.str();
  download->_try_urls.push_back(url);

  if (!is_contents_file) {
    // Before we try the cache-buster out of desperation, we try the
    // authoritative host, allowing caches.
    url = _host->get_download_url_prefix() + urlbase;
    download->_try_urls.push_back(url);

    // Before *that*, we try a couple of mirrors, chosen at random.
    vector<string> mirrors;
    _host->choose_random_mirrors(mirrors, 2);
    for (vector<string>::iterator si = mirrors.begin();
         si != mirrors.end(); 
         ++si) {
      url = (*si) + urlbase;
      download->_try_urls.push_back(url);
    }
  }

  if (dtype == DT_redownload_contents_file) {
    // When we're redownloading the contents file after a download
    // error, we always go straight to the authoritative host, not
    // even to the super-mirror.

  } else {
    // In other cases, if the "super mirror" is enabled, we try that
    // first.
    if (!inst_mgr->get_super_mirror().empty()) {
      string url = inst_mgr->get_super_mirror() + urlbase;
      download->_try_urls.push_back(url);
    }
  }

  if (download->_try_urls.size() == 1) {
    // If we only ended up with only one URL on the try list, then try
    // it twice, for a bit of redundancy in case there's a random
    // network hiccup or something.

    // Save a copy into its own string object first to avoid
    // self-dereferencing errors in the push_back() method.
    string url = download->_try_urls[0];
    download->_try_urls.push_back(url);
  }

  // OK, start the download.
  assert(!download->_try_urls.empty());
  url = download->_try_urls.back();
  download->_try_urls.pop_back();
  download->set_url(url);
  download->set_filename(pathname);

  set_active_download(download);
  assert(!_instances.empty());

  _instances[0]->start_download(download);
  return download;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::set_active_download
//       Access: Private
//  Description: Changes _active_download to point to the indicated
//               object, respecting reference counts.
////////////////////////////////////////////////////////////////////
void P3DPackage::
set_active_download(Download *download) {
  if (_active_download != download) {
    if (_active_download != NULL) {
      p3d_unref_delete(_active_download);
    }
    _active_download = download;
    if (_active_download != NULL) {
      _active_download->ref();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::set_saved_download
//       Access: Private
//  Description: Changes _saved_download to point to the indicated
//               object, respecting reference counts.
////////////////////////////////////////////////////////////////////
void P3DPackage::
set_saved_download(Download *download) {
  if (_saved_download != download) {
    if (_saved_download != NULL) {
      p3d_unref_delete(_saved_download);
    }
    _saved_download = download;
    if (_saved_download != NULL) {
      _saved_download->ref();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::is_extractable
//       Access: Private
//  Description: Returns true if the name file is on the extract list,
//               false otherwise.  If true, fills in the FileSpec with
//               the file's information.
////////////////////////////////////////////////////////////////////
bool P3DPackage::
is_extractable(FileSpec &file, const string &filename) const {
  Extracts::const_iterator ei;
  for (ei = _extracts.begin(); ei != _extracts.end(); ++ei) {
    if ((*ei).get_filename() == filename) {
      file = (*ei);
      return true;
    }
  }

  return false;
}


////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::instance_terminating
//       Access: Private
//  Description: Called when P3D_RC_shutdown is received by any
//               Download object, which indicates that the instance
//               owning this download object is terminating and we
//               should either find a new instance or abort the
//               download.
//
//               The return value is true if a new instance is
//               available, or false if not.
////////////////////////////////////////////////////////////////////
bool P3DPackage::
instance_terminating(P3DInstance *instance) {
  if (_instances.empty() ||
      (_instances.size() == 1 && instance == _instances[0])) {
    // No other instances.
    return false;
  }

  // There are more instances available to continue this download;
  // pick one of them.  Move this one to the end of the list.
  Instances::iterator ii = find(_instances.begin(), _instances.end(), instance);
  if (ii != _instances.end()) {
    _instances.erase(ii);
    _instances.push_back(instance);
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::set_fullname
//       Access: Private
//  Description: Assigns _package_fullname to the appropriate
//               combination of name, version, and platform.
////////////////////////////////////////////////////////////////////
void P3DPackage::
set_fullname() {
  _package_fullname = _package_name;
  if (!_package_version.empty()) {
    _package_fullname += string(".") + _package_version;
  }
  if (!_package_platform.empty()) {
    _package_fullname += string(".") + _package_platform;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Download::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::Download::
Download(P3DPackage *package, DownloadType dtype, const FileSpec &file_spec) :
  _package(package),
  _dtype(dtype),
  _file_spec(file_spec)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Download::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::Download::
Download(const P3DPackage::Download &copy) :
  P3DFileDownload(copy),
  _try_urls(copy._try_urls),
  _package(copy._package),
  _dtype(copy._dtype),
  _file_spec(copy._file_spec)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Download::download_progress
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DPackage::Download::
download_progress() {
  P3DFileDownload::download_progress();
  assert(_package->_active_download == this);

  switch (_dtype) {
  case DT_contents_file:
  case DT_redownload_contents_file:
    break;

  case DT_desc_file:
    break;

  case DT_install_step:
    _package->follow_install_plans(false, false);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Download::download_finished
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DPackage::Download::
download_finished(bool success) {
  P3DFileDownload::download_finished(success);
  assert(_package->_active_download == this);
  if (get_ref_count() == 1) {
    // No one cares anymore.
    nout << "No one cares about " << get_url() << "\n";
    _package->set_active_download(NULL);
    return;
  }

  _package->set_active_download(NULL);
  assert(get_ref_count() > 0);

  if (success && !_file_spec.get_filename().empty()) {
    // We think we downloaded it correctly.  Check the hash to be
    // sure.
    if (!_file_spec.full_verify(_package->_package_dir)) {
      nout << "After downloading " << get_url()
           << ", failed hash check\n";
      nout << "expected: ";
      _file_spec.output_hash(nout);
      nout << "\n";
      if (_file_spec.get_actual_file() != (FileSpec *)NULL) {
        nout << "     got: ";
        _file_spec.get_actual_file()->output_hash(nout);
        nout << "\n";
      }

      success = false;
    }
  }

  close_file();

  if (!success) {
    if (get_download_terminated()) {
      // Short-circuit the exit.
      _try_urls.clear();
      resume_download_finished(false);
      return;
    }

    // Maybe it failed because our contents.xml file is out-of-date.
    // Go try to freshen it.
    bool is_contents_file = (_dtype == DT_contents_file || _dtype == DT_redownload_contents_file);
    if (!is_contents_file) {
      _package->redownload_contents_file(this);
      return;
    }
  }

  // Carry on.
  resume_download_finished(success);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Download::resume_download_finished
//       Access: Public
//  Description: Continuing the work begun in download_finished().
//               This is a separate entry point so that it can be
//               called again after determining that the host's
//               contents.xml file is *not* stale.
////////////////////////////////////////////////////////////////////
void P3DPackage::Download::
resume_download_finished(bool success) {
  if (!success && !_try_urls.empty()) {
    // Try the next mirror.
    string url = _try_urls.back();
    _try_urls.pop_back();

    clear();
    set_url(url);
    set_filename(get_filename());
    _package->set_active_download(this);

    assert(!_package->_instances.empty());
    _package->_instances[0]->start_download(this);
    return;
  }

  switch (_dtype) {
  case DT_contents_file:
    _package->contents_file_download_finished(success);
    break;

  case DT_redownload_contents_file:
    _package->contents_file_redownload_finished(success);
    break;

  case DT_desc_file:
    _package->desc_file_download_finished(success);
    break;

  case DT_install_step:
    _package->follow_install_plans(true, !success);
    break;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStep::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallStep::
InstallStep(P3DPackage *package, size_t bytes, double factor) :
  _package(package),
  _bytes_needed(bytes),
  _bytes_done(0),
  _bytes_factor(factor)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStep::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallStep::
~InstallStep() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepDownloadFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallStepDownloadFile::
InstallStepDownloadFile(P3DPackage *package, const FileSpec &file) :
  InstallStep(package, file.get_size(), _download_factor),
  _file(file)
{
  _urlbase = _package->get_desc_file_dirname();
  _urlbase += "/";
  _urlbase += _file.get_filename();
  
  _pathname = _package->get_package_dir() + "/" + _file.get_filename();
    
  _download = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepDownloadFile::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallStepDownloadFile::
~InstallStepDownloadFile() {
  if (_download != NULL) {
    p3d_unref_delete(_download);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepDownloadFile::do_step
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallToken P3DPackage::InstallStepDownloadFile::
do_step(bool download_finished) {
  if (_download == NULL) {
    // First, we have to start the download going.
    assert(_package->_active_download == NULL);

    _download = _package->start_download(DT_install_step, _urlbase, 
                                         _pathname, _file);
    assert(_download != NULL);
    _download->ref();
  }

  _bytes_done = _download->get_total_data();
  report_step_progress();

  if (!_download->get_download_finished() || !download_finished) {
    // Wait for it.
    return IT_continue;
  }

  if (_download->get_download_success()) {
    // The Download object has already validated the hash.
    _package->_updated = true;
    return IT_step_complete;

  } else if (_download->get_download_terminated()) {
    // The download was interrupted because its instance is shutting
    // down.  Don't try any other plans, unless we have some more
    // instances.
    P3DInstance *instance = _download->get_instance();
    if (!_package->instance_terminating(instance)) {
      // That was the only instance referencing this package, so stop
      // the download.
      nout << "Terminating download of " << _urlbase << "\n";
      return IT_terminate;
    }
    nout << "Restarting download of " << _urlbase << " on new instance\n";

    p3d_unref_delete(_download);
    _download = NULL;
    return IT_continue;

  } else {
    // The Download object has already tried all of the mirrors, and
    // they all failed.
    return IT_step_failed;
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepDownloadFile::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DPackage::InstallStepDownloadFile::
output(ostream &out) {
  out << "InstallStepDownloadFile("  << _package->get_package_name()
      << ", " << _file.get_filename() << ")";
}


////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepThreaded::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallStepThreaded::
InstallStepThreaded(P3DPackage *package, size_t bytes, double factor) :
  InstallStep(package, bytes, factor)
{
  INIT_THREAD(_thread);
  INIT_LOCK(_thread_lock);
  _thread_started = false;
  _thread_token = IT_needs_callback;
  _thread_bytes_done = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepThreaded::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallStepThreaded::
~InstallStepThreaded() {
  if (_thread_started) {
    JOIN_THREAD(_thread);
    _thread_started = false;
  }
  DESTROY_LOCK(_thread_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepThreaded::do_step
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallToken P3DPackage::InstallStepThreaded::
do_step(bool download_finished) {
  // This method is called within the main thread.  It simply checks
  // the thread status, and returns.

  // Spawn a thread and wait for it to finish.
  if (!_thread_started) {
    nout << "Spawning thread to handle " << _package->get_package_name() << "\n";
    _thread_started = true;
    SPAWN_THREAD(_thread, thread_main, this);
  }

  InstallToken token;
  bool made_progress = false;

  ACQUIRE_LOCK(_thread_lock);
  token = _thread_token;
  if (_bytes_done != _thread_bytes_done) {
    _bytes_done = _thread_bytes_done;
    made_progress = true;
  }
  RELEASE_LOCK(_thread_lock);

  if (made_progress) {
    report_step_progress();
  }

  return token;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepThreaded::thread_main
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DPackage::InstallStepThreaded::
thread_main() {
  // This method is called within the sub-thread.  It calls
  // thread_step() to do its work.

  InstallToken token = IT_needs_callback;
  do {
    // Perform the nested step, then update the token.
    token = thread_step();

    ACQUIRE_LOCK(_thread_lock);
    _thread_token = token;
    RELEASE_LOCK(_thread_lock);
    
    // Do it again if needed.
  } while (token == IT_needs_callback);

  // All done.
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepThreaded::thread_set_bytes_done
//       Access: Public
//  Description: Should be called from time to time within the
//               sub-thread to update the number of bytes processed.
////////////////////////////////////////////////////////////////////
void P3DPackage::InstallStepThreaded::
thread_set_bytes_done(size_t bytes_done) {
  ACQUIRE_LOCK(_thread_lock);
  _thread_bytes_done = bytes_done;
  RELEASE_LOCK(_thread_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepThreaded::thread_add_bytes_done
//       Access: Public
//  Description: Should be called from time to time within the
//               sub-thread to update the number of bytes processed.
////////////////////////////////////////////////////////////////////
void P3DPackage::InstallStepThreaded::
thread_add_bytes_done(size_t bytes_done) {
  ACQUIRE_LOCK(_thread_lock);
  _thread_bytes_done += bytes_done;
  RELEASE_LOCK(_thread_lock);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepUncompressFile::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallStepUncompressFile::
InstallStepUncompressFile(P3DPackage *package, const FileSpec &source,
                          const FileSpec &target, bool verify_target) :
  InstallStepThreaded(package, target.get_size(), _uncompress_factor),
  _source(source),
  _target(target),
  _verify_target(verify_target)
{
}


////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepUncompressFile::thread_step
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallToken P3DPackage::InstallStepUncompressFile::
thread_step() {
  string source_pathname = _package->get_package_dir() + "/" + _source.get_filename();
  string target_pathname = _package->get_package_dir() + "/" + _target.get_filename();

  ifstream source;
#ifdef _WIN32
  wstring source_pathname_w;
  if (string_to_wstring(source_pathname_w, source_pathname)) {
    source.open(source_pathname_w.c_str(), ios::in | ios::binary);
  }
#else // _WIN32
  source.open(source_pathname.c_str(), ios::in | ios::binary);
#endif  // _WIN32
  if (!source) {
    nout << "Couldn't open " << source_pathname << "\n";
    return IT_step_failed;
  }

  if (!mkfile_complete(target_pathname, nout)) {
    return IT_step_failed;
  }

  ofstream target;
#ifdef _WIN32
  wstring target_pathname_w;
  if (string_to_wstring(target_pathname_w, target_pathname)) {
    target.open(target_pathname_w.c_str(), ios::out | ios::binary);
  }
#else // _WIN32
  target.open(target_pathname.c_str(), ios::out | ios::binary);
#endif  // _WIN32
  if (!target) {
    nout << "Couldn't write to " << target_pathname << "\n";
    return IT_step_failed;
  }
  
  static const int decompress_buffer_size = 81920;
  char decompress_buffer[decompress_buffer_size];
  static const int write_buffer_size = 81920;
  char write_buffer[write_buffer_size];

  z_stream z;
  z.next_in = Z_NULL;
  z.avail_in = 0;
  z.next_out = Z_NULL;
  z.avail_out = 0;
  z.zalloc = Z_NULL;
  z.zfree = Z_NULL;
  z.opaque = Z_NULL;
  z.msg = (char *)"no error message";

  bool eof = false;
  int flush = 0;

  source.read(decompress_buffer, decompress_buffer_size);
  streamsize read_count = source.gcount();
  eof = (read_count == 0 || source.eof() || source.fail());
  
  z.next_in = (Bytef *)decompress_buffer;
  z.avail_in = (size_t)read_count;

  int result = inflateInit(&z);
  if (result < 0) {
    nout << z.msg << "\n";
    return IT_step_failed;
  }

  while (true) {
    if (z.avail_in == 0 && !eof) {
      source.read(decompress_buffer, decompress_buffer_size);
      streamsize read_count = source.gcount();
      eof = (read_count == 0 || source.eof() || source.fail());
        
      z.next_in = (Bytef *)decompress_buffer;
      z.avail_in = (size_t)read_count;
    }

    z.next_out = (Bytef *)write_buffer;
    z.avail_out = write_buffer_size;
    int result = inflate(&z, flush);
    if (z.avail_out < write_buffer_size) {
      target.write(write_buffer, write_buffer_size - z.avail_out);
      if (!target) {
        nout << "Couldn't write entire file to " << target_pathname << "\n";
        return IT_step_failed;
      }

      thread_add_bytes_done(write_buffer_size - z.avail_out);
    }

    if (result == Z_STREAM_END) {
      // Here's the end of the file.
      break;

    } else if (result == Z_BUF_ERROR && flush == 0) {
      // We might get this if no progress is possible, for instance if
      // the input stream is truncated.  In this case, tell zlib to
      // dump everything it's got.
      flush = Z_FINISH;

    } else if (result < 0) {
      nout << z.msg << "\n";
      inflateEnd(&z);
      return IT_step_failed;
    }
  }

  result = inflateEnd(&z);
  if (result < 0) {
    nout << z.msg << "\n";
    return IT_step_failed;
  }

  source.close();
  target.close();

  if (_verify_target && !_target.full_verify(_package->get_package_dir())) {
    nout << "after uncompressing " << target_pathname
         << ", failed hash check\n";
    return IT_step_failed;
  }

  // Now that we've verified the target, make it read-only.
  chmod(target_pathname.c_str(), 0444);

  // Now we can safely remove the source.
#ifdef _WIN32
  chmod(source_pathname.c_str(), 0644);
#endif
  unlink(source_pathname.c_str());

  // All done uncompressing.
  _package->_updated = true;
  return IT_step_complete;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepUncompressFile::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DPackage::InstallStepUncompressFile::
output(ostream &out) {
  out << "InstallStepUncompressFile(" << _package->get_package_name()
      << ", " << _source.get_filename() << ", " << _target.get_filename()
      << ", " << _verify_target << ")";
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepUnpackArchive::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallStepUnpackArchive::
InstallStepUnpackArchive(P3DPackage *package, size_t unpack_size) :
  InstallStepThreaded(package, unpack_size, _unpack_factor)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepUnpackArchive::thread_step
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallToken P3DPackage::InstallStepUnpackArchive::
thread_step() {
  string source_pathname = _package->get_archive_file_pathname();
  P3DMultifileReader reader;

  if (!reader.open_read(source_pathname)) {
    nout << "Couldn't read " << source_pathname << "\n";
    return IT_step_failed;
  }

  if (!reader.extract_all(_package->get_package_dir(), _package, this)) {
    nout << "Failure extracting " << source_pathname << "\n";
    return IT_step_failed;
  }

  _package->_updated = true;
  return IT_step_complete;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepUnpackArchive::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DPackage::InstallStepUnpackArchive::
output(ostream &out) {
  out << "InstallStepUnpackArchive(" << _package->get_package_name() << ")";
}


////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepApplyPatch::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallStepApplyPatch::
InstallStepApplyPatch(P3DPackage *package, const FileSpec &patchfile,
                      const FileSpec &source, const FileSpec &target) :
  InstallStepThreaded(package, target.get_size(), _patch_factor),
  _reader(package->get_package_dir(), patchfile, source, target)
{
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepApplyPatch::thread_step
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::InstallToken P3DPackage::InstallStepApplyPatch::
thread_step() {
  // Open the patchfile
  if (!_reader.open_read()) {
    _reader.close();
    return IT_step_failed;
  }

  // Apply the patch.
  while (_reader.step()) {
    size_t bytes_written = _reader.get_bytes_written();
    thread_set_bytes_done(bytes_written);
  }

  // Close and verify.
  _reader.close();

  if (!_reader.get_success()) {
    nout << "Patching failed\n";
    return IT_step_failed;
  }

  _package->_updated = true;
  return IT_step_complete;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::InstallStepApplyPatch::output
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DPackage::InstallStepApplyPatch::
output(ostream &out) {
  out << "InstallStepApplyPatch(" << _package->get_package_name() << ")";
}
