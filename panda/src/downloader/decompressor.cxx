// Filename: decompressor.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

// This file is compiled only if we have zlib installed.

////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "decompressor.h"
#include "config_downloader.h"

#include <event.h>
#include <pt_Event.h>
#include <throw_event.h>
#include <eventParameter.h>
#include <filename.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////
// Defines
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//       Class : DecompressorToken
// Description : Holds a request for the decompressor.
////////////////////////////////////////////////////////////////////
class DecompressorToken : public ReferenceCount {
public:
  INLINE DecompressorToken(uint id, const Filename &source_file, 
	   	    const Filename &dest_file, const string &event_name) {
    _id = id;
    _source_file = source_file;
    _dest_file = dest_file;
    _event_name = event_name;
  }
  int _id;
  Filename _source_file;
  Filename _dest_file;
  string _event_name;
};

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Decompressor::
Decompressor(void) : AsyncUtility() {
  PT(Buffer) buffer = new Buffer(decompressor_buffer_size);
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Decompressor::
Decompressor(PT(Buffer) buffer) : AsyncUtility() {
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Constructor
//       Access: Private 
//  Description:
////////////////////////////////////////////////////////////////////
void Decompressor::
init(PT(Buffer) buffer) {
  nassertv(!buffer.is_null());
  _frequency = decompressor_frequency;
  _token_board = new DecompressorTokenBoard;
  _half_buffer_length = buffer->get_length()/2; 
  _buffer = buffer;
  char *temp_name = tempnam(NULL, "dc");
  _temp_file_name = temp_name;
  _temp_file_name.set_binary();
  delete temp_name;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Decompressor::
~Decompressor(void) {
  destroy_thread();

  delete _token_board;
  _temp_file_name.unlink();
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::request_decompress
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int Decompressor::
request_decompress(const Filename &source_file, const string &event_name) {
  Filename dest_file = source_file;
  string extension = source_file.get_extension();
  if (extension == "pz")
    dest_file = source_file.get_fullpath_wo_extension();
  else {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Decompressor::request_decompress() - Unknown file extension: ."
        << extension << endl; 
  }
  return request_decompress(source_file, dest_file, event_name);
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::request_decompress
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
int Decompressor::
request_decompress(const Filename &source_file, const Filename &dest_file,
		   const string &event_name) {

  PT(DecompressorToken) tok;
  if (_threads_enabled) {

    // Make sure we actually are threaded
    if (!_threaded) {
      downloader_cat.info()
        << "Decompressor::request_decompress() - create_thread() was "
        << "never called!  Calling it now..." << endl;
      create_thread();
    }

    // We need to grab the lock in order to signal the condition variable
#ifdef HAVE_IPC
    _lock.lock();
#endif

      if (_token_board->_waiting.is_full()) {
        downloader_cat.error()
          << "Downloader::request_download() - Too many pending requests\n";
        return 0;
      }

      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Decompress requested for file: " << source_file << endl;
      }

      tok = new DecompressorToken(_next_token++, source_file, dest_file,
					event_name);
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
        << "Downloader::request_download() - Too many pending requests\n";
      return 0;
    }
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << "Decompress requested for file: " << source_file << endl; 
    }

    tok = new DecompressorToken(_next_token++, source_file, dest_file,
					event_name); 
    _token_board->_waiting.insert(tok);
    process_request();
  }

  return tok->_id;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::process_request
//       Access: Private
//  Description: Serves any requests on the token board, moving them
//               to the done queue.
////////////////////////////////////////////////////////////////////
bool Decompressor::
process_request() {
  if (_shutdown) {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Decompressor shutting down...\n";
    return false;
  }

  // If there is actually a request token - process it
  while (!_token_board->_waiting.is_empty()) {
    PT(DecompressorToken) tok = _token_board->_waiting.extract();
    if (decompress(tok->_source_file, tok->_dest_file)) {
      _token_board->_done.insert(tok);

      // Throw a "done" event now.
      if (!tok->_event_name.empty()) {
        PT_Event done = new Event(tok->_event_name);
        done->add_parameter(EventParameter((int)tok->_id));
        throw_event(done);
      }

      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Decompressor::process_request() - decompress complete for " 
	  << tok->_source_file << "\n";
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::decompress
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Decompressor::
decompress(Filename &source_file) {
  Filename dest_file = source_file;
  string extension = source_file.get_extension();
  if (extension == "pz")
    dest_file = source_file.get_fullpath_wo_extension();
  else {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Decompressor::request_decompress() - Unknown file extension: ."
        << extension << endl;
    return false;
  }
  return decompress(source_file, dest_file);
}

////////////////////////////////////////////////////////////////////
//     Function: Decompressor::decompress
//       Access: Public 
//  Description:
////////////////////////////////////////////////////////////////////
bool Decompressor::
decompress(Filename &source_file, Filename &dest_file) {

  // Open source file
  ifstream read_stream;
  source_file.set_binary();
  if (!source_file.open_read(read_stream)) {
    downloader_cat.error()
      << "Decompressor::decompress() - Error opening source file: " 
      << source_file << endl;
    return false;
  } 

  // Determine source file length
  read_stream.seekg(0, ios::end);
  int source_file_length = read_stream.tellg();
  if (source_file_length == 0) {
    downloader_cat.warning()
      << "Decompressor::decompress() - Zero length file: "
      << source_file << endl;
    return true;
  }
  read_stream.seekg(0, ios::beg);

  // Open destination file 
  ofstream write_stream;
  dest_file.set_binary();
  if (!dest_file.open_write(write_stream)) {
    downloader_cat.error()
      << "Decompressor::decompress() - Error opening dest file: " 
      << source_file << endl;
    return false;
  } 

  // Read from the source file into the first half of the buffer,
  // decompress into the second half of the buffer, write the second
  // half of the buffer to disk, and repeat.
  int total_bytes_read = 0;
  bool read_all_input = false;
  bool handled_all_input = false;
  int source_buffer_length;
  ZDecompressor decompressor;
  while (handled_all_input == false) {

    // See if there is anything left in the source file
    if (read_all_input == false) {
      read_stream.read(_buffer->_buffer, _half_buffer_length);
      source_buffer_length = read_stream.gcount();
      total_bytes_read += source_buffer_length;
      if (read_stream.eof()) {
	nassertr(total_bytes_read == source_file_length, false);
	read_all_input = true;
      }
    }

    char *next_in = _buffer->_buffer;
    int avail_in = source_buffer_length;
    char *dest_buffer = _buffer->_buffer + source_buffer_length;
    char *next_out = dest_buffer;
    int dest_buffer_length = _buffer->get_length() - source_buffer_length;
    int avail_out = dest_buffer_length;
    nassertr(avail_out > 0 && avail_in > 0, false);

    while (avail_in > 0) {
      int ret = decompressor.decompress_to_stream(next_in, avail_in,
			next_out, avail_out, dest_buffer, 
			dest_buffer_length, write_stream);
      if (ret == ZCompressorBase::S_error)
	return false;
      if ((int)decompressor.get_total_in() == source_file_length &&
	  avail_out == dest_buffer_length)
	handled_all_input = true;
    }

    nap();

  }

  read_stream.close();
  write_stream.close();

  source_file.unlink();

  return true;
}
