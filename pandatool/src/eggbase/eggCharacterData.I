// Filename: eggCharacterData.I
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::get_num_eggs
//       Access: Public
//  Description: Returns the number of egg files that have
//               successfully been added to the Character table.
////////////////////////////////////////////////////////////////////
INLINE int EggCharacterData::
get_num_eggs() const {
  return _eggs.size();
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterData::get_egg
//       Access: Public
//  Description: Returns the ith egg file.
////////////////////////////////////////////////////////////////////
INLINE EggData *EggCharacterData::
get_egg(int i) const {
  nassertr(i >= 0 && i < (int)_eggs.size(), (EggData *)NULL);
  return _eggs[i]._egg;
}
