// Filename: httpBackup.cxx
// Created by:  drose (29Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "httpBackup.h"
#include "httpChannel.h"


static const int seconds_per_day = 60 * 60 * 24;

////////////////////////////////////////////////////////////////////
//     Function: HTTPBackup::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
HTTPBackup::
HTTPBackup() {
  clear_runlines();
  add_runline("[opts] url");
  add_runline("-check <days> url");

  set_program_description
    ("This program is designed to run periodically as a "
     "background task, e.g. via a cron job.  It fetches the "
     "latest copy of a document from an HTTP server and "
     "stores it, along with an optional number of previous "
     "versions, in a local directory so that it may be "
     "backed up to tape.\n\n"
     
     "If the copy on disk is already the same as the latest "
     "copy available on the HTTP server, this program generally "
     "does nothing (although it may delete old versions if they "
     "have expired past the maximum age specified on the command "
     "line).");

  add_option
    ("p", "url", 0,
     "Specifies the URL of the HTTP proxy server, if one is required.",
     &HTTPBackup::dispatch_url, &_got_proxy, &_proxy);

  add_option
    ("a", "", 0,
     "If this option is specified, the document is always downloaded every "
     "time httpbackup runs, even if the document does not appear to have "
     "been changed since last time.",
     &HTTPBackup::dispatch_none, &_always_download, NULL);

  add_option
    ("d", "dirname", 0,
     "Specifies the name of the directory in which to store the backup "
     "versions of the document.  The default is '.', the current "
     "directory.",
     &HTTPBackup::dispatch_filename, NULL, &_dirname);

  add_option
    ("c", "filename", 0,
     "Specifies the name of the catalog file that httpbackup uses to "
     "record the HTTP entity tags, etc., downloaded from previous "
     "versions.  If a relative filename is given, it is relative to "
     "the directory specified by -d.  The default is 'Catalog'.",
     &HTTPBackup::dispatch_filename, NULL, &_catalog_name);

  add_option
    ("n", "filename", 0,
     "Specifies the name of the document that is being retrieved.  This "
     "name is written to the catalog file to identify entries for this "
     "document, and is used to generate the filename to store the "
     "backup versions.  The default if this is omitted or empty is to use "
     "the basename of the URL.",
     &HTTPBackup::dispatch_string, NULL, &_document_name);

  add_option
    ("s", "string", 0,
     "Specifies how the date is appended onto the filename (see -n) for "
     "each version of the file.  This string should contain the sequence "
     "of characters from strftime() that correspond to the desired date "
     "format to append to the filename.  The default is '.%Y-%m-%d.%H-%M', "
     "or the year, month, day, hour, and minute.  (The date is always "
     "represented in GMT, according to HTTP convention.)",
     &HTTPBackup::dispatch_string, NULL, &_version_append);

  add_option
    ("maxage", "days", 0,
     "Specifies the maximum age, in days, to keep an old version of the "
     "file around.  If unspecified, the default is no limit.  This may "
     "be a floating-point number.",
     &HTTPBackup::dispatch_double, &_got_max_keep_days, &_max_keep_days);

  add_option
    ("minage", "days", 0,
     "Specifies the minimum age, in days, an old version of the file must "
     "have before it is automatically deleted due to exceeding -maxver.  "
     "The default is 0.  This may be a floating-point number.",
     &HTTPBackup::dispatch_double, NULL, &_min_keep_days);

  add_option
    ("maxver", "count", 0,
     "Specifies the maximum number of old versions of the file to keep "
     "around.  If unspecified, the default is no limit.",
     &HTTPBackup::dispatch_int, &_got_max_keep_versions, &_max_keep_versions);

  add_option
    ("minver", "count", 0,
     "Specifies the minimum number of old versions to keep after "
     "deleting versions older than -maxage.  The default is 1.",
     &HTTPBackup::dispatch_int, NULL, &_min_keep_versions);

  add_option
    ("check", "days", 0,
     "Instead of downloading any document, check the date of the most recent "
     "document downloaded.  Returns success if that date is no more than the "
     "indicated number of days old (which may be a floating-point number), "
     "or failure otherwise.",
     &HTTPBackup::dispatch_double, &_got_check_days, &_check_days);

  _dirname = ".";
  _catalog_name = "Catalog";
  _version_append = ".%Y-%m-%d.%H-%M";
  _max_keep_days = 0.0;
  _min_keep_days = 0.0;
  _max_keep_versions = 0;
  _min_keep_versions = 1;
}


////////////////////////////////////////////////////////////////////
//     Function: HTTPBackup::handle_args
//       Access: Protected, Virtual
//  Description: Does something with the additional arguments on the
//               command line (after all the -options have been
//               parsed).  Returns true if the arguments are good,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool HTTPBackup::
handle_args(ProgramBase::Args &args) {
  if (_got_check_days && !_document_name.empty() && args.empty()) {
    // If -check and -n are both specified, we don't really need to
    // specify an URL.  Accept it if we don't.
    return true;
  }

  if (args.size() != 1) {
    nout << 
      "You must specify the URL of the document to download "
      "on the command line.\n\n";
    return false;
  }

  _url = URLSpec(args[0]);
  if (!(_url.has_server() && _url.has_path())) {
    nout 
      << "Invalid URL specification: " << args[0] << "\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPBackup::post_command_line
//       Access: Protected, Virtual
//  Description: This is called after the command line has been
//               completely processed, and it gives the program a
//               chance to do some last-minute processing and
//               validation of the options and arguments.  It should
//               return true if everything is fine, false if there is
//               an error.
////////////////////////////////////////////////////////////////////
bool HTTPBackup::
post_command_line() {
  if (_got_proxy) {
    _http.set_proxy(_proxy);
  }

  if (!_catalog_name.is_fully_qualified()) {
    _catalog_name = Filename(_dirname, _catalog_name);
  }

  if (_document_name.empty()) {
    Filename pathname = _url.get_path();
    _document_name = pathname.get_basename();
  }

  if (_min_keep_days < 0.0) {
    nout << "Invalid -minage " << _min_keep_days << "\n";
    return false;
  }

  if (_min_keep_versions < 0) {
    nout << "Invalid -minver " << _min_keep_versions << "\n";
    return false;
  }

  if (_got_max_keep_days) {
    if (_max_keep_days < _min_keep_days) {
      nout
        << "-maxage " << _max_keep_days << " is less than -minage "
        << _min_keep_days << "\n";
      return false;
    }
  }

  if (_got_max_keep_versions) {
    if (_max_keep_versions < _min_keep_versions) {
      nout
        << "-maxver " << _max_keep_versions << " is less than -minver "
        << _min_keep_versions << "\n";
      return false;
    }
  }

  _now = HTTPDate::now();
  if (_got_max_keep_days) {
    _max_keep_date = _now - (time_t)(_max_keep_days * seconds_per_day);
  }
  _min_keep_date = _now - (time_t)(_min_keep_days * seconds_per_day);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPBackup::dispatch_url
//       Access: Protected, Static
//  Description: Dispatch function for a URL parameter.
////////////////////////////////////////////////////////////////////
bool HTTPBackup::
dispatch_url(const string &opt, const string &arg, void *var) {
  URLSpec *up = (URLSpec *)var;
  (*up) = URLSpec(arg);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPBackup::run
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void HTTPBackup::
run() {
  // Output the current date and time in GMT, for logging.
  nout << _now.get_string() << "\n";

  // First, read in the catalog.
  _catalog_name.set_text();
  if (!_catalog_name.exists()) {
    nout << _catalog_name << " does not yet exist.\n";
  } else {
    if (!_catalog.read(_catalog_name)) {
      nout << "Unable to read " << _catalog_name << ".\n";
      exit(1);
    }
  }

  if (_got_check_days) {
    // We're only checking the date of the latest download.
    BackupCatalog::Entries &entries = _catalog._table[_document_name];
    if (entries.empty()) {
      nout << "No previous downloads for " << _document_name << ".\n";
      exit(1);
    }
    BackupCatalog::Entry *latest = entries[entries.size() - 1];
    int diff_secs = _now - latest->get_date();
    double diff_days = (double)diff_secs / (double)seconds_per_day;
    nout << "Most recent at " << latest->get_date().get_string()
         << ", " << diff_days << " days old.\n";
    if (diff_days <= _check_days) {
      nout << "OK.\n";
    } else {
      nout << "Too old!\n";
      exit(1);
    }
    
  } else {
    // Try to do the download.

    // Now try to fetch the document.
    if (!fetch_latest()) {
      nout << "Errors while processing latest.\n";
      exit(1);
    }
    
    if (!cleanup_old()) {
      nout << "Errors while cleaning up old versions.\n";
      // We don't bother to exit the program in this case.
    }
    
    if (_catalog._dirty) {
      // Now write out the modified catalog.
      nout << "Writing " << _catalog_name << "\n";
      _catalog_name.make_dir();
      if (!_catalog.write(_catalog_name)) {
        nout << "Unable to rewrite " << _catalog_name << ".\n";
        exit(1);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPBackup::fetch_latest
//       Access: Private
//  Description: Tries to get the latest version of the document from
//               the server, if there is one available.  Returns true
//               on success (even if the most recent document hasn't
//               changed), or false if there was some error.
////////////////////////////////////////////////////////////////////
bool HTTPBackup::
fetch_latest() {
  // Check the most recent version of this document.
  BackupCatalog::Entries &entries = _catalog._table[_document_name];

  DocumentSpec document_spec(_url);
  if (!entries.empty()) {
    BackupCatalog::Entry *latest = entries[entries.size() - 1];
    document_spec = latest->_document_spec;
    document_spec.set_url(_url);
    if (!document_spec.has_date()) {
      // If we didn't get a last-modified date, use the download date
      // instead.
      document_spec.set_date(latest->get_date());
    }
    if (!_always_download) {
      document_spec.set_request_mode(DocumentSpec::RM_newer);
    }
  }

  // Since the purpose of this program is to check to see if a more
  // recent document is available, we probably always want any proxies
  // in the way to revalidate their cache.
  document_spec.set_cache_control(DocumentSpec::CC_revalidate);
 
  if (document_spec.get_request_mode() == DocumentSpec::RM_newer &&
      document_spec.has_date()) {
    nout << "Checking for newer than "<< document_spec.get_date().get_string()
         << ".\n";
  }
  nout << "Fetching " << document_spec.get_url() << "\n";

  PT(HTTPChannel) channel = _http.make_channel(true);
  if (_always_download) {
    channel->get_document(document_spec);
  } else {
    // Start out by asking for the header first, so we can verify the
    // document has changed before we try to download it again.
    channel->get_header(document_spec);
  }

  if (!channel->is_valid()) {
    if (channel->get_status_code() == 304) {
      nout << "Document has not been modified.\n";
      // This is considered a success condition.
      return true;
    }
    nout << "Error fetching document: " << channel->get_status_code()
         << " " << channel->get_status_string() << "\n";
    return false;
  }

  // The document is available.
  if (!_always_download) {
    if (!entries.empty()) {
      // Has it been modified?  We need to check again because the
      // If-None-Match fields, etc. might have been ignored (e.g. by a
      // proxy).
      if (document_spec == channel->get_document_spec()) {
        nout
          << "Document has not been modified (server ignored conditional request).\n";
        return true;
      }
    }

    // Ok, the document really is ready.  Go ahead and download it.
    if (!channel->get_document(document_spec)) {
      nout << "Unable to request document.\n";
      return false;
    }
  }

  // We've started to download the document.  Create an entry for it.
  BackupCatalog::Entry *entry = new BackupCatalog::Entry;
  entry->_document_name = _document_name;
  entry->_document_spec = channel->get_document_spec();
  entry->_download_date = _now;

  // Generate a filename based on the last-modified date or the
  // download date.
  time_t time = entry->get_date().get_time();
  struct tm *tp = gmtime(&time);

  static const int buffer_size = 512;
  char buffer[buffer_size];
  if (strftime(buffer, buffer_size, _version_append.c_str(), tp) == 0) {
    buffer[0] = '\0';
  }

  string filename = _document_name + string(buffer);
  
  // Check the filename for uniqueness, just for good measure.
  check_unique(filename);

  entry->_filename = filename;

  // Download to the indicated filename.
  Filename pathname(_dirname, filename);
  nout << "Downloading to " << pathname << "\n";
  pathname.make_dir();
  if (!channel->download_to_file(pathname)) {
    nout << "Error while downloading.\n";
    delete entry;
    return false;
  }

  // The file is successfully downloaded; save the entry.
  entries.push_back(entry);
  _catalog._dirty = true;
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPBackup::cleanup_old
//       Access: Private
//  Description: Removes old versions that are no longer needed.
////////////////////////////////////////////////////////////////////
bool HTTPBackup::
cleanup_old() {
  BackupCatalog::Entries &entries = _catalog._table[_document_name];

  if (_got_max_keep_versions && 
      (int)entries.size() > _max_keep_versions) {
    // Too many versions; delete the oldest ones, except those newer
    // than min_keep_date.
    int num_delete = entries.size() - _max_keep_versions;
    while (num_delete > 0 && entries[num_delete - 1]->get_date() > _min_keep_date) {
      num_delete--;
    }

    for (int i = 0; i < num_delete; i++) {
      entries[i]->delete_file(_dirname, "too many old versions");
      delete entries[i];
    }
    entries.erase(entries.begin(), entries.begin() + num_delete);
    _catalog._dirty = true;
  }

  if (_got_max_keep_days && 
      (int)entries.size() > _min_keep_versions && 
      entries[0]->get_date() < _max_keep_date) {
    // The oldest version is too old; delete all the oldest ones,
    // but keep at least min_keep_versions of them around.
    int num_delete = 1;
    while (num_delete < (int)entries.size() - _min_keep_versions &&
           entries[num_delete]->get_date() < _max_keep_date) {
      num_delete++;
    }

    for (int i = 0; i < num_delete; i++) {
      entries[i]->delete_file(_dirname, "too old");
      delete entries[i];
    }
    entries.erase(entries.begin(), entries.begin() + num_delete);
    _catalog._dirty = true;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: HTTPBackup::check_unique
//       Access: Private
//  Description: Ensures that the given filename is unique among all
//               files in the catalog.
////////////////////////////////////////////////////////////////////
void HTTPBackup::
check_unique(string &filename) {
  bool inserted = _catalog._filenames.insert(filename).second;

  if (!inserted) {
    // Conflict; append one or more letters to the filename until it
    // is unique.
    unsigned int uniquifier = 1;
    string orig_filename = filename;
    orig_filename += '.';

    while (!inserted) {
      filename = orig_filename;
      
      unsigned int count = uniquifier;
      while (count > 0) {
        char ch = (char)((count % 26) + 'a');
        filename += ch;
        count /= 26;
      }
      
      uniquifier++;
      inserted = _catalog._filenames.insert(filename).second;
    }
  }
}

int
main(int argc, char *argv[]) {
  HTTPBackup prog;
  prog.parse_command_line(argc, argv);
  prog.run();
  return 0;
}
