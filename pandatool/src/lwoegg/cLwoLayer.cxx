// Filename: cLwoLayer.cxx
// Created by:  drose (25Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "cLwoLayer.h"
#include "lwoToEggConverter.h"


////////////////////////////////////////////////////////////////////
//     Function: CLwoLayer::make_egg
//       Access: Public
//  Description: Creates the egg structures associated with this
//               Lightwave object.
////////////////////////////////////////////////////////////////////
void CLwoLayer::
make_egg() {
  _egg_group = new EggGroup(_layer->_name);

  if (_layer->_pivot != LPoint3f::zero()) {
    // If we have a nonzero pivot point, that's a translation
    // transform.
    LPoint3d translate = LCAST(double, _layer->_pivot);
    _egg_group->set_transform(LMatrix4d::translate_mat(translate));
    _egg_group->set_group_type(EggGroup::GT_instance);
  }
}

////////////////////////////////////////////////////////////////////
//     Function: CLwoLayer::connect_egg
//       Access: Public
//  Description: Connects all the egg structures together.
////////////////////////////////////////////////////////////////////
void CLwoLayer::
connect_egg() {
  if (_layer->_parent != -1) {
    const CLwoLayer *parent = _converter->get_layer(_layer->_parent);
    if (parent != (CLwoLayer *)NULL) {
      parent->_egg_group->add_child(_egg_group.p());
      return;
    }

    nout << "No layer found with number " << _layer->_parent 
	 << "; cannot parent layer " << _layer->_number << " properly.\n";
  }
  
  _converter->get_egg_root()->add_child(_egg_group.p());
}

