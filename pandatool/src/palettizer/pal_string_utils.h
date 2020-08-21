/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file pal_string_utils.h
 * @author drose
 * @date 2000-11-30
 */

#ifndef PAL_STRING_UTILS_H
#define PAL_STRING_UTILS_H

#include "pandatoolbase.h"
#include "string_utils.h"

class PNMFileType;

void extract_param_value(const std::string &str, std::string &param, std::string &value);

bool parse_image_type_request(const std::string &word, PNMFileType *&color_type,
                              PNMFileType *&alpha_type);

#endif
