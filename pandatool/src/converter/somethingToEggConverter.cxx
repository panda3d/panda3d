// Filename: somethingToEggConverter.cxx
// Created by:  drose (26Apr01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#include "somethingToEggConverter.h"

#include <eggData.h>
#include <eggExternalReference.h>

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
SomethingToEggConverter::
SomethingToEggConverter() {
  _allow_errors = false;
  _tpc = PC_absolute;
  _mpc = PC_absolute;
  _animation_convert = AC_none;
  _start_frame = 0.0;
  _end_frame = 0.0;
  _frame_inc = 0.0;
  _input_frame_rate = 0.0;
  _output_frame_rate = 0.0;
  _control_flags = 0;
  _merge_externals = false;
  _egg_data = (EggData *)NULL;
  _owns_egg_data = false;
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
  _tpc(copy._tpc),
  _tpc_directory(copy._tpc_directory),
  _mpc(copy._mpc),
  _mpc_directory(copy._mpc_directory),
  _merge_externals(copy._merge_externals)
{
  _egg_data = (EggData *)NULL;
  _owns_egg_data = false;
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
                          const Filename &orig_filename,
                          const DSearchPath &searchpath) {
  if (_merge_externals) {
    SomethingToEggConverter *ext = make_copy();
    EggData egg_data;
    egg_data.set_coordinate_system(get_egg_data().get_coordinate_system());
    ext->set_egg_data(&egg_data, false);

    // If we're reading references directly, don't mamby around with
    // the path conversion stuff, just look for the file and read it.
    Filename as_found = orig_filename;
    if (!as_found.resolve_filename(searchpath)) {
      // If the filename can't be found, try just the truncated
      // filename.
      Filename truncated = orig_filename.get_basename();
      if (truncated.resolve_filename(searchpath)) {
        as_found = truncated;
      }
    }

    if (!ext->convert_file(as_found)) {
      delete ext;
      nout << "Unable to read external reference: " << orig_filename << "\n";
      if (!_allow_errors) {
        _error = true;
      }
      return false;
    }

    egg_parent->steal_children(egg_data);
    delete ext;
    return true;

  } else {
    // If we're installing external references instead of reading
    // them, we should massage the filename as specified.
    Filename filename = convert_model_path(orig_filename, searchpath);

    filename.set_extension("egg");

    EggExternalReference *egg_ref = new EggExternalReference("", filename);
    egg_parent->add_child(egg_ref);
  }

  return true;
}

////////////////////////////////////////////////////////////////////
//     Function: SomethingToEggConverter::convert_path
//       Access: Public, Static
//  Description: Converts the pathname reference by the source file as
//               requested.  This may either make it absolute,
//               relative, or leave it alone.
//
//               orig_filename is the filename as it actually appeared
//               in the source file; searchpath is the search path to
//               look for it along.  rel_dir is the directory to make
//               the pathname relative to in the case of PC_relative
//               or PC_rel_abs.
////////////////////////////////////////////////////////////////////
Filename SomethingToEggConverter::
convert_path(const Filename &orig_filename, const DSearchPath &searchpath,
             const Filename &rel_dir, PathConvert path_convert) {
  // Try to look up the filename along the search path.
  Filename as_found = orig_filename;
  if (!as_found.resolve_filename(searchpath)) {
    // If the filename can't be found, try just the truncated
    // filename.
    Filename truncated = orig_filename.get_basename();
    if (truncated.resolve_filename(searchpath)) {
      as_found = truncated;
    }
  }

  Filename result;

  switch (path_convert) {
  case PC_relative:
    result = as_found;
    result.make_relative_to(rel_dir);
    return result;

  case PC_absolute:
    result = as_found;
    if (!result.exists()) {
      nout << "Warning: file " << orig_filename << " not found.\n";
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
