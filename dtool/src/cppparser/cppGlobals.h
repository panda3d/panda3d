// Filename: cppGlobals.h
// Created by:  drose (16May00)
// 
////////////////////////////////////////////////////////////////////

#ifndef CPPGLOBALS_H
#define CPPGLOBALS_H

#include <dtoolbase.h>

// Some compilers (notably VC++) define a special keyword to represent
// a 64-bit integer, but don't recognize "long long int".  To parse
// (and generate) code for these compilers, set this string to the
// 64-bit integer typename keyword.
extern string cpp_longlong_keyword;


#endif


