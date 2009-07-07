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
#include "mkdir_complete.h"

#include "zlib.h"

#include <algorithm>
#include <fstream>

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
P3DPackage(const string &package_name, const string &package_version,
           const string &package_platform,
           const string &package_display_name) :
  _package_name(package_name),
  _package_version(package_version),
  _package_platform(package_platform),
  _package_display_name(package_display_name)
{
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();

  _package_fullname = _package_name + "_" + _package_version;
  _package_dir = inst_mgr->get_root_dir() + string("/") + _package_name;

  if (!_package_platform.empty()) {
    _package_fullname += "_" + _package_platform;
    _package_dir += "/" + _package_platform;
  }

  _ready = false;
  _failed = false;
  _active_download = NULL;
  _partial_download = false;

  // Ensure the package directory exists; create it if it does not.
  _package_dir += string("/") + _package_version;
  mkdir_complete(_package_dir);

  _desc_file_basename = _package_fullname + ".xml";
  _desc_file_pathname = _package_dir + "/" + _desc_file_basename;
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

  // Cancel any pending download.
  if (_active_download != NULL) {
    _active_download->cancel();
    delete _active_download;
    _active_download = NULL;
  }

  assert(_instances.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::set_callback
//       Access: Public
//  Description: Registers a callback on the package.  The callback
//               object will be notified when the package is ready for
//               use (or when it has failed to download properly).
////////////////////////////////////////////////////////////////////
void P3DPackage::
set_callback(Callback *callback) {
  if (_ready || _failed) {
    // Actually, we're already done.  Signal the callback immediately.
    callback->package_ready(this, _ready);
    delete callback;
  } else {
    // Bootstrap still in progress.  Save the callback.
    _callbacks.push_back(callback);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::cancel_callback
//       Access: Public
//  Description: Unregisters a particular callback object.  This
//               object will no longer be notified when the package is
//               ready.
////////////////////////////////////////////////////////////////////
void P3DPackage::
cancel_callback(Callback *callback) {
  Callbacks::iterator ci;
  ci = find(_callbacks.begin(), _callbacks.end(), callback);
  if (ci != _callbacks.end()) {
    _callbacks.erase(ci);
  } else {
    nout << "Canceling unknown callback on " << _package_fullname << "\n";
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::set_instance
//       Access: Public
//  Description: Specifies an instance that may be responsible for
//               downloading this package.
////////////////////////////////////////////////////////////////////
void P3DPackage::
set_instance(P3DInstance *inst) {
  _instances.push_back(inst);

  begin_download();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::cancel_instance
//       Access: Public
//  Description: Indicates that the given instance will no longer be
//               responsible for downloading this package.
////////////////////////////////////////////////////////////////////
void P3DPackage::
cancel_instance(P3DInstance *inst) {
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

  begin_download();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::begin_download
//       Access: Public
//  Description: Begins downloading and installing the package, if
//               needed.
////////////////////////////////////////////////////////////////////
void P3DPackage::
begin_download() {  
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

  // TODO: we should check the desc file for updates with the server.
  // Perhaps this should be done in a parent class.

  // TODO: if the desc file exists, and is consistent with the server
  // contents file, don't re-download it.
  /*
  // Load the desc file, if it exists.
  TiXmlDocument doc(_desc_file_pathname.c_str());
  if (!doc.LoadFile()) {
    download_desc_file();
  } else {
    got_desc_file(&doc, false);
  }
  */

  download_desc_file();
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::download_desc_file
//       Access: Private
//  Description: Starts downloading the desc file for the package.
////////////////////////////////////////////////////////////////////
void P3DPackage::
download_desc_file() {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  string url = inst_mgr->get_download_url();
  url += _package_name + "/" + _package_version;
  if (!_package_platform.empty()) {
    url += "/" + _package_platform;
  }

  url += "/" + _desc_file_basename;

  start_download(DT_desc_file, url, _desc_file_pathname, false);
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

  TiXmlDocument doc(_desc_file_pathname.c_str());
  if (!doc.LoadFile()) {
    nout << "Couldn't read " << _desc_file_pathname << "\n";
    report_done(false);
    return;
  }

  got_desc_file(&doc, true);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::got_desc_file
//       Access: Private
//  Description: Reads the desc file and begins verifying the files.
////////////////////////////////////////////////////////////////////
void P3DPackage::
got_desc_file(TiXmlDocument *doc, bool freshly_downloaded) {
  TiXmlElement *xpackage = doc->FirstChildElement("package");
  TiXmlElement *uncompressed_archive = NULL;
  TiXmlElement *compressed_archive = NULL;
  
  if (xpackage != NULL) {
    uncompressed_archive = xpackage->FirstChildElement("uncompressed_archive");
    compressed_archive = xpackage->FirstChildElement("compressed_archive");
  }

  if (uncompressed_archive == NULL || compressed_archive == NULL) {
    // The desc file didn't include the archive file itself, weird.
    if (!freshly_downloaded) {
      download_desc_file();
      return;
    }
    report_done(false);
    return;
  }

  _uncompressed_archive.load_xml(uncompressed_archive);
  _compressed_archive.load_xml(compressed_archive);

  // Now get all the components.
  _components.clear();
  TiXmlElement *component = xpackage->FirstChildElement("component");
  while (component != NULL) {
    FileSpec file;
    file.load_xml(component);
    _components.push_back(file);
    component = component->NextSiblingElement("component");
  }

  // Verify all of the components.
  bool all_components_ok = true;
  Components::iterator ci;
  for (ci = _components.begin(); ci != _components.end(); ++ci) {
    if (!(*ci).quick_verify(_package_dir)) {
      all_components_ok = false;
      break;
    }
  }

  if (all_components_ok) {
    // Great, we're ready to begin.
    report_done(true);

  } else if (_uncompressed_archive.quick_verify(_package_dir)) {
    // We need to re-extract the archive.
    extract_archive();

  } else if (_compressed_archive.quick_verify(_package_dir)) {
    // We need to uncompress the archive.
    uncompress_archive();

  } else {
    // Shoot, we need to download the archive.
    download_compressed_archive(true);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::download_compressed_archive
//       Access: Private
//  Description: Starts downloading the archive file for the package.
////////////////////////////////////////////////////////////////////
void P3DPackage::
download_compressed_archive(bool allow_partial) {
  P3DInstanceManager *inst_mgr = P3DInstanceManager::get_global_ptr();
  string url = inst_mgr->get_download_url();
  url += _package_name + "/" + _package_version;
  if (!_package_platform.empty()) {
    url += "/" + _package_platform;
  }

  url += "/" + _compressed_archive.get_filename();

  string target_pathname = _package_dir + "/" + _compressed_archive.get_filename();

  start_download(DT_compressed_archive, url, target_pathname, allow_partial);
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

  if (_compressed_archive.full_verify(_package_dir)) {
    // Go on to uncompress the archive.
    uncompress_archive();
    return;
  }

  // Oof, didn't download it correctly.
  if (_partial_download) {
    // Go back and get the whole file this time.
    download_compressed_archive(false);
  }

  nout << _compressed_archive.get_filename()
       << " failed hash check after download\n";
  report_done(false);
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

  if (!mkfile_complete(target_pathname)) {
    nout << "Unable to create " << target_pathname << "\n";
    report_done(false);
    return;
  }

  ofstream target(target_pathname.c_str(), ios::out | ios::binary);
  if (!target) {
    nout << "Couldn't write to " << target_pathname << "\n";
    report_done(false);
    return;
  }
  
  static const int decompress_buffer_size = 1024;
  char decompress_buffer[decompress_buffer_size];
  static const int write_buffer_size = 1024;
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
  if (!reader.extract(source_pathname, _package_dir,
                      this, download_portion + uncompress_portion, extract_portion)) {
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
//               listening callbacks.
////////////////////////////////////////////////////////////////////
void P3DPackage::
report_progress(double progress) {
  Callbacks::iterator ci;
  for (ci = _callbacks.begin(); ci != _callbacks.end(); ++ci) {
    (*ci)->install_progress(this, progress);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::report_done
//       Access: Private
//  Description: Transitions the package to "ready" or "failure"
//               state, and reports this change to all the listening
//               callbacks.
////////////////////////////////////////////////////////////////////
void P3DPackage::
report_done(bool success) {
  if (success) {
    _ready = true;
    _failed = false;
  } else {
    _ready = false;
    _failed = true;
  }

  Callbacks orig_callbacks;
  orig_callbacks.swap(_callbacks);
  Callbacks::iterator ci;
  for (ci = orig_callbacks.begin(); ci != orig_callbacks.end(); ++ci) {
    (*ci)->package_ready(this, _ready);
    delete (*ci);
  }

  // We shouldn't have added any more callbacks during the above loop.
  assert(_callbacks.empty());
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::start_download
//       Access: Private
//  Description: Initiates a download of the indicated file.
////////////////////////////////////////////////////////////////////
void P3DPackage::
start_download(P3DPackage::DownloadType dtype, const string &url, 
               const string &pathname, bool allow_partial) {
  // Only one download should be active at a time
  assert(_active_download == NULL);
  
  if (!allow_partial) {
    unlink(pathname.c_str());
  }

  Download *download = new Download(this, dtype);
  download->set_url(url);
  download->set_filename(pathname);

  // TODO: implement partial file re-download.
  allow_partial = false;

  _active_download = download;
  _partial_download = false;

  assert(!_instances.empty());

  _instances[0]->start_download(download);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Callback::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::Callback::
~Callback() {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Callback::install_progress
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DPackage::Callback::
install_progress(P3DPackage *package, double progress) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Callback::package_ready
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
void P3DPackage::Callback::
package_ready(P3DPackage *package, bool success) {
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::Download::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
P3DPackage::Download::
Download(P3DPackage *package, DownloadType dtype) :
  _package(package),
  _dtype(dtype)
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

  switch (_dtype) {
  case DT_desc_file:
    _package->desc_file_download_finished(success);
    break;

  case DT_compressed_archive:
    _package->compressed_archive_download_finished(success);
    break;
  }
}
