// Filename: eggCharacterFilter.cxx
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggCharacterFilter.h"
#include "eggCharacterData.h"


////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggCharacterFilter::
EggCharacterFilter() : EggMultiFilter(false) {
  _character_data = (EggCharacterData *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
EggCharacterFilter::
~EggCharacterFilter() {
  if (_character_data != (EggCharacterData *)NULL) {
    delete _character_data;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::post_command_line
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool EggCharacterFilter::
post_command_line() {
  if (_character_data == (EggCharacterData *)NULL) {
    _character_data = make_character_data();
  }

  if (!EggMultiFilter::post_command_line()) {
    return false;
  }
  
  Eggs::iterator ei;
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    EggData *data = (*ei);
    
    if (!_character_data->add_egg(data)) {
      nout << data->get_egg_filename().get_basename()
	   << " does not contain a character model or animation channel.\n";
      return false;
    }
  }

  _character_data->write(cerr);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::make_character_data
//       Access: Protected, Virtual
//  Description: Allocates and returns a new EggCharacterData structure.
//               This is primarily intended as a hook so derived
//               classes can customize the type of EggCharacterData nodes
//               used to represent the character information.
////////////////////////////////////////////////////////////////////
EggCharacterData *EggCharacterFilter::
make_character_data() {
  return new EggCharacterData;
}
