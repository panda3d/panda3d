// Filename: trackerTransform.cxx
// Created by:  jason (08Aug00)
// 
////////////////////////////////////////////////////////////////////

#include <pandabase.h>

#include "trackerTransform.h"
#include "config_tform.h"

TypeHandle TrackerTransform::_type_handle;

TypeHandle TrackerTransform::_position_type;
TypeHandle TrackerTransform::_pquat_type;

TypeHandle TrackerTransform::_transform_type;

////////////////////////////////////////////////////////////////////
//     Function: TrackerTransform::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
TrackerTransform::
TrackerTransform(const string &name) :  DataNode(name) {
  _transform = new MatrixDataAttribute;
  _transform->set_value(LMatrix4f::ident_mat());
  _transform_attrib.set_attribute(_transform_type, _transform);
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerTransform::transmit_data
//       Access: Public
//  Description: Constructs a transformation matrix from tracker data
//               and passes it down the line
////////////////////////////////////////////////////////////////////
void TrackerTransform::
transmit_data(NodeAttributes &data) {
  const NodeAttribute *position = data.get_attribute(_position_type);
  
  if (tform_cat.is_debug()) {
    tform_cat.debug() << "TrackerTransform:transmit_data" << endl;
  }
  if (position != (NodeAttribute *)NULL) {
    LVecBase3f p = DCAST(Vec3DataAttribute, position)->get_value();
    
    LMatrix4f mat = LMatrix4f::translate_mat(p);
    if (tform_cat.is_debug()) {
      tform_cat.debug() << "Sending down " << mat << endl;
    }
    _transform->set_value(mat);
  }

  data = _transform_attrib;
}

////////////////////////////////////////////////////////////////////
//     Function: TrackerTransform::init_type
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
void TrackerTransform::
init_type() {
  DataNode::init_type();
  register_type(_type_handle, "TrackerTransform",
		DataNode::get_class_type());

  Vec3DataTransition::init_type();
  Vec4DataTransition::init_type();
  MatrixDataTransition::init_type();

  register_data_transition(_position_type, "Position",
			   Vec3DataTransition::get_class_type());
  register_data_transition(_pquat_type, "Position Quat",
			   Vec4DataTransition::get_class_type());

  register_data_transition(_transform_type, "Transform",
			   MatrixDataTransition::get_class_type());
}




