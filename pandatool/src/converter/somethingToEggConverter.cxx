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
  _tpc = PC_absolute;
  _mpc = PC_absolute;
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

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::convert_path
//       Access: Protected, Static
//  Description: Converts the pathname reference by the source file as
//               requested.  This may either make it absolute,
//               relative, or leave it alone.
//
//               orig_filename is the filename as it actually appeared
//               in the source file; as_found is the full pathname to the
//               file as it actually exists on disk, assuming it was
//               found on the search path.  rel_dir is the directory
//               to make the pathname relative to in the case of
//               PC_relative or PC_rel_abs.
////////////////////////////////////////////////////////////////////
Filename SomethingToEggConverter::
convert_path(const Filename &orig_filename, const Filename &as_found,
	     const Filename &rel_dir, PathConvert path_convert) {
  Filename result;

  switch (path_convert) {
  case PC_relative:
    result = as_found;
    result.make_relative_to(rel_dir);
    return result;

  case PC_absolute:
    result = as_found;
    if (as_found.is_local()) {
      nout << "Warning: file " << as_found << " not found; cannot make absolute.\n";
      return result;
    }
    result.make_absolute();
    return result;

  case PC_rel_abs:
    result = as_found;
    result.make_relative_to(rel_dir);
    result = Filename(rel_dir, result);
    return result;

  case PC_strip:
    return orig_filename.get_basename();

  case PC_unchanged:
    return orig_filename;
  }

  // Error case.
  nout << "Invalid PathConvert type: " << (int)path_convert << "\n";
  return orig_filename;
}
