// Filename: pfstreamBuf.cxx
// Created by:  cary (12Dec00)
// 
////////////////////////////////////////////////////////////////////

#include "pfstreamBuf.h"
#include <assert.h>

#ifndef HAVE_STREAMSIZE
// Some compilers (notably SGI) don't define this for us
typedef int streamsize;
#endif /* HAVE_STREAMSIZE */

PipeStreamBuf::PipeStreamBuf(PipeStreamBuf::Direction dir) : _dir(dir),
							     _pipe((FILE*)0L) {
#ifndef WIN32_VC
  // taken from Dr. Ose.
  // These lines, which are essential on Irix and Linux, seem to be
  // unnecessary and not understood on Windows.
  allocate();
  assert((dir == Input) || (dir == Output));
  if (dir == Input) {
    cerr << "allocated reserve is " << blen() << " bytes long" << endl;
    setg(base(), ebuf(), ebuf());
  } else {
    setp(base(), ebuf());
  }
#endif /* WIN32_VC */
}

PipeStreamBuf::~PipeStreamBuf(void) {
  if (_pipe != (FILE*)0L) {
    sync();
    flush();
    // any other cleanup needed (pclose, etc)
    pclose(_pipe);
  }
}

void PipeStreamBuf::flush(void) {
  assert(_pipe != (FILE*)0L);
  if (_dir == Output) {
    write_chars("", 0, true);
  }
}

void PipeStreamBuf::command(const string cmd) {
  assert(_pipe == (FILE*)0L);
  const char *typ = (_dir == Output)?"w":"r";
  _pipe = popen(cmd.c_str(), typ);
}

int PipeStreamBuf::overflow(int c) {
  assert(_pipe != (FILE*)0L);
  assert(_dir == Output);
  streamsize n = pptr() - pbase();
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
  assert(_pipe != (FILE*)0L);
  if (_dir == Output) {
    streamsize n = pptr() - pbase();
    write_chars(pbase(), n, false);
    pbump(-n);
  } else {
    streamsize n = egptr() - gptr();
    gbump(n);  // flush all our stored input away
    cerr << "pfstream tossed out " << n << " bytes" << endl;
  }
  return 0;
}

int PipeStreamBuf::underflow(void) {
  assert(_pipe != (FILE*)0L);
  assert(_dir == Input);
  if (gptr() < egptr()) {
    char c = *(gptr());
    return c;
  }
  if (feof(_pipe) != 0)
    return EOF;
  //  size_t len = ebuf() - base();
  size_t len = 1024;
  char* buf = new char[len];
  size_t n = fread(buf, 1, len, _pipe);
  int ret = buf[0];
  if (n == 0)
    ret = EOF;
  else {
    //    memcpy(base()+(len - n), buf, n);
    gbump(-n);
  }
  delete buf;
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
  size_t wrote = fwrite(line.c_str(), 1, line.length(), _pipe);
  if (wrote != line.length())
    cerr << "wrote only " << wrote << " of " << line.length()
	 << " bytes to pipe" << endl;
  fflush(_pipe);
}
