// Filename: eggCharacterData.I
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::get_num_models
//       Access: Public
//  Description: Returns the total number of models associated with
//               this character.
////////////////////////////////////////////////////////////////////
INLINE int EggCharacterData::
get_num_models() const {
  return _models.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::get_model_index
//       Access: Public
//  Description: Returns the model_index of the nth model associated
//               with this character.  This model_index may be used to
//               ask questions about the particular model from the
//               EggCharacterCollection object, or from the individual
//               EggJointData and EggSliderData objects.
////////////////////////////////////////////////////////////////////
INLINE int EggCharacterData::
get_model_index(int n) const {
  nassertr(n >= 0 && n < (int)_models.size(), 0);
  return _models[n]._model_index;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::get_model_root
//       Access: Public
//  Description: Returns the model_root of the nth model associated
//               with this character.  
//
//               This is the node at which the character, animation
//               bundle, or LOD officially began within its particular
//               egg file.
////////////////////////////////////////////////////////////////////
INLINE EggNode *EggCharacterData::
get_model_root(int n) const {
  nassertr(n >= 0 && n < (int)_models.size(), (EggNode *)NULL);
  return _models[n]._model_root;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::get_root_joint
//       Access: Public
//  Description: Returns the root joint of the character hierarchy.
//               This root joint does not represent an actual joint in
//               the hierarchy, but instead is a fictitious joint that
//               is the parent of all the top joints in the hierarchy
//               (since the hierarchy may actually contain zero or
//               more top joints).
////////////////////////////////////////////////////////////////////
INLINE EggJointData *EggCharacterData::
get_root_joint() const {
  return _root_joint;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::find_joint
//       Access: Public
//  Description: Returns the first joint found with the indicated
//               name, or NULL if no joint has that name.
////////////////////////////////////////////////////////////////////
INLINE EggJointData *EggCharacterData::
find_joint(const string &name) const {
  return _root_joint->find_joint(name);
}
