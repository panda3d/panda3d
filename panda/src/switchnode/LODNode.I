// Filename: LODNode.I
// Created by:  mike (27Sep99)
// 
////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////
// Includes
////////////////////////////////////////////////////////////////////

#include <LOD.h>
#include <notify.h>

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LODNode::
LODNode(const string &name) : NamedNode(name) {
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE LODNode::
LODNode(const LODNode &copy) : 
  NamedNode(copy),
  _lod(copy._lod)
{
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::add_switch
//       Access: Public
//  Description: Adds a switch range to the LODNode.  This implies
//               that an corresponding child node is also parent to
//               the node.
//
//               The sense of in vs. out distances is as if the object
//               were coming towards you from far away: it switches
//               "in" at the far distance, and switches "out" at the
//               close distance.  Thus, "in" should be larger than
//               "out".
////////////////////////////////////////////////////////////////////
INLINE void LODNode::
add_switch(float in, float out) {
  _lod._switch_vector.push_back(LODSwitch(in, out));
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::set_switch
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE bool LODNode::
set_switch(int index, float in, float out) {
  nassertr(index >= 0 && index < _lod._switch_vector.size(), false);
  _lod._switch_vector[index].set_range(in, out);
  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: LODNode::clear_switches
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE void LODNode::
clear_switches(void) {
  _lod._switch_vector.erase(_lod._switch_vector.begin(),
		    	    _lod._switch_vector.end());
}
