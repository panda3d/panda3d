// Filename: somethingToEggConverter.cxx
// Created by:  drose (26Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#include "somethingToEggConverter.h"

#include "eggData.h"
#include "eggExternalReference.h"

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SomethingToEggConverter::
SomethingToEggConverter() {
  _allow_errors = false;
  _path_replace = new PathReplace;
  _path_replace->_path_store = PS_absolute;
  _animation_convert = AC_none;
  _start_frame = 0.0;
  _end_frame = 0.0;
  _frame_inc = 0.0;
  _neutral_frame = 0.0;
  _input_frame_rate = 0.0;
  _output_frame_rate = 0.0;
  _control_flags = 0;
  _merge_externals = false;
  _egg_data = (EggData *)NULL;
  _error = false;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SomethingToEggConverter::
SomethingToEggConverter(const SomethingToEggConverter &copy) :
  _allow_errors(copy._allow_errors),
  _path_replace(copy._path_replace),
  _merge_externals(copy._merge_externals)
{
  _egg_data = (EggData *)NULL;
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
//               convert_file().
////////////////////////////////////////////////////////////////////
void SomethingToEggConverter::
set_egg_data(EggData *egg_data) {
  _egg_data = egg_data;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::get_additional_extensions
//       Access: Public, Virtual
//  Description: Returns a space-separated list of extension, in
//               addition to the one returned by get_extension(), that
//               are recognized by this converter.
////////////////////////////////////////////////////////////////////
string SomethingToEggConverter::
get_additional_extensions() const {
  return string();
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::get_input_units
//       Access: Public, Virtual
//  Description: This may be called after convert_file() has been
//               called and returned true, indicating a successful
//               conversion.  It will return the distance units
//               represented by the converted egg file, if known, or
//               DU_invalid if not known.
////////////////////////////////////////////////////////////////////
DistanceUnit SomethingToEggConverter::
get_input_units() {
  return DU_invalid;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::handle_external_reference
//       Access: Public
//  Description: Handles an external reference in the source file.  If
//               the merge_externals flag is true (see
//               set_merge_externals()), this causes the named file to
//               be read in and converted, and the converted egg
//               geometry is parented to egg_parent.  Otherwise, only
//               a reference to a similarly named egg file is parented
//               to egg_parent.
//
//               The parameters orig_filename and searchpath are as
//               those passed to convert_model_path().
//
//               Returns true on success, false on failure.
////////////////////////////////////////////////////////////////////
bool SomethingToEggConverter::
handle_external_reference(EggGroupNode *egg_parent,
                          const Filename &ref_filename) {
  if (_merge_externals) {
    SomethingToEggConverter *ext = make_copy();
    PT(EggData) egg_data = new EggData;
    egg_data->set_coordinate_system(get_egg_data()->get_coordinate_system());
    ext->set_egg_data(egg_data);

    if (!ext->convert_file(ref_filename)) {
      delete ext;
      nout << "Unable to read external reference: " << ref_filename << "\n";
      _error = true;
      return false;
    }

    egg_parent->steal_children(*egg_data);
    delete ext;
    return true;

  } else {
    // If we're installing external references instead of reading
    // them, we should make it into an egg filename.
    Filename filename = ref_filename;
    filename.set_extension("egg");

    EggExternalReference *egg_ref = new EggExternalReference("", filename);
    egg_parent->add_child(egg_ref);
  }

  return true;
}
