/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file extractor.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "extractor.h"
#include "config_downloader.h"
#include "trueClock.h"
#include "filename.h"
#include "error_utils.h"


/**
 *
 */
Extractor::
Extractor() {
  _initiated = false;
  _multifile = new Multifile;
}

/**
 *
 */
Extractor::
~Extractor() {
  reset();
}

/**
 * Specifies the filename of the Multifile that the Extractor will read.
 * Returns true on success, false if the mulifile name is invalid.
 */
bool Extractor::
set_multifile(const Filename &multifile_name) {
  reset();
  _multifile_name = multifile_name;
  return _multifile->open_read(multifile_name);
}

/**
 * Specifies the directory into which all extracted subfiles will be written.
 * Relative paths of subfiles within the Multifile will be written as relative
 * paths to this directory.
 */
void Extractor::
set_extract_dir(const Filename &extract_dir) {
  _extract_dir = extract_dir;
}

/**
 * Interrupts the Extractor in the middle of its business and makes it ready
 * to accept a new list of subfiles to extract.
 */
void Extractor::
reset() {
  if (_initiated) {
    if (_read != nullptr) {
      Multifile::close_read_subfile(_read);
      _read = nullptr;
    }
    _write.close();
    _initiated = false;
  }

  _requests.clear();
  _requests_total_length = 0;
}

/**
 * Requests a particular subfile to be extracted when step() or run() is
 * called.  Returns true if the subfile exists, false otherwise.
 */
bool Extractor::
request_subfile(const Filename &subfile_name) {
  int index = _multifile->find_subfile(subfile_name);
  if (index < 0) {
    return false;
  }
  _requests.push_back(index);
  _requests_total_length += _multifile->get_subfile_length(index);
  return true;
}

/**
 * Requests all subfiles in the Multifile to be extracted.  Returns the number
 * requested.
 */
int Extractor::
request_all_subfiles() {
  _requests.clear();
  _requests_total_length = 0;
  int num_subfiles = _multifile->get_num_subfiles();
  for (int i = 0; i < num_subfiles; i++) {
    _requests.push_back(i);
    _requests_total_length += _multifile->get_subfile_length(i);
  }
  return num_subfiles;
}

/**
 * After all of the requests have been made via request_file() or
 * request_all_subfiles(), call step() repeatedly until it stops returning
 * EU_ok.
 *
 * step() extracts the next small unit of data from the Multifile.  Returns
 * EU_ok if progress is continuing, EU_error_abort if there is a problem, or
 * EU_success when the last piece has been extracted.
 *
 * Also see run().
 */
int Extractor::
step() {
  if (!_initiated) {
    _request_index = 0;
    _subfile_index = 0;
    _subfile_pos = 0;
    _subfile_length = 0;
    _total_bytes_extracted = 0;
    _read = nullptr;
    _initiated = true;
  }

  TrueClock *clock = TrueClock::get_global_ptr();
  double now = clock->get_short_time();
  double finish = now + extractor_step_time;

  do {
    if (_read == nullptr) {
      // Time to open the next subfile.
      if (_request_index >= (int)_requests.size()) {
        // All done!
        if (downloader_cat.is_debug()) {
          downloader_cat.debug()
            << "Finished extracting.\n";
        }
        reset();
        return EU_success;
      }

      _subfile_index = _requests[_request_index];
      _subfile_filename = Filename(_extract_dir,
                                   _multifile->get_subfile_name(_subfile_index));

      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Extracting " << _subfile_filename << ".\n";
      }

      _subfile_filename.set_binary();
      _subfile_filename.make_dir();
      if (!_subfile_filename.open_write(_write, true)) {
        downloader_cat.error()
          << "Unable to write to " << _subfile_filename << ".\n";
        reset();
        return EU_error_abort;
      }

      _subfile_length = _multifile->get_subfile_length(_subfile_index);
      _subfile_pos = 0;
      _read = _multifile->open_read_subfile(_subfile_index);
      if (_read == nullptr) {
        downloader_cat.error()
          << "Unable to read subfile "
          << _multifile->get_subfile_name(_subfile_index) << ".\n";
        reset();
        return EU_error_abort;
      }

    } else if (_subfile_pos >= _subfile_length) {
      // Time to close this subfile.

      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Finished current subfile.\n";
      }
      Multifile::close_read_subfile(_read);
      _read = nullptr;
      _write.close();
      _request_index++;

    } else {
      // Read a number of bytes from the subfile and write them to the output.
      static const size_t buffer_size = 1024;
      char buffer[buffer_size];

      size_t max_bytes = std::min(buffer_size, _subfile_length - _subfile_pos);
      _read->read(buffer, max_bytes);
      size_t count = _read->gcount();
      while (count != 0) {
        if (downloader_cat.is_spam()) {
          downloader_cat.spam()
            << " . . . read " << count << " bytes.\n";
        }
        _write.write(buffer, count);
        if (!_write) {
          downloader_cat.error()
            << "Error writing to " << _subfile_filename << ".\n";
          reset();
          return EU_error_abort;
        }

        _subfile_pos += count;
        _total_bytes_extracted += count;

        now = clock->get_short_time();
        if (now >= finish) {
          // That's enough for now.
          return EU_ok;
        }

        max_bytes = std::min(buffer_size, _subfile_length - _subfile_pos);
        _read->read(buffer, max_bytes);
        count = _read->gcount();
      }

      if (max_bytes != 0) {
        downloader_cat.error()
          << "Unexpected EOF on multifile " << _multifile_name << ".\n";
        reset();
        return EU_error_abort;
      }
    }

    now = clock->get_short_time();
  } while (now < finish);

  // That's enough for now.
  return EU_ok;
}

/**
 * Returns the fraction of the Multifile extracted so far.
 */
PN_stdfloat Extractor::
get_progress() const {
  if (!_initiated) {
    return 0.0f;
  }
  if (_requests_total_length == 0) {
    return 1.0f;
  }

  return (PN_stdfloat)_total_bytes_extracted / (PN_stdfloat)_requests_total_length;
}

/**
 * A convenience function to extract the Multifile all at once, when you don't
 * care about doing it in the background.
 *
 * First, call request_file() or request_all_files() to specify the files you
 * would like to extract, then call run() to do the extraction.  Also see
 * step() for when you would like the extraction to happen as a background
 * task.
 */
bool Extractor::
run() {
  while (true) {
    int ret = step();
    if (ret == EU_success) {
      return true;
    }
    if (ret < 0) {
      return false;
    }
  }
}
