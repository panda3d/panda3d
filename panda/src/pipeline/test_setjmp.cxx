// Filename: test_setjmp.cxx
// Created by:  drose (19Jun07)
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

#include <setjmp.h>

int
main(int argc, char *argv[]) {
  jmp_buf buf1, buf2;
  char * volatile scratch;

  setjmp(buf1);
  scratch = (char *)alloca(1024);
  setjmp(buf2);

  size_t word_size = sizeof(buf1[0]);
  size_t num_words = sizeof(buf1) / word_size;
  
  cerr << num_words << " words of " << word_size << " bytes\n";
  for (size_t i = 0; i < num_words; ++i) {
    cerr << "  word " << i << ": " << (void *)buf1[i] << " vs. "
         << (void *)buf2[i] << ", delta " << buf1[i] - buf2[i] << "\n";
  }
  cerr << "scratch = " << (void *)scratch << "\n";
  cerr << "scratch end = " << (void *)(scratch + 1024) << "\n";
    
  return 0;
}
