// Filename: pfstreamBuf.cxx
// Created by:  cary (12Dec00)
// 
////////////////////////////////////////////////////////////////////

#include "pfstreamBuf.h"

#ifndef HAVE_STREAMSIZE
// Some compilers (notably SGI) don't define this for us
typedef int streamsize;
#endif /* HAVE_STREAMSIZE */

PipeStreamBuf::PipeStreamBuf(void) {
#ifndef WIN32_VC
  // taken from Dr. Ose.
  // These lines, which are essential on Irix and Linux, seem to be
  // unnecessary and not understood on Windows.
  allocate();
  setp(base(), ebuf());
#endif /* WIN32_VC */
}

PipeStreamBuf::~PipeStreamBuf(void) {
  sync();
  // any other cleanup needed (pclose, etc)
}

void PipeStreamBuf::flush(void) {
  // if there's anything to do here
}

int PipeStreamBuf::overflow(int c) {
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
  streamsize n = pptr() - pbase();
  write_chars(pbase(), n, false);
  pbump(-n);
  return 0;
}

int PipeStreamBuf::underflow(void) {
}
