// Filename: filename.h
// Created by:  drose (19Oct00)
// 
////////////////////////////////////////////////////////////////////

#ifndef FILENAME_H
#define FILENAME_H

#include "ppremake.h"

// This header file defines a few functions handy for dealing with
// filenames in a cross-platform world.

bool is_fullpath(const string &pathname);
string to_os_filename(string pathname);
string to_unix_filename(string pathname);

#endif

