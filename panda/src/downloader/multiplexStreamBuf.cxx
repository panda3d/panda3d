/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file multiplexStreamBuf.cxx
 * @author drose
 * @date 2000-11-27
 */

#include "multiplexStreamBuf.h"

#if defined(WIN32_VC) || defined(WIN64_VC)
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#undef WINDOWS_LEAN_AND_MEAN
#endif

// We use real assert() instead of nassert(), because we're likely to be
// invoked directly by pnotify.here, and we don't want to risk infinite
// recursion.
#include <assert.h>

using std::string;

/**
 * Closes or deletes the relevant pointers, if _owns_obj is true.
 */
void MultiplexStreamBuf::Output::
close() {
  if (_owns_obj) {
    switch (_output_type) {
    case OT_ostream:
      assert(_out != nullptr);
      delete _out;
      break;

    case OT_stdio:
      assert(_fout != nullptr);
      fclose(_fout);
      break;

    default:
      break;
    }
  }
}

/**
 * Dumps the indicated string to the appropriate place.
 */
void MultiplexStreamBuf::Output::
write_string(const string &str) {
  switch (_output_type) {
  case OT_ostream:
    assert(_out != nullptr);
    _out->write(str.data(), str.length());
    _out->flush();
    break;

  case OT_stdio:
    assert(_fout != nullptr);
    fwrite(str.data(), str.length(), 1, _fout);
    fflush(_fout);
    break;

  case OT_system_debug:
#if defined(WIN32_VC) || defined(WIN64_VC)
    OutputDebugString(str.c_str());
#endif
    break;
  }
}

/**
 *
 */
MultiplexStreamBuf::
MultiplexStreamBuf() {
#ifndef PHAVE_IOSTREAM
  // Older iostream implementations required this.
  allocate();
  setp(base(), ebuf());
#endif
}

/**
 *
 */
MultiplexStreamBuf::
~MultiplexStreamBuf() {
  sync();

  // Make sure all of our owned pointers are freed.
  Outputs::iterator oi;
  for (oi = _outputs.begin(); oi != _outputs.end(); ++oi) {
    Output &out = (*oi);
    out.close();
  }
}

/**
 * Adds the indicated output destinition to the set of things that will be
 * written to when characters are output to the MultiplexStream.
 */
void MultiplexStreamBuf::
add_output(MultiplexStreamBuf::BufferType buffer_type,
           MultiplexStreamBuf::OutputType output_type,
           std::ostream *out, FILE *fout, bool owns_obj) {

  Output o;
  o._buffer_type = buffer_type;
  o._output_type = output_type;
  o._out = out;
  o._fout = fout;
  o._owns_obj = owns_obj;

  // Ensure that we have the mutex while we fiddle with the list of outputs.
  _lock.lock();
  _outputs.push_back(o);
  _lock.unlock();
}


/**
 * Forces out all output that hasn't yet been written.
 */
void MultiplexStreamBuf::
flush() {
  _lock.lock();
  write_chars("", 0, true);
  _lock.unlock();
}

/**
 * Called by the system ostream implementation when its internal buffer is
 * filled, plus one character.
 */
int MultiplexStreamBuf::
overflow(int ch) {
  _lock.lock();

  std::streamsize n = pptr() - pbase();

  if (n != 0) {
    write_chars(pbase(), n, false);
    pbump(-n);  // Reset pptr().
  }

  if (ch != EOF) {
    // Write one more character.
    char c = ch;
    write_chars(&c, 1, false);
  }

  _lock.unlock();
  return 0;
}

/**
 * Called by the system ostream implementation when the buffer should be
 * flushed to output (for instance, on destruction).
 */
int MultiplexStreamBuf::
sync() {
  _lock.lock();

  std::streamsize n = pptr() - pbase();

  // We pass in false for the flush value, even though our transmitting
  // ostream said to sync.  This allows us to get better line buffering, since
  // our transmitting ostream is often set unitbuf, and might call sync
  // multiple times in one line.  We still have an explicit flush() call to
  // force the issue.
  write_chars(pbase(), n, false);
  pbump(-n);

  _lock.unlock();
  return 0;  // Return 0 for success, EOF to indicate write full.
}

/**
 * An internal function called by sync() and overflow() to store one or more
 * characters written to the stream into the memory buffer.
 *
 * It is assumed that there is only one thread at a time running this code; it
 * is the responsibility of the caller to grab the _lock mutex before calling
 * this.
 */
void MultiplexStreamBuf::
write_chars(const char *start, int length, bool flush) {
  size_t orig = _line_buffer.length();
  string latest;
  if (length != 0) {
    latest = string(start, length);
  }
  string line;

  if (flush) {
    // If we're to flush the stream now, we dump the whole thing regardless of
    // whether we have reached end-of-line.
    line = _line_buffer + latest;
    _line_buffer = "";

  } else {
    // Otherwise, we check for the end-of-line character, for our ostreams
    // that only want a complete line at a time.
    _line_buffer += latest;
    size_t eol = _line_buffer.rfind('\n', orig);
    if (eol != string::npos) {
      line = _line_buffer.substr(0, eol + 1);
      _line_buffer = _line_buffer.substr(eol + 1);
    }
  }

  Outputs::iterator oi;
  for (oi = _outputs.begin(); oi != _outputs.end(); ++oi) {
    Output &out = (*oi);
    switch (out._buffer_type) {
    case BT_none:
      // No buffering: send all new characters directly to the ostream.
      if (!latest.empty()) {
        out.write_string(latest);
      }
      break;

    case BT_line:
      // Line buffering: send only when a complete line has been received.
      if (!line.empty()) {
        out.write_string(line);
      }
      break;
    }
  }
}
