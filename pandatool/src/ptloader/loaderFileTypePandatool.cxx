// Filename: loaderFileTypePandatool.cxx
// Created by:  drose (26Apr01)
// 
////////////////////////////////////////////////////////////////////

#include "loaderFileTypePandatool.h"

#include <somethingToEggConverter.h>
#include <config_util.h>
#include <load_egg_file.h>
#include <eggData.h>

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
PT_Node LoaderFileTypePandatool::
load_file(const Filename &path, bool) const {
  PT_NamedNode result;

  EggData egg_data;
  _converter->set_egg_data(&egg_data, false);
  if (_converter->convert_file(path)) {
    egg_data.set_coordinate_system(CS_default);
    result = load_egg_data(egg_data);
  }
  _converter->clear_egg_data();
  return result.p();
}
