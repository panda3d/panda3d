// Filename: loader.I
// Created by:  mike (09Jan97)
//
////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////
//     Function: Loader::load_sync
//       Access: Public
//  Description: Loads the file immediately, waiting for it to complete.
////////////////////////////////////////////////////////////////////
INLINE PT_Node Loader::
load_sync(const Filename &filename) const {
  if (!_file_types_loaded) {
    load_file_types();
  }
  return load_file(filename);
}
