// Filename: minmax.h -- <OneLineSynopsis>
// Description: <TooLazyToMakeOne>
// Created by:  drose (23Aug96)
//
// Revision Overview:
//
//
////////////////////////////////////////////////////////////////////
// Copyright (C) 1992,93,94,95        Walt Disney Imagineering, Inc.
//
// These  coded  instructions,  statements,  data   structures   and
// computer  programs contain unpublished proprietary information of
// Walt Disney Imagineering and are protected by  Federal  copyright
// law.  They may  not be  disclosed to third  parties  or copied or
// duplicated in any form, in whole or in part,  without  the  prior
// written consent of Walt Disney Imagineering Inc.
////////////////////////////////////////////////////////////////////
//
// RCSID:
// $Header$
//

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
