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
#include <maya/MPlugArray.h>
#include <maya/MPlug.h>
#include <maya/MFnAttribute.h>
#include <maya/MFnTypedAttribute.h>
#include <maya/MFnEnumAttribute.h>
#include <maya/MFnMatrixData.h>
#include <maya/MMatrix.h>
#include "post_maya_include.h"

////////////////////////////////////////////////////////////////////
//     Function: get_maya_plug
//  Description: Gets the named MPlug associated, if any.
////////////////////////////////////////////////////////////////////
bool
get_maya_plug(MObject &node, const string &attribute_name, MPlug &plug) {
  MStatus status;
  MFnDependencyNode node_fn(node, &status);
  if (!status) {
    maya_cat.error()
      << "Object is a " << node.apiTypeStr() << ", not a DependencyNode.\n";
    return false;
  }

  MObject attr = node_fn.attribute(attribute_name.c_str(), &status);
  if (!status) {
    maya_cat.debug()
      << "Object " << node_fn.name() << " does not support attribute "
      << attribute_name << "\n";
    return false;
  }

  MFnAttribute attr_fn(attr, &status);
  if (!status) {
    maya_cat.error()
      << "Attribute " << attribute_name << " on " << node_fn.name()
      << " is a " << attr.apiTypeStr() << ", not an Attribute.\n";
    return false;
  }

  plug = MPlug(node, attr);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: has_attribute
//  Description: Returns true if the node has the indicated attribute,
//               false otherwise.
////////////////////////////////////////////////////////////////////
bool
has_attribute(MObject &node, const string &attribute_name) { 
  MStatus status;
  MFnDependencyNode node_fn(node, &status);
  if (!status) {
    maya_cat.error()
      << "Object is a " << node.apiTypeStr() << ", not a DependencyNode.\n";
    return false;
  }

  node_fn.attribute(attribute_name.c_str(), &status);
  if (!status) {
    // No such attribute.
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: get_bool_attribute
//  Description: Extracts the named boolean attribute from the
//               MObject.
////////////////////////////////////////////////////////////////////
bool
get_bool_attribute(MObject &node, const string &attribute_name,
                   bool &value) {
  if (!has_attribute(node, attribute_name)) {
    // For bool attributes only, we assume if the attribute is absent
    // it's the same thing as being false.
    return false;
  }

  if (!get_maya_attribute(node, attribute_name, value)) {
    maya_cat.error()
      << "Attribute " << attribute_name
      << " does not have a bool value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: get_bool_attribute
//  Description: Extracts the named angle in degrees from the
//               MObject.
////////////////////////////////////////////////////////////////////
bool
get_angle_attribute(MObject &node, const string &attribute_name,
                    double &value) {
  MAngle maya_value;
  if (!get_maya_attribute(node, attribute_name, maya_value)) {
    maya_cat.error()
      << "Attribute " << attribute_name
      << " does not have an angle value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }
  value = maya_value.asDegrees();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: get_vec2f_attribute
//  Description: Extracts the named two-component vector from the
//               MObject.
////////////////////////////////////////////////////////////////////
bool
get_vec2f_attribute(MObject &node, const string &attribute_name,
                    LVecBase2f &value) {
  MStatus status;

  MObject vec2f_object;
  if (!get_maya_attribute(node, attribute_name, vec2f_object)) {
    maya_cat.error()
      << "Attribute " << attribute_name
      << " does not have a vec2f object value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }

  MFnNumericData data(vec2f_object, &status);
  if (!status) {
    maya_cat.error()
      << "Attribute " << attribute_name << " is of type "
      << vec2f_object.apiTypeStr() << ", not a NumericData.\n";
    return false;
  }

  status = data.getData(value[0], value[1]);
  if (!status) {
    maya_cat.error()
      << "Unable to extract 2 floats from " << attribute_name
      << ", of type " << vec2f_object.apiTypeStr() << "\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: get_vec3f_attribute
//  Description: Extracts the named three-component vector from the
//               MObject.
////////////////////////////////////////////////////////////////////
bool
get_vec3f_attribute(MObject &node, const string &attribute_name,
                    LVecBase3f &value) {
  MStatus status;

  MObject vec3f_object;
  if (!get_maya_attribute(node, attribute_name, vec3f_object)) {
    maya_cat.error()
      << "Attribute " << attribute_name
      << " does not have a vec3f object value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }

  MFnNumericData data(vec3f_object, &status);
  if (!status) {
    maya_cat.error()
      << "Attribute " << attribute_name << " is of type "
      << vec3f_object.apiTypeStr() << ", not a NumericData.\n";
    return false;
  }

  status = data.getData(value[0], value[1], value[2]);
  if (!status) {
    maya_cat.error()
      << "Unable to extract 3 floats from " << attribute_name
      << ", of type " << vec3f_object.apiTypeStr() << "\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: get_vec2d_attribute
//  Description: Extracts the named two-component vector from the
//               MObject.
////////////////////////////////////////////////////////////////////
bool
get_vec2d_attribute(MObject &node, const string &attribute_name,
                    LVecBase2d &value) {
  MStatus status;

  MObject vec2d_object;
  if (!get_maya_attribute(node, attribute_name, vec2d_object)) {
    maya_cat.error()
      << "Attribute " << attribute_name
      << " does not have a vec2d object value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }

  MFnNumericData data(vec2d_object, &status);
  if (!status) {
    maya_cat.error()
      << "Attribute " << attribute_name << " is of type "
      << vec2d_object.apiTypeStr() << ", not a NumericData.\n";
    return false;
  }

  status = data.getData(value[0], value[1]);
  if (!status) {
    maya_cat.error()
      << "Unable to extract 2 doubles from " << attribute_name
      << ", of type " << vec2d_object.apiTypeStr() << "\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: get_vec3d_attribute
//  Description: Extracts the named three-component vector from the
//               MObject.
////////////////////////////////////////////////////////////////////
bool
get_vec3d_attribute(MObject &node, const string &attribute_name,
                    LVecBase3d &value) {
  MStatus status;

  MObject vec3d_object;
  if (!get_maya_attribute(node, attribute_name, vec3d_object)) {
    maya_cat.error()
      << "Attribute " << attribute_name
      << " does not have a vec3d object value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }

  MFnNumericData data(vec3d_object, &status);
  if (!status) {
    maya_cat.error()
      << "Attribute " << attribute_name << " is of type "
      << vec3d_object.apiTypeStr() << ", not a NumericData.\n";
    return false;
  }

  status = data.getData(value[0], value[1], value[2]);
  if (!status) {
    maya_cat.error()
      << "Unable to extract 3 doubles from " << attribute_name
      << ", of type " << vec3d_object.apiTypeStr() << "\n";
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: get_mat4d_attribute
//  Description: Extracts the named 4x4 matrix from the MObject.
////////////////////////////////////////////////////////////////////
bool
get_mat4d_attribute(MObject &node, const string &attribute_name,
                    LMatrix4d &value) {
  MStatus status;
  MObject matrix;
  if (!get_maya_attribute(node, attribute_name, matrix)) {
    return false;
  }

  MFnMatrixData matrix_data(matrix, &status);
  if (!status) {
    maya_cat.error()
      << "Attribute " << attribute_name << " is of type "
      << node.apiTypeStr() << ", not a Matrix.\n";
    return false;
  }
    
  const MMatrix &mat = matrix_data.matrix();
  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 4; j++) {
      value(i, j) = mat(i, j);
    }
  }
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: get_enum_attribute
//  Description: Extracts the enum attribute from the MObject as a
//               string value.
////////////////////////////////////////////////////////////////////
bool
get_enum_attribute(MObject &node, const string &attribute_name,
                   string &value) {
  MStatus status;

  MPlug plug;
  if (!get_maya_plug(node, attribute_name.c_str(), plug)) {
    return false;
  }

  MObject attrib = plug.attribute();
  MFnEnumAttribute enum_attrib(attrib, &status);
  if (!status) {
    maya_cat.error()
      << "Not an enum attribute: " << attribute_name << "\n";
    return false;
  }

  short index;
  status = plug.getValue(index);
  if (!status) {
    maya_cat.error()
      << "Could not get numeric value of " << attribute_name << "\n";
    status.perror("MPlug::getValue(short)");
    return false;
  }

  MString name = enum_attrib.fieldName(index, &status);
  if (!status) {
    maya_cat.error()
      << "Invalid value for " << attribute_name << ": " << index << "\n";
    status.perror("MFnEnumAttribute::fieldName()");
    return false;
  }

  value = name.asChar();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: get_string_attribute
//  Description: Extracts the named string attribute from the
//               MObject.
////////////////////////////////////////////////////////////////////
bool
get_string_attribute(MObject &node, const string &attribute_name,
                     string &value) {
  MStatus status;

  MObject string_object;
  if (!get_maya_attribute(node, attribute_name, string_object)) {
    maya_cat.error()
      << "Attribute " << attribute_name
      << " does not have an string object value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }

  MFnStringData data(string_object, &status);
  if (!status) {
    maya_cat.error()
      << "Attribute " << attribute_name << " is of type "
      << string_object.apiTypeStr() << ", not a StringData.\n";
    return false;
  }

  value = data.string().asChar();
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: set_string_attribute
//  Description: Sets the named string attribute on the
//               MObject.
////////////////////////////////////////////////////////////////////
bool
set_string_attribute(MObject &node, const string &attribute_name,
                     const string &value) {
  MStatus status;

  // First, we get the string_object, then we set its string.
  MObject string_object;
  if (!get_maya_attribute(node, attribute_name, string_object)) {
    maya_cat.error()
      << "Attribute " << attribute_name
      << " does not have a string object value.\n";
    describe_maya_attribute(node, attribute_name);
    return false;
  }

  MFnStringData data(string_object, &status);
  if (!status) {
    maya_cat.error()
      << "Attribute " << attribute_name << " is of type "
      << string_object.apiTypeStr() << ", not a StringData.\n";
    return false;
  }

  MString mstring_value(value.data(), value.length());
  status = data.set(mstring_value);
  if (!status) {
    status.perror(attribute_name.c_str());
    return false;
  }

  // And it appears we now need to set the string object back.
  if (!set_maya_attribute(node, attribute_name, string_object)) {
    maya_cat.error()
      << "Attribute " << attribute_name
      << " suddenly does not have a string object value.\n";
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: describe_maya_attribute
//  Description: Writes some error output about the indicated Maya
//               attribute.
////////////////////////////////////////////////////////////////////
void
describe_maya_attribute(MObject &node, const string &attribute_name) {
  MStatus status;
  MFnDependencyNode node_fn(node, &status);
  if (!status) {
    maya_cat.error()
      << "Object is a " << node.apiTypeStr() << ", not a DependencyNode.\n";
    return;
  }

  MObject attr = node_fn.attribute(attribute_name.c_str(), &status);
  if (!status) {
    maya_cat.error()
      << "Object " << node_fn.name() << " does not support attribute "
      << attribute_name << "\n";
    return;
  }

  maya_cat.error()
    << "Attribute " << attribute_name << " on object "
    << node_fn.name() << " has type " << attr.apiTypeStr() << "\n";
}

string
string_mfndata_type(MFnData::Type type) {
  switch (type) {
  case MFnData::kInvalid:
    return "kInvalid";

  case MFnData::kNumeric:
    return "kNumeric";

  case MFnData::kPlugin:
    return "kPlugin";

  case MFnData::kPluginGeometry:
    return "kPluginGeometry";

  case MFnData::kString:
    return "kString";

  case MFnData::kMatrix:
    return "kMatrix";

  case MFnData::kStringArray:
    return "kStringArray";

  case MFnData::kDoubleArray:
    return "kDoubleArray";

  case MFnData::kIntArray:
    return "kIntArray";

  case MFnData::kPointArray:
    return "kPointArray";

  case MFnData::kVectorArray:
    return "kVectorArray";

  case MFnData::kComponentList:
    return "kComponentList";

  case MFnData::kMesh:
    return "kMesh";

  case MFnData::kLattice:
    return "kLattice";

  case MFnData::kNurbsCurve:
    return "kNurbsCurve";

  case MFnData::kNurbsSurface:
    return "kNurbsSurface";

  case MFnData::kSphere:
    return "kSphere";

  case MFnData::kDynArrayAttrs:
    return "kDynArrayAttrs";

  case MFnData::kDynSweptGeometry:
    return "kDynSweptGeometry";

  case MFnData::kSubdSurface:
    return "kSubdSurface";

  case MFnData::kLast:
    return "kLast";
  }

  return "**invalid**";
}

////////////////////////////////////////////////////////////////////
//     Function: list_maya_attributes
//  Description: Writes some info output showing all the attributes on
//               the given dependency node.  Primarily useful during
//               development, to figure out where the heck Maya hides
//               some of the connected properties.
////////////////////////////////////////////////////////////////////
void
list_maya_attributes(MObject &node) {
  MStatus status;
  MFnDependencyNode node_fn(node, &status);
  if (!status) {
    maya_cat.error()
      << "Object is a " << node.apiTypeStr() << ", not a DependencyNode.\n";
    return;
  }

  string name = node_fn.name().asChar();
  unsigned i;

  MPlugArray connections;
  status = node_fn.getConnections(connections);
  if (!status) {
    status.perror("MFnDependencyNode::getConnections");

  } else {
    maya_cat.info()
      << name << " has " << connections.length() << " connections.\n";
    for (i = 0; i < connections.length(); i++) {
      MPlug plug = connections[i];
      maya_cat.info(false)
        << "  " << i << ". " << plug.name().asChar() << ", "
        << plug.attribute().apiTypeStr() << ", " 
        << plug.node().apiTypeStr() << "\n";
    }
  }
    
  maya_cat.info()
    << name << " has " << node_fn.attributeCount() << " attributes.\n";
  for (i = 0; i < node_fn.attributeCount(); i++) {
    MObject attr = node_fn.attribute(i, &status);
    if (status) {
      MFnTypedAttribute typed_attrib(attr, &status);
      if (status) {
        // It's a typed attrib.
          maya_cat.info(false) 
            << "  " << i << ". " << typed_attrib.name().asChar()
            << " [" << attr.apiTypeStr() << ", "
            << string_mfndata_type(typed_attrib.attrType()) << "]\n";
      } else {
        MFnAttribute attrib(attr, &status);
        if (status) {
          // It's a generic attrib.
          maya_cat.info(false) 
            << "  " << i << ". " << attrib.name().asChar()
            << " [" << attr.apiTypeStr() << "]\n";
        } else {
          // Don't know what it is.
          maya_cat.info(false)
            << "  " << i << ". [" << attr.apiTypeStr() << "]\n";
        }
      }
    }
  }
}

