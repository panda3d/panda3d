// Filename: test_ordered_vector.cxx
// Created by:  drose (20Feb02)
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

#include "ordered_vector.h"

typedef ov_multiset<int> myvec;

void
search(myvec &v, int element) {
  pair<myvec::const_iterator, myvec::const_iterator> result;
  
  result = v.equal_range(element);
  size_t count = v.count(element);

  cerr << element << " bounded by " << result.first - v.begin() << " and " 
       << result.second - v.begin() << "; " << count << " total.\n";
}

int
main(int argc, char *argv[]) {
  myvec a, b;

  myvec::iterator mi;
  mi = a.insert(a.end(), 3);
  mi = a.insert(mi, 5);
  mi = a.insert(mi, 4);
  mi = a.insert(mi, 4);
  mi = a.insert(mi, 2);
  mi = a.insert(mi, 5);
  mi = a.insert(mi, 2);

  a.swap(b);

  cerr << b.size() << " elements:\n";

  myvec::iterator bi;
  for (bi = b.begin(); bi != b.end(); ++bi) {
    cerr << *bi << " ";
  }
  cerr << "\n";

  search(b, 1);
  search(b, 2);
  search(b, 3);
  search(b, 4);
  search(b, 5);
  search(b, 6);

  cerr << "Removing 4:\n";
  size_t count = b.erase(4);
  cerr << "Removed " << count << "\n";


  cerr << b.size() << " elements:\n";

  for (bi = b.begin(); bi != b.end(); ++bi) {
    cerr << *bi << " ";
  }
  cerr << "\n";

  search(b, 1);
  search(b, 2);
  search(b, 3);
  search(b, 4);
  search(b, 5);
  search(b, 6);

  return (0);
}
