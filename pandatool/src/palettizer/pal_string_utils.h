// Filename: pal_string_utils.h
// Created by:  drose (30Nov00)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef PAL_STRING_UTILS_H
#define PAL_STRING_UTILS_H

#include "pandatoolbase.h"
#include <string_utils.h>

class PNMFileType;

void extract_param_value(const string &str, string &param, string &value);

bool parse_image_type_request(const string &word, PNMFileType *&color_type,
                              PNMFileType *&alpha_type);

#endif

