// Filename: maya_funcs.I
// Created by:  drose (16Feb00)
// 
////////////////////////////////////////////////////////////////////

#include <maya/MFnAttribute.h>
#include <maya/MPlug.h>
#include <maya/MStatus.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MObject.h>

template<class ValueType>
bool
get_maya_attribute(MObject &node, const string &attribute_name,
		   ValueType &value) {
  MStatus status;
  MFnDependencyNode node_fn(node, &status);
  if (!status) {
    nout << "Object is a " << node.apiTypeStr() << ", not a DependencyNode.\n";
    return false;
  }

  MObject attr = node_fn.attribute(attribute_name.c_str(), &status);
  if (!status) {
    nout << "Object " << node_fn.name() << " does not support attribute "
	 << attribute_name << "\n";
    return false;
  }

  MFnAttribute attr_fn(attr, &status);
  if (!status) {
    nout << "Attribute " << attribute_name << " on " << node_fn.name()
	 << " is a " << attr.apiTypeStr() << ", not an Attribute.\n";
    return false;
  }
  
  MPlug plug(node, attr);
  status = plug.getValue(value);

  return status;
}
