// Filename: loaderFileTypeEgg.cxx
// Created by:  drose (20Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "loaderFileTypeEgg.h"
#include "load_egg_file.h"

#include <eggData.h>

TypeHandle LoaderFileTypeEgg::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
LoaderFileTypeEgg::
LoaderFileTypeEgg() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::get_name
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
string LoaderFileTypeEgg::
get_name() const {
  return "Egg";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::get_extension
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
string LoaderFileTypeEgg::
get_extension() const {
  return "egg";
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::resolve_filename
//       Access: Public, Virtual
//  Description: Searches for the indicated filename on whatever paths
//               are appropriate to this file type, and updates it if
//               it is found.  It is not necessary to call this before
//               calling load_file(), but it doesn't hurt; this is
//               useful for when the loader needs to know the full
//               pathname to the exact file it will be loading.
////////////////////////////////////////////////////////////////////
void LoaderFileTypeEgg::
resolve_filename(Filename &path) const {
  EggData::resolve_egg_filename(path);
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileTypeEgg::load_file
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PT_Node LoaderFileTypeEgg::
load_file(const Filename &path, bool) const {
  PT_NamedNode result = load_egg_file(path);
  return result.p();
}
