// Filename: movingPartScalar.I
// Created by:  drose (23Feb99)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: MovingPartScalar::Copy Constructor
//       Access: Protected
//  Description: Normally, you'd use make_copy() or copy_subgraph() to
//               make a copy of this.
////////////////////////////////////////////////////////////////////
INLINE MovingPartScalar::
MovingPartScalar(const MovingPartScalar &copy) :
  MovingPart<ACScalarSwitchType>(copy)
{
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartScalar::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MovingPartScalar::
MovingPartScalar(PartGroup *parent, const string &name,
		 const float &initial_value) 
  : MovingPart<ACScalarSwitchType>(parent, name, initial_value) {
}

////////////////////////////////////////////////////////////////////
//     Function: MovingPartScalar::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
INLINE MovingPartScalar::
MovingPartScalar(void){
}
