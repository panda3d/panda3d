// Filename: find_searchpath.h
// Created by:  drose (09Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FIND_SEARCHPATH_H
#define FIND_SEARCHPATH_H

#include "ppremake.h"

#include <vector>

// Searchs for the given filename along the indicated set of
// directories, and returns the first place in which it is found, or
// empty string if it is not found.
string find_searchpath(const vector<string> &directories,
		       const string &filename);

#endif

