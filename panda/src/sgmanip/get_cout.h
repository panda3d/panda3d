// Filename: get_cout.h
// Created by:  drose (23Feb00)
// 
////////////////////////////////////////////////////////////////////

#ifndef GET_COUT_H
#define GET_COUT_H

// This file defines some general object-placement functions for scene
// graph operations.  These are all higher-level wrappers around more
// fundamental scene graph manipulations, and are most useful from a
// scripting-language point of view.

#include <pandabase.h>

// These are just handy hooks to export these so a non-C++ scripting
// language can access iostream objects for input and output.  When
// interrogate supports exporting system globals directly, the need
// for these will go away.

INLINE ostream &get_cout() {
  return cout;
}

INLINE ostream &get_cerr() {
  return cerr;
}

INLINE istream &get_cin() {
  return cin;
}

#endif

