// Filename: minmax.h -- <OneLineSynopsis>
// Description: <TooLazyToMakeOne>
// Created by:  drose (23Aug96)
//
// Revision Overview:
//
//
////////////////////////////////////////////////////////////////////

#ifndef _PNMMINMAX_H_
#define _PNMMINMAX_H_

#include <pandabase.h>

#include <algorithm>

// We now inherit these functions from STL.

/*
template <class Type>
inline Type
max(const Type &a, const Type &b) {
  return (a<b) ? b : a;
}

template <class Type>
inline Type
min(const Type &a, const Type &b) {
  return (a<b) ? a : b;
}
*/

template <class Type>
inline Type
bounds(const Type &value, const Type &lower, const Type &upper) {
  return min(max(value, lower), upper);
}

#endif
