// Filename: trackerTransform.h
// Created by:  jason (08Aug00)
// 
////////////////////////////////////////////////////////////////////

#ifndef TRACKER_TRANSFORM_H
#define TRACKER_TRANSFORM_H

#include <pandabase.h>

#include <dataNode.h>

#include <doubleDataAttribute.h>
#include <doubleDataTransition.h>
#include <vec3DataAttribute.h>
#include <vec3DataTransition.h>
#include <vec3DataAttribute.h>
#include <vec4DataTransition.h>
#include <vec4DataAttribute.h>
#include <matrixDataTransition.h>
#include <matrixDataAttribute.h>
#include <nodeAttributes.h>

#include <luse.h>
#include <lmatrix.h>


////////////////////////////////////////////////////////////////////
//       Class : TrackerTransform
// Description : TrackerTransform reads the data send down the line
//               by a TrackerNode and creates a transformation matrix
//               from the data and sends that matrix down the line
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA TrackerTransform : public DataNode {
public:
  TrackerTransform(const string &name = "");

  virtual void transmit_data(NodeAttributes &data);

private:
  NodeAttributes _transform_attrib;
  PT(MatrixDataAttribute) _transform;

  // inputs
  static TypeHandle _position_type;
  //NOTE!!!
  //Currently not being factored into the matrix.  Needs to be done
  static TypeHandle _pquat_type;

  // outputs
  static TypeHandle _transform_type;

public:
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type();

private:
  static TypeHandle _type_handle;
};

#endif
