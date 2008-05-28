// Filename: loaderFileTypePandatool.cxx
// Created by:  drose (26Apr01)
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

#include "loaderFileTypePandatool.h"
#include "config_ptloader.h"
#include "somethingToEggConverter.h"
#include "config_util.h"
#include "load_egg_file.h"
#include "eggData.h"
#include "loaderOptions.h"

TypeHandle LoaderFileTypePandatool::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypePandatool::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileTypePandatool::
LoaderFileTypePandatool(SomethingToEggConverter *converter) :
  _converter(converter)
{
  converter->set_merge_externals(true);
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypePandatool::Destructor
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
LoaderFileTypePandatool::
~LoaderFileTypePandatool() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypePandatool::get_name
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypePandatool::
get_name() const {
  return _converter->get_name();
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypePandatool::get_extension
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
string LoaderFileTypePandatool::
get_extension() const {
  return _converter->get_extension();
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypePandatool::get_additional_extensions
//       Access: Public, Virtual
//  Description: Returns a space-separated list of extension, in
//               addition to the one returned by get_extension(), that
//               are recognized by this converter.
////////////////////////////////////////////////////////////////////
string LoaderFileTypePandatool::
get_additional_extensions() const {
  return _converter->get_additional_extensions();
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypePandatool::supports_compressed
//       Access: Published, Virtual
//  Description: Returns true if this file type can transparently load
//               compressed files (with a .pz extension), false
//               otherwise.
////////////////////////////////////////////////////////////////////
bool LoaderFileTypePandatool::
supports_compressed() const {
  return _converter->supports_compressed();
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypePandatool::resolve_filename
//       Access: Public, Virtual
//  Description: Searches for the indicated filename on whatever paths
//               are appropriate to this file type, and updates it if
//               it is found.
////////////////////////////////////////////////////////////////////
void LoaderFileTypePandatool::
resolve_filename(Filename &path) const {
  path.resolve_filename(get_model_path(), get_extension());
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypePandatool::load_file
//       Access: Public, Virtual
//  Description:
////////////////////////////////////////////////////////////////////
PT(PandaNode) LoaderFileTypePandatool::
load_file(const Filename &path, const LoaderOptions &options,
          BamCacheRecord *record) const {
  PT(PandaNode) result;

  PT(EggData) egg_data = new EggData;
  _converter->set_egg_data(egg_data);

  DSearchPath file_path;
  file_path.append_directory(path.get_dirname());
  _converter->get_path_replace()->_path = file_path;

  // Convert animation, if the converter supports it.
  switch (options.get_flags() & LoaderOptions::LF_convert_anim) {
  case LoaderOptions::LF_convert_anim:
    _converter->set_animation_convert(AC_both);
    break;
    
  case LoaderOptions::LF_convert_skeleton:
    _converter->set_animation_convert(AC_model);
    break;
    
  case LoaderOptions::LF_convert_channels:
    _converter->set_animation_convert(AC_chan);
    break;

  default:
    break;
  }

  if (_converter->convert_file(path)) {
    DistanceUnit input_units = _converter->get_input_units();
    if (input_units != DU_invalid && ptloader_units != DU_invalid && 
        input_units != ptloader_units) {
      // Convert the file to the units specified by the ptloader-units
      // Configrc variable.
      ptloader_cat.info()
        << "Converting from " << format_long_unit(input_units)
        << " to " << format_long_unit(ptloader_units) << "\n";
      double scale = convert_units(input_units, ptloader_units);
      egg_data->transform(LMatrix4d::scale_mat(scale));
    }

    if (!egg_data->has_normals()) {
      egg_data->recompute_polygon_normals();
    }

    result = load_egg_data(egg_data);
  }
  _converter->clear_egg_data();
  return result.p();
}
