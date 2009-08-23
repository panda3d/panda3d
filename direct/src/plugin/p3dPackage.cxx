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
           const string &package_version) :
  _host(host),
  _package_name(package_name),
  _package_version(package_version)
{
  _package_fullname = _package_name;
  _package_dir = _host->get_host_dir() + string("/packages/") + _package_name;
  _package_fullname += string("_") + _package_version;
  _package_dir += string("/") + _package_version;

  _temp_contents_file = NULL;

  _info_ready = false;
  _download_size = 0;
  _allow_data_download = false;
  _ready = false;
  _failed = false;
  _active_download = NULL;
  _partial_download = false;

  // Ensure the package directory exists; create it if it does not.
  mkdir_complete(_package_dir, nout);

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

  if (_temp_contents_file != NULL) {
    delete _temp_contents_file;
    _temp_contents_file = NULL;
  }

  assert(_instances.empty());
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
  xpackage->SetAttribute("install_dir", _package_dir);

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
//               the instance manager doesn't have the file already.
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
    download_desc_file();
    return;
  }

  string url = _host->get_host_url_prefix();
  url += "contents.xml";

  // Download contents.xml to a temporary filename first, in case
  // multiple packages are downloading it simultaneously.
  assert(_temp_contents_file == NULL);
  _temp_contents_file = new P3DTemporaryFile(".xml");

  start_download(DT_contents_file, url, _temp_contents_file->get_filename(), false);
}

////////////////////////////////////////////////////////////////////
//     Function: P3DPackage::contents_file_download_finished
//       Access: Private
//  Description: Called when the desc file has been fully downloaded.
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
  FileSpec desc_file;
  if (!_host->get_package_desc_file(desc_file, _package_platform,
                                    _package_name, _package_version)) {
    nout << "Couldn't find package " << _package_fullname
         << " in contents file.\n";
    return;
  }

  // The desc file might have a different path on the host server than
  // it has locally, because we strip out the platform locally.
  // Adjust desc_file to point to the local file.
  string url_filename = desc_file.get_filename();
  desc_file.set_filename(_desc_file_basename);
  assert (desc_file.get_pathname(_package_dir) == _desc_file_pathname);

  if (!desc_file.full_verify(_package_dir)) {
    nout << _desc_file_pathname << " is stale.\n";

  } else {
    // The desc file is current.  Attempt to read it.
    TiXmlDocument doc(_desc_file_pathname.c_str());
    if (doc.LoadFile()) {
      got_desc_file(&doc, false);
      return;
    }
  }

  // The desc file is not current.  Go download it.
  string url = _host->get_host_url_prefix();
  url += url_filename;

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
    const char *display_name_cstr = xpackage->Attribute("display_name");
    if (display_name_cstr != NULL) {
      _package_display_name = display_name_cstr;
    }

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

  // Now get all the extractable components.
  _extracts.clear();
  TiXmlElement *extract = xpackage->FirstChildElement("extract");
  while (extract != NULL) {
    FileSpec file;
    file.load_xml(extract);
    _extracts.push_back(file);
    extract = extract->NextSiblingElement("extract");
  }

  // Verify the uncompressed archive.
  bool all_extracts_ok = true;
  if (!_uncompressed_archive.quick_verify(_package_dir)) {
    nout << "File is incorrect: " << _uncompressed_archive.get_filename() << "\n";
    all_extracts_ok = false;
  }

  // Verify all of the extracts.
  Extracts::iterator ci;
  for (ci = _extracts.begin(); ci != _extracts.end() && all_extracts_ok; ++ci) {
    if (!(*ci).quick_verify(_package_dir)) {
      nout << "File is incorrect: " << (*ci).get_filename() << "\n";
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
  string url = _host->get_host_url_prefix();
  url += _package_name;
  if (!_package_platform.empty()) {
    url += "/" + _package_platform;
  }
  url += "/" + _package_version;
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
  if (!reader.extract_all(source_pathname, _package_dir,
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
//     Function: P3DPackage::is_extractable
//       Access: Private
//  Description: Returns true if the name file is on the extract list,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool P3DPackage::
is_extractable(const string &filename) const {
  Extracts::const_iterator ci;
  for (ci = _extracts.begin(); ci != _extracts.end(); ++ci) {
    if ((*ci).get_filename() == filename) {
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
