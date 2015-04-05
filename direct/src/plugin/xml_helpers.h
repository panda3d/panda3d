// Filename: xml_helpers.h
// Created by:  drose (28Sep12)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef XML_HELPERS_H
#define XML_HELPERS_H

#include "get_tinyxml.h"

bool parse_bool_attrib(TiXmlElement *xelem, const string &attrib,
                       bool default_value);

#endif

