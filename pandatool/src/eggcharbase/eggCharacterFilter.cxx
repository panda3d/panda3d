// Filename: eggCharacterFilter.cxx
// Created by:  drose (23Feb01)
// 
////////////////////////////////////////////////////////////////////

#include "eggCharacterFilter.h"
#include "eggCharacterCollection.h"


////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
EggCharacterFilter::
EggCharacterFilter() : EggMultiFilter(false) {
  _collection = (EggCharacterCollection *)NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
EggCharacterFilter::
~EggCharacterFilter() {
  if (_collection != (EggCharacterCollection *)NULL) {
    delete _collection;
  }
}


////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::post_command_line
//       Access: Protected, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
bool EggCharacterFilter::
post_command_line() {
  if (_collection == (EggCharacterCollection *)NULL) {
    _collection = make_collection();
  }

  if (!EggMultiFilter::post_command_line()) {
    return false;
  }
  
  Eggs::iterator ei;
  for (ei = _eggs.begin(); ei != _eggs.end(); ++ei) {
    EggData *data = (*ei);
    
    if (!_collection->add_egg(data)) {
      nout << data->get_egg_filename().get_basename()
	   << " does not contain a character model or animation channel.\n";
      return false;
    }
  }

  _collection->write(cerr);

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: EggCharacterFilter::make_collection
//       Access: Protected, Virtual
//  Description: Allocates and returns a new EggCharacterCollection
//               structure.  This is primarily intended as a hook so
//               derived classes can customize the type of
//               EggCharacterCollection object used to represent the
//               character information.
////////////////////////////////////////////////////////////////////
EggCharacterCollection *EggCharacterFilter::
make_collection() {
  return new EggCharacterCollection;
}
