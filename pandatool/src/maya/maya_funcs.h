// Filename: maya_funcs.h
// Created by:  drose (16Feb00)
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

#ifndef MAYA_FUNCS_H
#define MAYA_FUNCS_H

#include <pandatoolbase.h>

#include <luse.h>

#include <string>

class MObject;

template<class ValueType>
bool
get_maya_attribute(MObject &node, const string &attribute_name,
                   ValueType &value);

bool
get_bool_attribute(MObject &node, const string &attribute_name,
                   bool &value);

bool
get_angle_attribute(MObject &node, const string &attribute_name,
                    double &value);

bool
get_vec2f_attribute(MObject &node, const string &attribute_name,
                    LVecBase2f &value);

bool
get_vec2d_attribute(MObject &node, const string &attribute_name,
                    LVecBase2d &value);

bool
get_string_attribute(MObject &node, const string &attribute_name,
                     string &value);

void
describe_maya_attribute(MObject &node, const string &attribute_name);

#include "maya_funcs.I"

#endif
