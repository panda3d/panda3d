// Filename: maya_funcs.cxx
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

#include "maya_funcs.h"

#include "pre_maya_include.h"
#include <maya/MObject.h>
#include <maya/MAngle.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MStatus.h>
#include <maya/MFnStringData.h>
#include <maya/MFnNumericData.h>
#include "post_maya_include.h"

bool
get_bool_attribute(MObject &node, const string &attribute_name,
                   bool &value) {
  if (!get_maya_attribute(node, attribute_name, value)) {
    nout << "Attribute " << attribute_name
         << " does not have an bool value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }
  return true;
}

bool
get_angle_attribute(MObject &node, const string &attribute_name,
                    double &value) {
  MAngle maya_value;
  if (!get_maya_attribute(node, attribute_name, maya_value)) {
    nout << "Attribute " << attribute_name
         << " does not have an angle value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }
  value = maya_value.asDegrees();
  return true;
}

bool
get_vec2f_attribute(MObject &node, const string &attribute_name,
                    LVecBase2f &value) {
  MStatus status;

  MObject vec2f_object;
  if (!get_maya_attribute(node, attribute_name, vec2f_object)) {
    nout << "Attribute " << attribute_name
         << " does not have a vec2f object value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }

  MFnNumericData data(vec2f_object, &status);
  if (!status) {
    nout << "Attribute " << attribute_name << " is of type "
         << vec2f_object.apiTypeStr() << ", not a NumericData.\n";
    return false;
  }

  status = data.getData(value[0], value[1]);
  if (!status) {
    nout << "Unable to extract 2 floats from " << attribute_name
         << ", of type " << vec2f_object.apiTypeStr() << "\n";
  }

  return true;
}

bool
get_vec2d_attribute(MObject &node, const string &attribute_name,
                    LVecBase2d &value) {
  MStatus status;

  MObject vec2d_object;
  if (!get_maya_attribute(node, attribute_name, vec2d_object)) {
    nout << "Attribute " << attribute_name
         << " does not have a vec2d object value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }

  MFnNumericData data(vec2d_object, &status);
  if (!status) {
    nout << "Attribute " << attribute_name << " is of type "
         << vec2d_object.apiTypeStr() << ", not a NumericData.\n";
    return false;
  }

  status = data.getData(value[0], value[1]);
  if (!status) {
    nout << "Unable to extract 2 doubles from " << attribute_name
         << ", of type " << vec2d_object.apiTypeStr() << "\n";
  }

  return true;
}

bool
get_string_attribute(MObject &node, const string &attribute_name,
                     string &value) {
  MStatus status;

  MObject string_object;
  if (!get_maya_attribute(node, attribute_name, string_object)) {
    nout << "Attribute " << attribute_name
         << " does not have an string object value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }

  MFnStringData data(string_object, &status);
  if (!status) {
    nout << "Attribute " << attribute_name << " is of type "
         << string_object.apiTypeStr() << ", not a StringData.\n";
    return false;
  }

  value = data.string().asChar();
  return true;
}

void
describe_maya_attribute(MObject &node, const string &attribute_name) {
  MStatus status;
  MFnDependencyNode node_fn(node, &status);
  if (!status) {
    nout << "Object is a " << node.apiTypeStr() << ", not a DependencyNode.\n";
    return;
  }

  MObject attr = node_fn.attribute(attribute_name.c_str(), &status);
  if (!status) {
    nout << "Object " << node_fn.name() << " does not support attribute "
         << attribute_name << "\n";
    return;
  }

  nout << "Attribute " << attribute_name << " on object "
       << node_fn.name() << " has type " << attr.apiTypeStr() << "\n";
}
