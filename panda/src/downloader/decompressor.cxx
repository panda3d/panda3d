/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file decompressor.cxx
 * @author mike
 * @date 1997-01-09
 */

#include "pandabase.h"

#ifdef HAVE_ZLIB

#include "config_downloader.h"

#include "error_utils.h"
#include "filename.h"
#include "ramfile.h"
#include "zStream.h"
#include "config_express.h"
#include "trueClock.h"

#include "decompressor.h"

#include <stdio.h>
#include <errno.h>

/**
 *
 */
Decompressor::
Decompressor() {
  _source = nullptr;
  _decompress = nullptr;
  _dest = nullptr;
}

/**
 *
 */
Decompressor::
~Decompressor() {
  cleanup();
}

/**
 * Begins a background decompression of the named file (whose filename must
 * end in ".pz") to a new file without the .pz extension.  The source file is
 * removed after successful completion.
 */
int Decompressor::
initiate(const Filename &source_file) {
  std::string extension = source_file.get_extension();
  if (extension == "pz" || extension == "gz") {
    Filename dest_file = source_file;
    dest_file = source_file.get_fullpath_wo_extension();
    return initiate(source_file, dest_file);
  }

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "Unknown file extension for decompressor: ."
      << extension << std::endl;
  }
  return EU_error_abort;
}

/**
 * Begins a background decompression from the named source file to the named
 * destination file.  The source file is removed after successful completion.
 */
int Decompressor::
initiate(const Filename &source_file, const Filename &dest_file) {
  cleanup();

  // Open source file
  _source_filename = Filename(source_file);
  _source_filename.set_binary();

  pifstream *source_pfstream = new pifstream;
  _source = source_pfstream;
  if (!_source_filename.open_read(*source_pfstream)) {
    downloader_cat.error()
      << "Unable to read " << _source_filename << "\n";
    return get_write_error();
  }

  // Determine source file length
  source_pfstream->seekg(0, std::ios::end);
  _source_length = source_pfstream->tellg();
  if (_source_length == 0) {
    downloader_cat.warning()
      << "Zero length file: " << source_file << "\n";
    return EU_error_file_empty;
  }
  source_pfstream->seekg(0, std::ios::beg);

  // Open destination file
  Filename dest_filename(dest_file);
  dest_filename.set_binary();

  pofstream *dest_pfstream = new pofstream;
  _dest = dest_pfstream;
  if (dest_filename.exists()) {
    downloader_cat.info()
      << dest_filename << " already exists, removing.\n";
    if (!dest_filename.unlink()) {
      downloader_cat.error()
        << "Unable to remove old " << dest_filename << "\n";
      return get_write_error();
    }
  } else {
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << dest_filename << " does not already exist.\n";
    }
  }
  if (!dest_filename.open_write(*dest_pfstream, true)) {
    downloader_cat.error()
      << "Unable to write to " << dest_filename << "\n";
    return get_write_error();
  }

  // Now create the decompressor stream.
  _decompress = new IDecompressStream(_source, false);
  return EU_success;
}

/**
 * Called each frame to do the next bit of work in the background task.
 * Returns EU_ok if a chunk is completed but there is more to go, or
 * EU_success when we're all done.  Any other return value indicates an error.
 */
int Decompressor::
run() {
  if (_decompress == nullptr) {
    // Hmm, we were already done.
    return EU_success;
  }

  TrueClock *clock = TrueClock::get_global_ptr();
  double now = clock->get_short_time();
  double finish = now + decompressor_step_time;

  static const size_t buffer_size = 1024;
  char buffer[buffer_size];

  _decompress->read(buffer, buffer_size);
  size_t count = _decompress->gcount();
  while (count != 0) {
    _dest->write(buffer, count);

    now = clock->get_short_time();
    if (now >= finish) {
      // That's enough for now.
      return EU_ok;
    }

    _decompress->read(buffer, buffer_size);
    count = _decompress->gcount();
  }

  // All done!
  cleanup();
  if (!keep_temporary_files) {
    _source_filename.unlink();
  }
  return EU_success;
}

/**
 * Performs a foreground decompression of the named file; does not return
 * until the decompression is complete.
 */
bool Decompressor::
decompress(const Filename &source_file) {
  int ret = initiate(source_file);
  if (ret < 0)
    return false;

  int ch = _decompress->get();
  while (ch != EOF && !_decompress->fail()) {
    _dest->put(ch);
    ch = _decompress->get();
  }

  cleanup();
  if (!keep_temporary_files) {
    _source_filename.unlink();
  }
  return true;
}

/**
 * Does an in-memory decompression of the indicated Ramfile.  The decompressed
 * contents are written back into the same Ramfile on completion.
 */
bool Decompressor::
decompress(Ramfile &source_and_dest_file) {
  std::istringstream source(source_and_dest_file._data);
  std::ostringstream dest;

  IDecompressStream decompress(&source, false);

  int ch = decompress.get();
  while (ch != EOF && !decompress.fail()) {
    dest.put(ch);
    ch = decompress.get();
  }

  source_and_dest_file._pos = 0;
  source_and_dest_file._data = dest.str();
  return true;
}

/**
 * Returns the ratio through the decompression step in the background.
 */
PN_stdfloat Decompressor::
get_progress() const {
  if (_decompress == nullptr) {
    // Hmm, we were already done.
    return 1.0f;
  }

  nassertr(_source_length > 0, 0.0);
  size_t source_pos = _source->tellg();

  // We stop the scale at 0.99 because there may be a little bit more to do
  // even after the decompressor has read all of the source.
  return (0.99f * (PN_stdfloat)source_pos / (PN_stdfloat)_source_length);
}

/**
 * Called to reset a previous decompressor state and clean up properly.
 */
void Decompressor::
cleanup() {
  if (_source != nullptr) {
    delete _source;
    _source = nullptr;
  }
  if (_dest != nullptr) {
    delete _dest;
    _dest = nullptr;
  }
  if (_decompress != nullptr) {
    delete _decompress;
    _decompress = nullptr;
  }
}

#endif  // HAVE_ZLIB
