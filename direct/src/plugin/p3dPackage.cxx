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
#include "mkdir_complete.h"

#include "zlib.h"

#include <algorithm>
#include <fstream>

#ifdef _WIN32
#include <io.h>    // chmod()
#endif

// The relative breakdown of the full install process.  Each phase is
// worth this fraction of the total movement of the progress bar.
static const double download_portion = 0.9;
static const double uncompress_portion = 0.05;
static const double extract_portion = 0.05;

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::
P3DPackage(P3DHost *host, const string &package_name,
           const string &package_version, const string &alt_host) :
  _host(host),
  _package_name(package_name),
  _package_version(package_version),
  _alt_host(alt_host)
{
  _package_fullname = _package_name;
  if (!_package_version.empty()) {
    _package_fullname += string(".") + _package_version;
  }
  _patch_version = 0;

  // This is set true if the package is a "solo", i.e. a single
  // file, instead of an xml file and a multifile to unpack.
  _package_solo = false;

  _xconfig = NULL;
  _temp_contents_file = NULL;

  _info_ready = false;
  _download_size = 0;
  _allow_data_download = false;
  _ready = false;
  _failed = false;
  _active_download = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Destructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::
~P3DPackage() {
  // Tell any pending callbacks that we're no good any more.
  report_done(false);

  if (_xconfig != NULL) {
    delete _xconfig;
    _xconfig = NULL;
  }

  // Cancel any pending download.
  if (_active_download != NULL) {
    _active_download->cancel();
    delete _active_download;
    _active_download = NULL;
  }

  if (_temp_contents_file != NULL) {
    delete _temp_contents_file;
    _temp_contents_file = NULL;
  }

  assert(_instances.empty());
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
  if (_allow_data_download) {
    // Already activated.
  }

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
      begin_data_download();
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
//  Description: Specifies an instance that may be responsible for
//               downloading this package.
////////////////////////////////////////////////////////////////////
void P3DPackage::
add_instance(P3DInstance *inst) {
  _instances.push_back(inst);

  begin_info_download();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::remove_instance
//       Access: Public
//  Description: Indicates that the given instance will no longer be
//               responsible for downloading this package.
////////////////////////////////////////////////////////////////////
void P3DPackage::
remove_instance(P3DInstance *inst) {
  assert(!_instances.empty());

  if (inst == _instances[0]) {
    // This was the primary instance.  Cancel any pending download and
    // move to the next instance.
    if (_active_download != NULL) {
      _active_download->cancel();
      delete _active_download;
      _active_download = NULL;
    }
  }

  Instances::iterator ii = find(_instances.begin(), _instances.end(), inst);
  assert(ii != _instances.end());
  _instances.erase(ii);

  begin_info_download();
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
//               This is only done for the first package, and only if
//               the host doesn't have the file already.
////////////////////////////////////////////////////////////////////
void P3DPackage::
download_contents_file() {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  if (!_host->has_contents_file() && !inst_mgr->get_verify_contents()) {
    // If we're allowed to read a contents file without checking the
    // server first, try it now.
    _host->read_contents_file();
  }

  if (_host->has_contents_file()) {
    // We've already got a contents.xml file; go straight to the
    // package desc file.
    host_got_contents_file();
    return;
  }

  // Get the URL for contents.xml.
  ostringstream strm;
  strm << "contents.xml";
  // Append a uniquifying query string to the URL to force the
  // download to go all the way through any caches.  We use the time
  // in seconds; that's unique enough.
  strm << "?" << time(NULL);
  string urlbase = strm.str();

  // Download contents.xml to a temporary filename first, in case
  // multiple packages are downloading it simultaneously.
  if (_temp_contents_file != NULL) {
    delete _temp_contents_file;
    _temp_contents_file = NULL;
  }
  _temp_contents_file = new P3DTemporaryFile(".xml");

  start_download(DT_contents_file, urlbase, _temp_contents_file->get_filename(), 
                 FileSpec());
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::contents_file_download_finished
//       Access: Private
//  Description: Called when the contents.xml file has been fully
//               downloaded.
////////////////////////////////////////////////////////////////////
void P3DPackage::
contents_file_download_finished(bool success) {
  if (!_host->has_contents_file()) {
    if (!success || !_host->read_contents_file(_temp_contents_file->get_filename())) {
      nout << "Couldn't read " << *_temp_contents_file << "\n";

      // Maybe we can read an already-downloaded contents.xml file.
      string standard_filename = _host->get_host_dir() + "/contents.xml";
      if (!_host->read_contents_file(standard_filename)) {
        // Couldn't even read that.  Fail.
        report_done(false);
        delete _temp_contents_file;
        _temp_contents_file = NULL;
        return;
      }
    }
  }
    
  // The file is correctly installed by now; we can remove the
  // temporary file.
  delete _temp_contents_file;
  _temp_contents_file = NULL;

  host_got_contents_file();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::host_got_contents_file
//       Access: Private
//  Description: We come here when we've successfully downloaded and
//               read the host's contents.xml file.
////////////////////////////////////////////////////////////////////
void P3DPackage::
host_got_contents_file() {
  if (!_alt_host.empty()) {
    // If we have an alt host specification, maybe we need to change
    // the host now.
    P3DHost *new_host = _host->get_alt_host(_alt_host);
    nout << "Migrating " << get_package_name() << " to alt_host " 
         << _alt_host << ": " << new_host->get_host_url() << "\n";
    if (new_host != _host) {
      _host->migrate_package(this, _alt_host, new_host);
      _host = new_host;
    }

    // Clear the alt_host string now that we're migrated to our final
    // host.
    _alt_host.clear();

    if (!_host->has_contents_file()) {
      // Now go back and get the contents.xml file for the new host.
      download_contents_file();
      return;
    }
  }

  // Now that we have a valid host, we can define the _package_dir.
  _package_dir = _host->get_host_dir() + string("/") + _package_name;
  if (!_package_version.empty()) {
    _package_dir += string("/") + _package_version;
  }

  // Ensure the package directory exists; create it if it does not.
  mkdir_complete(_package_dir, nout);

  download_desc_file();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::download_desc_file
//       Access: Private
//  Description: Starts downloading the desc file for the package, if
//               it's needed; or read to local version if it's fresh
//               enough.
////////////////////////////////////////////////////////////////////
void P3DPackage::
download_desc_file() {
  // Attempt to check the desc file for freshness.  If it already
  // exists, and is consistent with the server contents file, we don't
  // need to re-download it.
  if (!_host->get_package_desc_file(_desc_file, _package_platform, 
                                    _package_solo,
                                    _package_name, _package_version)) {
    nout << "Couldn't find package " << _package_fullname
         << " in contents file.\n";
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
  // it has locally, because we strip out the platform directory
  // locally.
  FileSpec local_desc_file = _desc_file;
  local_desc_file.set_filename(_desc_file_basename);
  _desc_file_pathname = local_desc_file.get_pathname(_package_dir);

  if (!local_desc_file.full_verify(_package_dir)) {
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

  // Now that we've downloaded the desc file, make it read-only.
  chmod(_desc_file_pathname.c_str(), 0444);

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
  TiXmlElement *xuncompressed_archive = NULL;
  TiXmlElement *xcompressed_archive = NULL;
  
  if (xpackage != NULL) {
    xpackage->Attribute("patch_version", &_patch_version);

    xuncompressed_archive = xpackage->FirstChildElement("uncompressed_archive");
    xcompressed_archive = xpackage->FirstChildElement("compressed_archive");

    TiXmlElement *xconfig = xpackage->FirstChildElement("config");
    if (xconfig != NULL) {
      const char *display_name_cstr = xconfig->Attribute("display_name");
      if (display_name_cstr != NULL) {
        _package_display_name = display_name_cstr;
      }

      // Save the config entry within this class for others to query.
      _xconfig = (TiXmlElement *)xconfig->Clone();
    }
  }

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
  _extracts.clear();
  TiXmlElement *extract = xpackage->FirstChildElement("extract");
  while (extract != NULL) {
    FileSpec file;
    file.load_xml(extract);
    _extracts.push_back(file);
    extract = extract->NextSiblingElement("extract");
  }

  // Get a list of all of the files in the directory, so we can remove
  // files that don't belong.
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  vector<string> contents;
  inst_mgr->scan_directory_recursively(_package_dir, contents);

  inst_mgr->remove_file_from_list(contents, _desc_file_basename);
  inst_mgr->remove_file_from_list(contents, _uncompressed_archive.get_filename());
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
  }

  if (all_extracts_ok) {
    // Great, we're ready to begin.
    nout << "All " << _extracts.size() << " extracts of " << _package_name
         << " seem good.\n";

    report_done(true);

  } else {
    // We need to get the file data still, but at least we know all
    // about it by this point.
    if (!_allow_data_download) {
      // Not authorized to start downloading yet; just report that
      // we're ready.
      report_info_ready();
    } else {
      // We've already been authorized to start downloading, so do it.
      begin_data_download();
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::begin_data_download
//       Access: Private
//  Description: Begins downloading and installing the package data
//               itself, if needed.
////////////////////////////////////////////////////////////////////
void P3DPackage::
begin_data_download() {
  if (_instances.empty()) {
    // Can't download without any instances.
    return;
  }

  if (_ready) {
    // Already downloaded.
    return;
  }

  if (_active_download != NULL) {
    // In the middle of downloading.
    return;
  }

  if (!_allow_data_download) {
    // Not authorized yet.
    return;
  }

  if (_uncompressed_archive.quick_verify(_package_dir)) {
    // We need to re-extract the archive.
    extract_archive();

  } else if (_compressed_archive.quick_verify(_package_dir)) {
    // We need to uncompress the archive.
    uncompress_archive();

  } else {
    // Shoot, we need to download the archive.
    download_compressed_archive();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::download_compressed_archive
//       Access: Private
//  Description: Starts downloading the archive file for the package.
////////////////////////////////////////////////////////////////////
void P3DPackage::
download_compressed_archive() {
  string urlbase = _desc_file_dirname;
  urlbase += "/";
  urlbase += _compressed_archive.get_filename();

  string target_pathname = _package_dir + "/" + _compressed_archive.get_filename();

  start_download(DT_compressed_archive, urlbase, target_pathname, 
                 _compressed_archive);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::compressed_archive_download_progress
//       Access: Private
//  Description: Called as the file is downloaded.
////////////////////////////////////////////////////////////////////
void P3DPackage::
compressed_archive_download_progress(double progress) {
  report_progress(download_portion * progress);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::compressed_archive_download_finished
//       Access: Private
//  Description: Called when the desc file has been fully downloaded.
////////////////////////////////////////////////////////////////////
void P3DPackage::
compressed_archive_download_finished(bool success) {
  if (!success) {
    report_done(false);
    return;
  }

  // Go on to uncompress the archive.
  uncompress_archive();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::uncompress_archive
//       Access: Private
//  Description: Uncompresses the archive file.
////////////////////////////////////////////////////////////////////
void P3DPackage::
uncompress_archive() {
  string source_pathname = _package_dir + "/" + _compressed_archive.get_filename();
  string target_pathname = _package_dir + "/" + _uncompressed_archive.get_filename();

  ifstream source(source_pathname.c_str(), ios::in | ios::binary);
  if (!source) {
    nout << "Couldn't open " << source_pathname << "\n";
    report_done(false);
    return;
  }

  if (!mkfile_complete(target_pathname, nout)) {
    report_done(false);
    return;
  }

  ofstream target(target_pathname.c_str(), ios::out | ios::binary);
  if (!target) {
    nout << "Couldn't write to " << target_pathname << "\n";
    report_done(false);
    return;
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
  size_t read_count = source.gcount();
  eof = (read_count == 0 || source.eof() || source.fail());
  
  z.next_in = (Bytef *)decompress_buffer;
  z.avail_in = read_count;

  int result = inflateInit(&z);
  if (result < 0) {
    nout << z.msg << "\n";
    report_done(false);
    return;
  }

  size_t total_out = 0;
  while (true) {
    if (z.avail_in == 0 && !eof) {
      source.read(decompress_buffer, decompress_buffer_size);
      size_t read_count = source.gcount();
      eof = (read_count == 0 || source.eof() || source.fail());
        
      z.next_in = (Bytef *)decompress_buffer;
      z.avail_in = read_count;
    }

    z.next_out = (Bytef *)write_buffer;
    z.avail_out = write_buffer_size;
    int result = inflate(&z, flush);
    if (z.avail_out < write_buffer_size) {
      target.write(write_buffer, write_buffer_size - z.avail_out);
      if (!target) {
        nout << "Couldn't write entire file to " << target_pathname << "\n";
        report_done(false);
        return;
      }
      total_out += (write_buffer_size - z.avail_out);
      if (_uncompressed_archive.get_size() != 0) {
        double progress = (double)total_out / (double)_uncompressed_archive.get_size();
        progress = min(progress, 1.0);
        report_progress(download_portion + uncompress_portion * progress);
      }
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
      report_done(false);
      return;
    }
  }

  result = inflateEnd(&z);
  if (result < 0) {
    nout << z.msg << "\n";
    report_done(false);
    return;
  }

  source.close();
  target.close();

  if (!_uncompressed_archive.full_verify(_package_dir)) {
    nout << "after uncompressing " << target_pathname
         << ", failed hash check\n";
    report_done(false);
    return;
  }

  // Now that we've verified the archive, make it read-only.
  chmod(target_pathname.c_str(), 0444);

  // Now we can safely remove the compressed archive.
#ifdef _WIN32
  chmod(source_pathname.c_str(), 0644);
#endif
  unlink(source_pathname.c_str());

  // All done uncompressing.
  extract_archive();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::extract_archive
//       Access: Private
//  Description: Extracts the components from the archive file.
////////////////////////////////////////////////////////////////////
void P3DPackage::
extract_archive() {
  string source_pathname = _package_dir + "/" + _uncompressed_archive.get_filename();
  P3DMultifileReader reader;
  if (!reader.open_read(source_pathname)) {
    nout << "Couldn't read " << _uncompressed_archive.get_filename() << "\n";
    report_done(false);
    return;
  }

  if (!reader.extract_all(_package_dir, this, 
                          download_portion + uncompress_portion, 
                          extract_portion)) {
    nout << "Failure extracting " << _uncompressed_archive.get_filename()
         << "\n";
    report_done(false);
    return;
  }

  report_done(true);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::report_progress
//       Access: Private
//  Description: Reports the indicated install progress to all
//               interested instances.
////////////////////////////////////////////////////////////////////
void P3DPackage::
report_progress(double progress) {
  Instances::iterator ii;
  for (ii = _instances.begin(); ii != _instances.end(); ++ii) {
    (*ii)->report_package_progress(this, progress);
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
  _download_size = _compressed_archive.get_size();

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
    // report that we're ready to start, but that we don't have to
    // download anything.
    _download_size = 0;
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
//  Description: Initiates a download of the indicated file.
////////////////////////////////////////////////////////////////////
void P3DPackage::
start_download(P3DPackage::DownloadType dtype, const string &urlbase, 
               const string &pathname, const FileSpec &file_spec) {
  // Only one download should be active at a time
  assert(_active_download == NULL);

  // TODO: support partial downloads.
  static const bool allow_partial = false;
  if (!allow_partial) {
#ifdef _WIN32
    // Windows can't delete a file if it's read-only.
    chmod(pathname.c_str(), 0644);
#endif
    unlink(pathname.c_str());
  } else {
    // Make sure the file is writable.
    chmod(pathname.c_str(), 0644);
  }
    
  Download *download = new Download(this, dtype, file_spec);

  // Fill up the _try_urls vector for URL's to try getting this file
  // from, in reverse order.

  // The last thing we try is the actual authoritative host.
  string url = _host->get_host_url_prefix() + urlbase;
  download->_try_urls.push_back(url);

  // The first thing we try is a couple of mirrors, chosen at random
  // (except for the contents.xml file, which always goes straight to
  // the host).
  if (dtype != DT_contents_file) {
    vector<string> mirrors;
    _host->choose_random_mirrors(mirrors, 2);
    for (vector<string>::iterator si = mirrors.begin();
         si != mirrors.end(); 
         ++si) {
      url = (*si) + urlbase;
      download->_try_urls.push_back(url);
    }
  }

  // OK, start the download.
  assert(!download->_try_urls.empty());
  url = download->_try_urls.back();
  download->_try_urls.pop_back();
  download->set_url(url);
  download->set_filename(pathname);

  _active_download = download;
  assert(!_instances.empty());

  _instances[0]->start_download(download);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::is_extractable
//       Access: Private
//  Description: Returns true if the name file is on the extract list,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DPackage::
is_extractable(const string &filename) const {
  Extracts::const_iterator ei;
  for (ei = _extracts.begin(); ei != _extracts.end(); ++ei) {
    if ((*ei).get_filename() == filename) {
      return true;
    }
  }

  return false;
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
  case DT_desc_file:
    break;

  case DT_compressed_archive:
    _package->compressed_archive_download_progress(get_download_progress());
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
  _package->_active_download = NULL;

  if (success && !_file_spec.get_filename().empty()) {
    // We think we downloaded it correctly.  Check the hash to be
    // sure.
    if (!_file_spec.full_verify(_package->_package_dir)) {
      nout << "After downloading " << get_url()
           << ", failed hash check\n";
      success = false;
    }
  }

  if (!success && !_try_urls.empty()) {
    // Well, that URL failed, but we can try another mirror.
    close_file();
    string url = _try_urls.back();
    _try_urls.pop_back();

    Download *new_download = new Download(*this);
    new_download->set_filename(get_filename());
    new_download->set_url(url);

    _package->_active_download = new_download;

    assert(!_package->_instances.empty());
    _package->_instances[0]->start_download(new_download);
    return;
  }

  switch (_dtype) {
  case DT_contents_file:
    _package->contents_file_download_finished(success);
    break;

  case DT_desc_file:
    _package->desc_file_download_finished(success);
    break;

  case DT_compressed_archive:
    _package->compressed_archive_download_finished(success);
    break;
  }
}
