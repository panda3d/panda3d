// Filename: eggCharacterData.I
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////


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
