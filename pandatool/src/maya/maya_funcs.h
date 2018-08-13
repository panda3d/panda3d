/**
 * PANDA 3D SOFTWARE
 * Copyright (c) Carnegie Mellon University.  All rights reserved.
 *
 * All use of this software is subject to the terms of the revised BSD
 * license.  You should have received a copy of this license along
 * with this source code in a file named "LICENSE."
 *
 * @file maya_funcs.h
 * @author drose
 * @date 2000-02-16
 */

#ifndef MAYA_FUNCS_H
#define MAYA_FUNCS_H

#include "pandatoolbase.h"
#include "luse.h"
#include "config_maya.h"

#include "pre_maya_include.h"
#include <maya/MFnAttribute.h>
#include <maya/MPlug.h>
#include <maya/MStatus.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MObject.h>
#include <maya/MString.h>
#include <maya/MVector.h>
#include "post_maya_include.h"


bool
get_maya_plug(MObject &node, const std::string &attribute_name, MPlug &plug);

bool
is_connected(MObject &node, const std::string &attribute_name);

template<class ValueType>
bool
get_maya_attribute(MObject &node, const std::string &attribute_name,
                   ValueType &value);

template<class ValueType>
bool
set_maya_attribute(MObject &node, const std::string &attribute_name,
                   ValueType &value);

bool
has_attribute(MObject &node, const std::string &attribute_name);

bool
remove_attribute(MObject &node, const std::string &attribute_name);

bool
get_bool_attribute(MObject &node, const std::string &attribute_name,
                   bool &value);

bool
get_angle_attribute(MObject &node, const std::string &attribute_name,
                    double &value);

bool
get_vec2_attribute(MObject &node, const std::string &attribute_name,
                    LVecBase2 &value);

bool
get_vec3_attribute(MObject &node, const std::string &attribute_name,
                    LVecBase3 &value);

bool
get_vec2d_attribute(MObject &node, const std::string &attribute_name,
                    LVecBase2d &value);

bool
get_vec3d_attribute(MObject &node, const std::string &attribute_name,
                    LVecBase3d &value);

bool
get_mat4d_attribute(MObject &node, const std::string &attribute_name,
                    LMatrix4d &value);

void
get_tag_attribute_names(MObject &node, pvector<std::string> &tag_names);

bool
get_enum_attribute(MObject &node, const std::string &attribute_name,
                   std::string &value);

bool
get_string_attribute(MObject &node, const std::string &attribute_name,
                     std::string &value);

bool
set_string_attribute(MObject &node, const std::string &attribute_name,
                     const std::string &value);

void
describe_maya_attribute(MObject &node, const std::string &attribute_name);

bool
describe_compound_attribute(MObject &node);

std::string
string_mfndata_type(MFnData::Type type);

void
list_maya_attributes(MObject &node);

// Also, we must define some output functions for Maya objects, since we can't
// use those built into Maya (which forward-defines the ostream type
// incorrectly).
INLINE std::ostream &operator << (std::ostream &out, const MString &str);
INLINE std::ostream &operator << (std::ostream &out, const MVector &vec);

#include "maya_funcs.I"
#include "maya_funcs.T"

#endif
