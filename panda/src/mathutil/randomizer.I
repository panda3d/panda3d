// Filename: randomizer.I
// Created by:  drose (18Jan07)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: Randomizer::Constructor
//       Access: Public
//  Description: If seed is nonzero, it is used to define the tables;
//               if it is zero a random seed is generated.
////////////////////////////////////////////////////////////////////
Randomizer::
Randomizer(unsigned long seed) :
  _mersenne(seed != 0 ? seed : get_next_seed())
{
}

////////////////////////////////////////////////////////////////////
//     Function: Randomizer::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
Randomizer::
Randomizer(const Randomizer &copy) :
  _mersenne(copy._mersenne)
{
}

////////////////////////////////////////////////////////////////////
//     Function: Randomizer::Copy Assignment Operator
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
void Randomizer::
operator = (const Randomizer &copy) {
  _mersenne = copy._mersenne;
}

////////////////////////////////////////////////////////////////////
//     Function: Randomizer::random_int
//       Access: Public
//  Description: Returns a random integer in the range [0, range).
////////////////////////////////////////////////////////////////////
INLINE int Randomizer::
random_int(int range) {
  return (int)floor(random_real((double)range));
}

////////////////////////////////////////////////////////////////////
//     Function: Randomizer::random_real
//       Access: Public
//  Description: Returns a random double in the range [0, range).
////////////////////////////////////////////////////////////////////
INLINE double Randomizer::
random_real(double range) {
  return (range * _mersenne.get_uint31()) / ((double)0x80000000);
}

////////////////////////////////////////////////////////////////////
//     Function: Randomizer::random_real_unit
//       Access: Public
//  Description: Returns a random double in the range [-0.5, 0.5).
////////////////////////////////////////////////////////////////////
INLINE double Randomizer::
random_real_unit() {
  return random_real(1.0f) - 0.5f;
}

////////////////////////////////////////////////////////////////////
//     Function: Randomizer::get_next_seed
//       Access: Public, Static
//  Description: Returns a random seed value for the next global
//               Randomizer object.
////////////////////////////////////////////////////////////////////
INLINE unsigned long Randomizer::
get_next_seed() {
  if (!_got_first_seed) {
    _next_seed = Mersenne((unsigned long)time(NULL));
    _got_first_seed = true;
  }
  return _next_seed.get_uint31();
}

////////////////////////////////////////////////////////////////////
//     Function: Randomizer::get_seed
//       Access: Public
//  Description: Returns a unique seed value based on the seed value
//               passed to this Randomizer object (and on its current
//               state).
////////////////////////////////////////////////////////////////////
INLINE unsigned long Randomizer::
get_seed() {
  return _mersenne.get_uint31();
}
