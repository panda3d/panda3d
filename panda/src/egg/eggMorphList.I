// Filename: eggMorphList.I
// Created by:  drose (29Jan99)
// 
////////////////////////////////////////////////////////////////////

#include <indent.h>


////////////////////////////////////////////////////////////////////
//     Function: EggMorphList::write
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
template<class MorphType>
void EggMorphList<MorphType>::
write(ostream &out, int indent_level) const {
  const_iterator i;

  for (i = begin(); i != end(); ++i) {
    indent(out, indent_level) << *i << "\n";
  }
}

