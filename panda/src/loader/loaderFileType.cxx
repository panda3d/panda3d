// Filename: loaderFileType.cxx
// Created by:  drose (20Jun00)
// 
////////////////////////////////////////////////////////////////////

#include "loaderFileType.h"

TypeHandle LoaderFileType::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::Constructor
//       Access: Protected
//  Description: 
////////////////////////////////////////////////////////////////////
LoaderFileType::
LoaderFileType() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
LoaderFileType::
~LoaderFileType() {
}

////////////////////////////////////////////////////////////////////
//     Function: LoaderFileType::resolve_filename
//       Access: Public, Virtual
//  Description: Searches for the indicated filename on whatever paths
//               are appropriate to this file type, and updates it if
//               it is found.  It is not necessary to call this before
//               calling load_file(), but it doesn't hurt; this is
//               useful for when the loader needs to know the full
//               pathname to the exact file it will be loading.
////////////////////////////////////////////////////////////////////
void LoaderFileType::
resolve_filename(Filename &) const {
}
