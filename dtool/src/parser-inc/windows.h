// Filename: windows.h
// Created by:  drose (17Aug00)
// 
////////////////////////////////////////////////////////////////////

// This file, and all the other files in this directory, aren't
// intended to be compiled--they're just parsed by CPPParser (and
// interrogate) in lieu of the actual system headers, to generate the
// interrogate database.

#ifndef WINDOWS_H
#define WINDOWS_H

typedef bool BOOL;

union LARGE_INTEGER {
  __int64 QuadPart;
};  
