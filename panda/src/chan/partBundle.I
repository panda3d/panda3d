// Filename: partBundle.I
// Created by:  drose (22Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_blend_type
//       Access: Public
//  Description: Returns the way the character responds to multiple
//               animations being bound simultaneously.
////////////////////////////////////////////////////////////////////
INLINE PartBundle::BlendType PartBundle::
get_blend_type() const {
  return _blend_type;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_node
//       Access: Public
//  Description: Returns the PartBundleNode associated with this
//               PartBundle.
////////////////////////////////////////////////////////////////////
INLINE PartBundleNode *PartBundle::
get_node() const {
  return _node;
}

////////////////////////////////////////////////////////////////////
//     Function: PartBundle::control_begin
//       Access: Public
//  Description: Returns an iterator that can be used to traverse the
//               entire set of AnimControls currently in effect.
////////////////////////////////////////////////////////////////////
INLINE PartBundle::control_iterator PartBundle::
control_begin() const {
  return _blend.begin();
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::control_end
//       Access: Public
//  Description: Returns an iterator that can be used to traverse the
//               entire set of AnimControls currently in effect.
////////////////////////////////////////////////////////////////////
INLINE PartBundle::control_iterator PartBundle::
control_end() const {
  return _blend.end();
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::control_size
//       Access: Public
//  Description: Returns the number of entries between control_begin()
//               and control_end(): the number of AnimControls
//               currently in effect.
////////////////////////////////////////////////////////////////////
INLINE PartBundle::control_size_type PartBundle::
control_size() const {
  return _blend.size();
}


////////////////////////////////////////////////////////////////////
//     Function: PartBundle::get_blend_map
//       Access: Public
//  Description: Returns the entire mapping of AnimControls to blend
//               effect.
////////////////////////////////////////////////////////////////////
INLINE const PartBundle::ChannelBlend &PartBundle::
get_blend_map() const {
  return _blend;
}
