// Filename: asyncExtractor.cxx
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
#include "asyncExtractor.h"
#include "config_downloader.h"

#include <event.h>
#include <pt_Event.h>
#include <throw_event.h>
#include <eventParameter.h>
#include <filename.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : ExtractorToken
// Description : Holds a request for the extractor.
////////////////////////////////////////////////////////////////////
class ExtractorToken : public ReferenceCount {
public:
  INLINE ExtractorToken(uint id, const Filename &source_file,
                        const string &event_name,
                        const Filename &rel_path) {
    _id = id;
    _source_file = source_file;
    _event_name = event_name;
    _rel_path = rel_path;
  }
  int _id;
  Filename _source_file;
  string _event_name;
  Filename _rel_path;
};

////////////////////////////////////////////////////////////////////
//     Function: Extractor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Extractor::
Extractor(void) : AsyncUtility() {
  PT(Buffer) buffer = new Buffer(extractor_buffer_size);
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Extractor::
Extractor(PT(Buffer) buffer) : AsyncUtility() {
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::Constructor
//       Access: Private
//  Description:
////////////////////////////////////////////////////////////////////
void Extractor::
init(PT(Buffer) buffer) {
  nassertv(!buffer.is_null());
  _frequency = extractor_frequency;
  _token_board = new ExtractorTokenBoard;
  _buffer = buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Extractor::
~Extractor(void) {
  destroy_thread();

  delete _token_board;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::request_extract
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Extractor::
request_extract(const Filename &source_file, const string &event_name,
                const Filename &rel_path) {

  PT(ExtractorToken) tok;
  if (_threads_enabled) {

    // Make sure we actually are threaded
    if (!_threaded) {
      downloader_cat.info()
        << "Extractor::request_extract() - create_thread() was "
        << "never called!  Calling it now..." << endl;
      create_thread();
    }

    // We need to grab the lock in order to signal the condition variable
#ifdef HAVE_IPC
    _lock.lock();
#endif

      if (_token_board->_waiting.is_full()) {
        downloader_cat.error()
          << "Extractor::request_extract() - Too many pending requests\n";
        return 0;
      }

      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Extract requested for file: " << source_file << endl;
      }

      tok = new ExtractorToken(_next_token++, source_file, event_name,
                                                rel_path);
      _token_board->_waiting.insert(tok);

#ifdef HAVE_IPC
      _request_cond->signal();
    _lock.unlock();
#endif

  } else {
    // If we're not running asynchronously, process the load request
    // directly now.
    if (_token_board->_waiting.is_full()) {
      downloader_cat.error()
        << "Extractor::request_extract() - Too many pending requests\n";
      return 0;
    }
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << "Extract requested for file: " << source_file << endl;
    }

    tok = new ExtractorToken(_next_token++, source_file, event_name,
                                                rel_path);
    _token_board->_waiting.insert(tok);
    process_request();
  }

  return tok->_id;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::process_request
//       Access: Private
//  Description: Serves any requests on the token board, moving them
//               to the done queue.
////////////////////////////////////////////////////////////////////
bool Extractor::
process_request() {
  if (_shutdown) {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Extractor shutting down...\n";
    return false;
  }

  // If there is actually a request token - process it
  while (!_token_board->_waiting.is_empty()) {
    PT(ExtractorToken) tok = _token_board->_waiting.extract();
    if (extract(tok->_source_file, tok->_rel_path)) {
      _token_board->_done.insert(tok);

      // Throw a "done" event now.
      if (!tok->_event_name.empty()) {
        PT_Event done = new Event(tok->_event_name);
        done->add_parameter(EventParameter((int)tok->_id));
        throw_event(done);
      }

      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Extractor::process_request() - extract complete for "
          << tok->_source_file << "\n";
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Extractor::extract
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Extractor::
extract(Filename &source_file, const Filename &rel_path) {

  // Open source file
  ifstream read_stream;
  source_file.set_binary();
  if (!source_file.open_read(read_stream)) {
    downloader_cat.error()
      << "Extractor::extract() - Error opening source file: "
      << source_file << endl;
    return false;
  }

  // Determine source file length
  read_stream.seekg(0, ios::end);
  int source_file_length = read_stream.tellg();
  read_stream.seekg(0, ios::beg);

  // Read the multifile header
  Multifile mfile;

  // Read from the source file and write to the appropriate extracted file
  int total_bytes_read = 0;
  bool read_all_input = false;
  bool handled_all_input = false;
  int source_buffer_length;
  while (handled_all_input == false) {

    // See if there is anything left in the source file
    if (read_all_input == false) {
      read_stream.read(_buffer->_buffer, _buffer->get_length());
      source_buffer_length = read_stream.gcount();
      total_bytes_read += source_buffer_length;
      if (read_stream.eof()) {
        nassertr(total_bytes_read == source_file_length, false);
        read_all_input = true;
      }
    }

    // Write to the out file
    char *start = _buffer->_buffer;
    int size = source_buffer_length;
    if (mfile.write_extract(start, size, rel_path) == true)
      handled_all_input = true;

    nap();
  }

  read_stream.close();
  source_file.unlink();

  return true;
}
