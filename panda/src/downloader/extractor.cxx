// Filename: extractor.cxx
// Created by:  mike (09Jan97)
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

#include "extractor.h"
#include "config_downloader.h"

#include "filename.h"
#include "error_utils.h"


////////////////////////////////////////////////////////////////////
//     Function: Extractor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Extractor::
Extractor() {
  _initiated = false;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Extractor::
~Extractor() {
  if (downloader_cat.is_debug())
    downloader_cat.debug()
      << "Extractor destructor called" << endl;
  if (_initiated) {
    cleanup();
  }
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::initiate
//       Access: Public
//  Description: Begins the extraction process.  Returns EU_success if
//               successful, EU_error_abort otherwise.
//
//               After calling initiate(), you should repeatedly call
//               run() as a background task until the file is
//               completely extracted.
////////////////////////////////////////////////////////////////////
int Extractor::
initiate(const Filename &multifile_name, const Filename &extract_to) {
  if (_initiated) {
    downloader_cat.error()
      << "Extractor::initiate() - Extraction has already been initiated"
      << endl;
    return EU_error_abort;
  }

  _multifile_name = multifile_name;
  _extract_to = extract_to;

  if (!_multifile.open_read(_multifile_name)) {
    downloader_cat.error()
      << "Error opening Multifile " << _multifile_name << ".\n";
    return EU_error_abort;
  }

  _subfile_index = 0;
  _subfile_pos = 0;
  _subfile_length = 0;
  _read = (istream *)NULL;
  _initiated = true;

  return EU_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::run
//       Access: Public
//  Description: Extracts the next small unit of data from the
//               Multifile.  Returns EU_ok if progress is continuing,
//               EU_error_abort if there is a problem, or EU_success
//               when the last piece has been extracted.
////////////////////////////////////////////////////////////////////
int Extractor::
run() {
  if (!_initiated) {
    downloader_cat.error()
      << "Extractor::run() - Extraction has not been initiated"
      << endl;
    return EU_error_abort;
  }

  if (_read == (istream *)NULL) {
    // Time to open the next subfile.
    if (_subfile_index >= _multifile.get_num_subfiles()) {
      // All done!
      cleanup();
      return EU_success;
    }

    Filename subfile_filename(_extract_to, 
                              _multifile.get_subfile_name(_subfile_index));
    subfile_filename.set_binary();
    subfile_filename.make_dir();
    if (!subfile_filename.open_write(_write)) {
      downloader_cat.error()
        << "Unable to write to " << subfile_filename << ".\n";
      cleanup();
      return EU_error_abort;
    }

    _subfile_length = _multifile.get_subfile_length(_subfile_index);
    _subfile_pos = 0;
    _read = _multifile.open_read_subfile(_subfile_index);

  } else if (_subfile_pos >= _subfile_length) {
    // Time to close this subfile.
    delete _read;
    _read = (istream *)NULL;
    _write.close();
    _subfile_index++;

  } else {
    // Read a number of bytes from the subfile and write them to the
    // output.
    size_t max_bytes = min((size_t)extractor_buffer_size, 
                           _subfile_length - _subfile_pos);
    for (size_t p = 0; p < max_bytes; p++) {
      int byte = _read->get();
      if (_read->eof() || _read->fail()) {
        downloader_cat.error()
          << "Unexpected EOF on multifile " << _multifile_name << ".\n";
        cleanup();
        return EU_error_abort;
      }
      _write.put(byte);
    }
    _subfile_pos += max_bytes;
  }

  return EU_ok;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::extract
//       Access: Public
//  Description: A convenience function to extract the Multifile all
//               at once, when you don't care about doing it in the
//               background.
////////////////////////////////////////////////////////////////////
bool Extractor::
extract(const Filename &source_file, const Filename &rel_path) {
  int ret = initiate(source_file, rel_path);
  if (ret < 0) {
    return false;
  }
  for (;;) {
    ret = run();
    if (ret == EU_success) {
      return true;
    }
    if (ret < 0) {
      return false;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::cleanup
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Extractor::
cleanup() {
  if (downloader_cat.is_debug())
    downloader_cat.debug()
      << "Extractor cleanup called" << endl;
  if (!_initiated) {
    downloader_cat.error()
      << "Extractor::cleanup() - Extraction has not been initiated"
      << endl;
    return;
  }

  if (_read != (istream *)NULL) {
    delete _read;
    _read = (istream *)NULL;
  }
  _multifile.close();
  _write.close();
}
