// Filename: patcher.cxx
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////
#include "patcher.h"
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
//       Class : PatcherToken
// Description : Holds a request for the patcher.
////////////////////////////////////////////////////////////////////
class PatcherToken : public ReferenceCount {
public:
  INLINE PatcherToken(uint id, const Filename &patch, 
	   	    const Filename &infile, const string &event_name) {
    _id = id;
    _patch = patch;
    _infile = infile;
    _event_name = event_name;
  }
  int _id;
  Filename _patch;
  Filename _infile;
  string _event_name;
};

////////////////////////////////////////////////////////////////////
//     Function: Patcher::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patcher::
Patcher(void) : AsyncUtility() {
  PT(Buffer) buffer = new Buffer(patcher_buffer_size);
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Patcher::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patcher::
Patcher(PT(Buffer) buffer) : AsyncUtility() {
  init(buffer);
}

////////////////////////////////////////////////////////////////////
//     Function: Patcher::Constructor
//       Access: Private 
//  Description:
////////////////////////////////////////////////////////////////////
void Patcher::
init(PT(Buffer) buffer) {
  nassertv(!buffer.is_null());
  _token_board = new PatcherTokenBoard;
  _buffer = buffer;
}

////////////////////////////////////////////////////////////////////
//     Function: Patcher::Destructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
Patcher::
~Patcher(void) {
  destroy_thread();

  delete _token_board;
}

////////////////////////////////////////////////////////////////////
//     Function: Patcher::request_patch
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
int Patcher::
request_patch(const Filename &patch, const Filename &infile,
			const string &event_name) {
  PT(PatcherToken) tok;
  if (_threads_enabled) {

    // Make sure we actually are threaded
    if (!_threaded) {
      downloader_cat.info()
        << "Patcher::request_patch() - create_thread() was "
        << "never called!  Calling it now..." << endl;
      create_thread();
    }

    // We need to grab the lock in order to signal the condition variable
    _lock.lock();

      if (_token_board->_waiting.is_full()) {
        downloader_cat.error()
          << "Patcher::request_patch() - Too many pending requests\n";
        return 0;
      }

      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Patch requested for file: " << infile << endl;
      }

      tok = new PatcherToken(_next_token++, patch, infile, event_name);
      _token_board->_waiting.insert(tok);

      _request_cond->signal();

    _lock.unlock();

  } else {
    // If we're not running asynchronously, process the load request
    // directly now.
    if (_token_board->_waiting.is_full()) {
      downloader_cat.error()
        << "Patcher::request_patch() - Too many pending requests\n";
      return 0;
    }
    if (downloader_cat.is_debug()) {
      downloader_cat.debug()
        << "Patch requested for file: " << infile << endl; 
    }

    tok = new PatcherToken(_next_token++, patch, infile, event_name);
    _token_board->_waiting.insert(tok);
    process_request();
  }

  return tok->_id;
}

////////////////////////////////////////////////////////////////////
//     Function: Patcher::process_request
//       Access: Private
//  Description: Serves any requests on the token board, moving them
//               to the done queue.
////////////////////////////////////////////////////////////////////
bool Patcher::
process_request() {
  if (_shutdown) {
    if (downloader_cat.is_debug())
      downloader_cat.debug()
        << "Patcher shutting down...\n";
    return false;
  }

  // If there is actually a request token - process it
  while (!_token_board->_waiting.is_empty()) {
    PT(PatcherToken) tok = _token_board->_waiting.extract();
    if (patch(tok->_patch, tok->_infile)) {
      _token_board->_done.insert(tok);

      // Throw a "done" event now.
      if (!tok->_event_name.empty()) {
        PT_Event done = new Event(tok->_event_name);
        done->add_parameter(EventParameter((int)tok->_id));
        throw_event(done);
      }

      if (downloader_cat.is_debug()) {
        downloader_cat.debug()
          << "Patcher::process_request() - patching complete for " 
	  << tok->_infile << "\n";
      }
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: Patcher::patch 
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
bool Patcher::
patch(Filename &patch, Filename &infile) {
  Patchfile pfile(_buffer);
  return pfile.apply(patch, infile);
}
