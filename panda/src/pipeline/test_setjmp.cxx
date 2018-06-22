/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file test_setjmp.cxx
 * @author drose
 * @date 2007-06-19
 */

#include "pandabase.h"

#include <setjmp.h>

using std::cerr;

int
main(int argc, char *argv[]) {

  // If we have ucontext.h, we don't need to use setjmp, so don't bother
  // trying to compile this program (it may not compile anyway).

#ifndef PHAVE_UCONTEXT_H
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
#endif  // PHAVE_UCONTEXT_H

  return 0;
}
