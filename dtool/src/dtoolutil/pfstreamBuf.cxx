/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pfstreamBuf.cxx
 * @author cary
 * @date 2000-12-12
 */

#include "pfstreamBuf.h"
#include <assert.h>

using std::cerr;
using std::endl;
using std::string;

PipeStreamBuf::PipeStreamBuf(PipeStreamBuf::Direction dir) :
  _dir(dir)
{
  init_pipe();

#ifndef PHAVE_IOSTREAM
  // These lines, which are essential on older implementations of the iostream
  // library, are not understood by more recent versions.
  allocate();
  assert((dir == Input) || (dir == Output));
  if (dir == Input) {
    setg(base(), ebuf(), ebuf());
  } else {
    setp(base(), ebuf());
  }
#endif /* PHAVE_IOSTREAM */
}

PipeStreamBuf::
~PipeStreamBuf(void) {
  if (is_open()) {
    sync();
    flush();
    close_pipe();
  }
}

void PipeStreamBuf::flush(void) {
  assert(is_open());
  if (_dir == Output) {
    write_chars("", 0, true);
  }
}

void PipeStreamBuf::command(const string cmd) {
  assert(!is_open());
  open_pipe(cmd);
}

int PipeStreamBuf::overflow(int c) {
  assert(is_open());
  assert(_dir == Output);
  std::streamsize n = pptr() - pbase();
  if (n != 0) {
    write_chars(pbase(), n, false);
    pbump(-n);  // reset pptr()
  }
  if (c != EOF) {
    // write one more character
    char ch = c;
    write_chars(&ch, 1, false);
  }
  return 0;
}

int PipeStreamBuf::sync(void) {
  assert(is_open());
  if (_dir == Output) {
    std::streamsize n = pptr() - pbase();
    write_chars(pbase(), n, false);
    pbump(-n);
  } else {
    std::streamsize n = egptr() - gptr();
    if (n != 0) {
      gbump(n);  // flush all our stored input away
#ifndef NDEBUG
      cerr << "pfstream tossed out " << n << " bytes" << endl;
#endif
    }
  }
  return 0;
}

int PipeStreamBuf::underflow(void) {
  assert(_dir == Input);
  if ((eback() == nullptr) || (gptr() == nullptr) ||
      (egptr() == nullptr)) {
    // must be new-style iostream library
    char* buf = new char[4096];
    char* ebuf = &(buf[4096]);
    setg(buf, ebuf, ebuf);
  }
  if (gptr() < egptr()) {
    char c = *(gptr());
    return c;
  }
  if (eof_pipe()) {
    return EOF;
  }
#ifdef PHAVE_IOSTREAM
  size_t len = 4096;
#else /* PHAVE_IOSTREAM */
  size_t len = ebuf() - base();
#endif /* PHAVE_IOSTREAM */
  char* buf = new char[len];
  size_t n = read_pipe(buf, len);
  int ret = buf[0];
  if (n == 0)
    ret = EOF;
  else {
#ifdef PHAVE_IOSTREAM
    memcpy(eback()+(len-n), buf, n);
#else /* PHAVE_IOSTREAM */
    memcpy(base()+(len-n), buf, n);
#endif /* PHAVE_IOSTREAM */
    gbump(-((int)n));
  }
  delete[] buf;
  return ret;
}

void PipeStreamBuf::write_chars(const char* start, int length, bool flush) {
  assert(_dir == Output);
  size_t orig = _line_buffer.length();
  string latest(start, length);
  string line;

  if (flush) {
    // if we're going to flush the stream now, we dump the whole thing
    // reguardless of whether we have reached end-of-line.
    line = _line_buffer + latest;
    _line_buffer = "";
  } else {
    // Otherwise, we check for the end-of-line character.
    _line_buffer += latest;
    size_t eol = _line_buffer.rfind('\n', orig);
    if (eol != string::npos) {
      line = _line_buffer.substr(0, eol+1);
      _line_buffer = _line_buffer.substr(eol+1);
    }
  }
  // now output 'line'
  size_t wrote = write_pipe(line.c_str(), line.length());
#ifndef NDEBUG
  if (wrote != line.length())
    cerr << "wrote only " << wrote << " of " << line.length()
         << " bytes to pipe" << endl;
#endif
}

#ifndef WIN_PIPE_CALLS

/**
 * Initializes whatever data structures store the child process information.
 * This function is only called once at startup, by the constructor.
 */
void PipeStreamBuf::
init_pipe() {
  _pipe = nullptr;
}

/**
 * Returns true if the pipe has been opened, false otherwise.
 */
bool PipeStreamBuf::
is_open() const {
  return _pipe != nullptr;
}

/**
 * Returns true if there is an end-of-file condition on the input, or if the
 * pipe was never opened.
 */
bool PipeStreamBuf::
eof_pipe() const {
  return (_pipe == nullptr) && feof(_pipe);
}

/**
 * Forks a child to run the indicated command, and according to the setting of
 * _dir, binds either its input or output to this process for writing or
 * reading.
 *
 * Returns true on success, false on failure.
 */
bool PipeStreamBuf::
open_pipe(const string &cmd) {
  const char *typ = (_dir == Output)?"w":"r";
  _pipe = popen(cmd.c_str(), typ);
  return (_pipe != nullptr);
}

/**
 * Closes the pipe opened previously.
 */
void PipeStreamBuf::
close_pipe() {
  if (_pipe != nullptr) {
    fclose(_pipe);
    _pipe = nullptr;
  }
}

/**
 * Writes the indicated data out to the child process opened previously.
 * Returns the number of bytes read.
 */
size_t PipeStreamBuf::
write_pipe(const char *data, size_t len) {
  size_t wrote_count = fwrite(data, 1, len, _pipe);
  fflush(_pipe);
  return wrote_count;
}

/**
 * Reads the indicated amount of data from the child process opened
 * previously.  Returns the number of bytes read.
 */
size_t PipeStreamBuf::
read_pipe(char *data, size_t len) {
  return fread(data, 1, len, _pipe);
}

#else  // WIN_PIPE_CALLS

// The official Windows way of reading from a child process, without using a
// Unix-style convenience function like popen(), is similar in principle to
// the Unix pipe() method.  We have to first redirect our own stdout to an
// anonymous pipe, then we spawn a child, who inherits this new stdout.  Then
// we can restore our own stdout, and read from the other end of the pipe.

/**
 * Initializes whatever data structures store the child process information.
 * This function is only called once at startup, by the constructor.
 */
void PipeStreamBuf::
init_pipe() {
  _child_out = 0;
}

/**
 * Returns true if the pipe has been opened, false otherwise.
 */
bool PipeStreamBuf::
is_open() const {
  return (_child_out != 0);
}

/**
 * Returns true if there is an end-of-file condition on the input, or if the
 * pipe was never opened.
 */
bool PipeStreamBuf::
eof_pipe() const {
  return (_child_out == 0);
}

/**
 * Forks a child to run the indicated command, and according to the setting of
 * _dir, binds either its input or output to this process for writing or
 * reading.
 *
 * Returns true on success, false on failure.
 */
bool PipeStreamBuf::
open_pipe(const string &cmd) {
  close_pipe();

  // At the present, this only works for input pipes.  We can add code to
  // support output pipes later if anyone cares.
  if (_dir == Output) {
    return false;
  }

  // First, save our current stdout, so we can restore it after all of this
  // nonsense.
  HANDLE hSaveStdout = GetStdHandle(STD_OUTPUT_HANDLE);

  // Now create a pipe to accept the child processes' output.
  HANDLE hChildStdoutRd, hChildStdoutWr;

  // Set the bInheritHandle flag so pipe handles are inherited.
  SECURITY_ATTRIBUTES saAttr;
  saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
  saAttr.bInheritHandle = TRUE;
  saAttr.lpSecurityDescriptor = nullptr;
  if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0)) {
#ifndef NDEBUG
    cerr << "Unable to create output pipe\n";
#endif
    return false;
  }

  // Remap stdout to the "write" end of this pipe.
  if (!SetStdHandle(STD_OUTPUT_HANDLE, hChildStdoutWr)) {
#ifndef NDEBUG
    cerr << "Unable to redirect stdout\n";
#endif
    CloseHandle(hChildStdoutRd);
    CloseHandle(hChildStdoutWr);
    return false;
  }

  // Create noninheritable read handle and close the inheritable read handle.

  BOOL fSuccess = DuplicateHandle(GetCurrentProcess(), hChildStdoutRd,
                                  GetCurrentProcess(), &_child_out,
                                  0, FALSE, DUPLICATE_SAME_ACCESS);

  if (!fSuccess) {
#ifndef NDEBUG
    cerr << "DuplicateHandle failed\n";
#endif
    CloseHandle(hChildStdoutRd);
    CloseHandle(hChildStdoutWr);
    return false;
  }
  CloseHandle(hChildStdoutRd);

  // Now spawn the child process.

  // Both WinExec() and CreateProcess() want a non-const char pointer.  Maybe
  // they change it, and maybe they don't.  I'm not taking chances.
  char *cmdline = new char[cmd.length() + 1];
  strcpy(cmdline, cmd.c_str());

  // We should be using CreateProcess() instead of WinExec(), but that seems
  // to be likely to crash Win98.  WinExec() seems better behaved, and it's
  // all we need anyway.
  if (!WinExec(cmdline, 0)) {
#ifndef NDEBUG
    cerr << "Unable to spawn process.\n";
#endif
    close_pipe();
    // Don't return yet, since we still need to clean up.
  }

  delete[] cmdline;

  // Now restore our own stdout, up here in the parent process.
  if (!SetStdHandle(STD_OUTPUT_HANDLE, hSaveStdout)) {
#ifndef NDEBUG
    cerr << "Unable to restore stdout\n";
#endif
  }

  // Close the write end of the pipe before reading from the read end of the
  // pipe.
  if (!CloseHandle(hChildStdoutWr)) {
#ifndef NDEBUG
    cerr << "Unable to close write end of pipe\n";
#endif
  }

  return (_child_out != 0);
}

/**
 * Closes the pipe opened previously.
 */
void PipeStreamBuf::
close_pipe() {
  if (_child_out != 0) {
    CloseHandle(_child_out);
    _child_out = 0;
  }
}

/**
 * Writes the indicated data out to the child process opened previously.
 * Returns the number of bytes read.
 */
size_t PipeStreamBuf::
write_pipe(const char *data, size_t len) {
  return 0;
}

/**
 * Reads the indicated amount of data from the child process opened
 * previously.  Returns the number of bytes read.
 */
size_t PipeStreamBuf::
read_pipe(char *data, size_t len) {
  if (_child_out == 0) {
    return 0;
  }
  DWORD dwRead;
  if (!ReadFile(_child_out, data, len, &dwRead, nullptr)) {
    close_pipe();
    return 0;
  }

  return dwRead;
}


#endif  // WIN_PIPE_CALLS
