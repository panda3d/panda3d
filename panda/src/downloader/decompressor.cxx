// Filename: decompressor.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "pandabase.h"

#ifdef HAVE_ZLIB

#include "config_downloader.h"

#include "error_utils.h"
#include "filename.h"
#include "ramfile.h"
#include "zStream.h"
#include "config_express.h"

#include "decompressor.h"

#include <stdio.h>
#include <errno.h>

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Decompressor::
Decompressor() {
  _source = NULL;
  _decompress = NULL;
  _dest = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Decompressor::
~Decompressor() {
  cleanup();
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::initiate
//       Access: Public
//  Description: Begins a background decompression of the named file
//               (whose filename must end in ".pz") to a new file
//               without the .pz extension.  The source file is
//               removed after successful completion.
////////////////////////////////////////////////////////////////////
int Decompressor::
initiate(const Filename &source_file) {
  string extension = source_file.get_extension();
  if (extension == "pz") {
    Filename dest_file = source_file;
    dest_file = source_file.get_fullpath_wo_extension();
    return initiate(source_file, dest_file);
  }

  if (downloader_cat.is_debug()) {
    downloader_cat.debug()
      << "Unknown file extension for decompressor: ."
      << extension << endl;
  }
  return EU_error_abort;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::initiate
//       Access: Public
//  Description: Begins a background decompression from the named
//               source file to the named destination file.  The
//               source file is removed after successful completion.
////////////////////////////////////////////////////////////////////
int Decompressor::
initiate(const Filename &source_file, const Filename &dest_file) {
  cleanup();

  // Open source file
  _source_filename = Filename(source_file);
  _source_filename.set_binary();

  ifstream *source_fstream = new ifstream;
  _source = source_fstream;
  if (!_source_filename.open_read(*source_fstream)) {
    downloader_cat.error()
      << "Unable to read " << _source_filename << "\n";
    return get_write_error();
  }

  // Determine source file length
  source_fstream->seekg(0, ios::end);
  _source_length = source_fstream->tellg();
  if (_source_length == 0) {
    downloader_cat.warning()
      << "Zero length file: " << source_file << "\n";
    return EU_error_file_empty;
  }
  source_fstream->seekg(0, ios::beg);

  // Open destination file
  Filename dest_filename(dest_file);
  dest_filename.set_binary();

  ofstream *dest_fstream = new ofstream;
  _dest = dest_fstream;
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
  if (!dest_filename.open_write(*dest_fstream, true)) {
    downloader_cat.error()
      << "Unable to write to " << dest_filename << "\n";
    return get_write_error();
  }

  // Now create the decompressor stream.
  _decompress = new IDecompressStream(_source, false);
  return EU_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::run
//       Access: Public
//  Description: Called each frame to do the next bit of work in the
//               background task.  Returns EU_ok if a chunk is
//               completed but there is more to go, or EU_success when
//               we're all done.  Any other return value indicates an
//               error.
////////////////////////////////////////////////////////////////////
int Decompressor::
run() {
  if (_decompress == (istream *)NULL) {
    // Hmm, we were already done.
    return EU_success;
  }
  
  // Read a bunch of characters from the decompress stream, but no
  // more than decompressor_buffer_size.
  int count = 0;
  int ch = _decompress->get();
  while (!_decompress->eof() && !_decompress->fail()) {
    _dest->put(ch);
    if (++count >= decompressor_buffer_size) {
      // That's enough for now.
      return EU_ok;
    }

    ch = _decompress->get();
  }

  // All done!
  cleanup();
  if (!keep_temporary_files) {
    _source_filename.unlink();
  }
  return EU_success;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::decompress
//       Access: Public
//  Description: Performs a foreground decompression of the named
//               file; does not return until the decompression is
//               complete.
////////////////////////////////////////////////////////////////////
bool Decompressor::
decompress(const Filename &source_file) {
  int ret = initiate(source_file);
  if (ret < 0)
    return false;

  int ch = _decompress->get();
  while (!_decompress->eof() && !_decompress->fail()) {
    _dest->put(ch);
    ch = _decompress->get();
  }

  cleanup();
  if (!keep_temporary_files) {
    _source_filename.unlink();
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::decompress
//       Access: Public
//  Description: Does an in-memory decompression of the indicated
//               Ramfile.  The decompressed contents are written back
//               into the same Ramfile on completion.
////////////////////////////////////////////////////////////////////
bool Decompressor::
decompress(Ramfile &source_and_dest_file) {
  istringstream source(source_and_dest_file._data);
  ostringstream dest;

  IDecompressStream decompress(&source, false);

  int ch = decompress.get();
  while (!decompress.eof() && !decompress.fail()) {
    dest.put(ch);
    ch = decompress.get();
  }

  source_and_dest_file._pos = 0;
  source_and_dest_file._data = dest.str();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::get_progress
//       Access: Public
//  Description: Returns the ratio through the decompression step
//               in the background.
////////////////////////////////////////////////////////////////////
float Decompressor::
get_progress() const {
  if (_decompress == (istream *)NULL) {
    // Hmm, we were already done.
    return 1.0f;
  }

  nassertr(_source_length > 0, 0.0);
  size_t source_pos = _source->tellg();

  // We stop the scale at 0.99 because there may be a little bit more
  // to do even after the decompressor has read all of the source.
  return (0.99f * (float)source_pos / (float)_source_length);
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::cleanup
//       Access: Private
//  Description: Called to reset a previous decompressor state and
//               clean up properly.
////////////////////////////////////////////////////////////////////
void Decompressor::
cleanup() {
  if (_source != (istream *)NULL) {
    delete _source;
    _source = NULL;
  }
  if (_dest != (ostream *)NULL) {
    delete _dest;
    _dest = NULL;
  }
  if (_decompress != (istream *)NULL) {
    delete _decompress;
    _decompress = NULL;
  }
}

#endif  // HAVE_ZLIB
