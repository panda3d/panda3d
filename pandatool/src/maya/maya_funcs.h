// Filename: maya_funcs.h
// Created by:  drose (16Feb00)
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
