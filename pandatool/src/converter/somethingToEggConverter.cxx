// Filename: somethingToEggConverter.cxx
// Created by:  drose (26Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "somethingToEggConverter.h"

#include <eggData.h>

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
SomethingToEggConverter::
SomethingToEggConverter() {
  _egg_data = (EggData *)NULL;
  _owns_egg_data = false;
  _tpc = PC_unchanged;
  _mpc = PC_unchanged;
  _error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
SomethingToEggConverter::
~SomethingToEggConverter() {
  clear_egg_data();
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::set_egg_data
//       Access: Public
//  Description: Sets the egg data that will be filled in when
//               convert_file() is called.  This must be called before
//               convert_file().  If owns_egg_data is true, the
//               egg_data will be deleted when the converter
//               destructs.  (We don't use the reference counting on
//               EggData, to allow static EggDatas to be passed in.)
////////////////////////////////////////////////////////////////////
void SomethingToEggConverter::
set_egg_data(EggData *egg_data, bool owns_egg_data) {
  if (_owns_egg_data) {
    delete _egg_data;
  }
  _egg_data = egg_data;
  _owns_egg_data = owns_egg_data;
}
