// Filename: movingPartMatrix.I
// Created by:  drose (23Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::Copy Constructor
//       Access: Protected
//  Description: Normally, you'd use make_copy() or copy_subgraph() to
//               make a copy of this.
////////////////////////////////////////////////////////////////////
INLINE MovingPartMatrix::
MovingPartMatrix(const MovingPartMatrix &copy) :
  MovingPart<ACMatrixSwitchType>(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MovingPartMatrix::
MovingPartMatrix(PartGroup *parent, const string &name,
		 const LMatrix4f &initial_value) 
  : MovingPart<ACMatrixSwitchType>(parent, name, initial_value) {
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartMatrix::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MovingPartMatrix::
MovingPartMatrix(void) {
}
