// Filename: trueClock.I
// Created by:  drose (04Jul00)
// 
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: TrueClock::get_ptr
//       Access: Public
//  Description: Returns a pointer to the one TrueClock object in
//               the world.
////////////////////////////////////////////////////////////////////
INLINE TrueClock *TrueClock::
get_ptr() {
  if (_global_ptr == (TrueClock *)NULL) {
    _global_ptr = new TrueClock;
  }
  return _global_ptr;
}

////////////////////////////////////////////////////////////////////
//     Function: TrueClock::Destructor
//       Access: Protected
//  Description: A protected destructor because no one should try to
//               delete the global TrueClock.
////////////////////////////////////////////////////////////////////
INLINE TrueClock::
~TrueClock() {
}
