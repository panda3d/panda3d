// Filename: switchNodeOne.I
// Created by:  drose (15May01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: SwitchNodeOne::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SwitchNodeOne::
SwitchNodeOne(const string &name) : 
  SwitchNode(name) 
{ 
  _selected_child_index = 0;
}

////////////////////////////////////////////////////////////////////
//     Function: SwitchNodeOne::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE SwitchNodeOne::
SwitchNodeOne(const SwitchNodeOne &copy) :
  SwitchNode(copy),
  _selected_child_index(copy._selected_child_index)
{
}

////////////////////////////////////////////////////////////////////
//     Function: SwitchNodeOne::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE void SwitchNodeOne::
operator = (const SwitchNodeOne &copy) {
  SwitchNode::operator = (copy);
  _selected_child_index = copy._selected_child_index;
}

////////////////////////////////////////////////////////////////////
//     Function: SwitchNodeOne::select_child
//       Access: Public
//  Description: Indicates the particular child which should be
//               visible.
////////////////////////////////////////////////////////////////////
INLINE void SwitchNodeOne::
select_child(int index) {
  if (switchnode_cat.is_debug()) {
    if (_selected_child_index != index) {
      switchnode_cat.debug()
	<< *this << " selecting child " << index << "\n";
    }
  }

  _selected_child_index = index;
}
