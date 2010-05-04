// Filename: textureStagePool.I
// Created by:  drose (03May10)
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


////////////////////////////////////////////////////////////////////
//     Function: TextureStagePool::get_stage
//       Access: Public, Static
//  Description: Returns a TextureStage pointer that represents the
//               same TextureStage described by temp, except that it is a
//               shared pointer.
//
//               Each call to get_stage() passing an equivalent
//               TextureStage pointer will return the same shared pointer.
//
//               If you modify the shared pointer, it will
//               automatically disassociate it from the pool.
//
//               Also, the return value may be a different pointer
//               than that passed in, or it may be the same pointer.
//               In either case, the passed in pointer has now been
//               sacrificed to the greater good and should not be used
//               again (like any other PointerTo, it will be freed
//               when the last reference count is removed).
////////////////////////////////////////////////////////////////////
INLINE TextureStage *TextureStagePool::
get_stage(TextureStage *temp) {
  return get_global_ptr()->ns_get_stage(temp);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStagePool::release_stage
//       Access: Published, Static
//  Description: Removes the indicated TextureStage from the pool.
////////////////////////////////////////////////////////////////////
INLINE void TextureStagePool::
release_stage(TextureStage *stage) {
  get_global_ptr()->ns_release_stage(stage);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStagePool::release_all_stages
//       Access: Published, Static
//  Description: Releases all TextureStages in the pool and restores the
//               pool to the empty state.
////////////////////////////////////////////////////////////////////
INLINE void TextureStagePool::
release_all_stages() {
  get_global_ptr()->ns_release_all_stages();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStagePool::set_mode
//       Access: Published, Static
//  Description: Specifies the fundamental operating mode of the
//               TextureStagePool.
//
//               If this is M_none, each call to get_stage() returns
//               the same TextureStage pointer that was passed in (the
//               pool is effectively disabled).  If this is M_name,
//               each call to get_stage() returns the last
//               TextureStage passed in with the same name, whether it
//               has different properties or not.  If this is
//               M_unique, then each call to get_stage() returns only
//               TextureStages with identical properties.
////////////////////////////////////////////////////////////////////
INLINE void TextureStagePool::
set_mode(TextureStagePool::Mode mode) {
  get_global_ptr()->ns_set_mode(mode);
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStagePool::get_mode
//       Access: Published, Static
//  Description: Returns the fundamental operating mode of the
//               TextureStagePool.  See set_mode().
////////////////////////////////////////////////////////////////////
INLINE TextureStagePool::Mode TextureStagePool::
get_mode() {
  return get_global_ptr()->ns_get_mode();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStagePool::garbage_collect
//       Access: Public, Static
//  Description: Releases only those TextureStages in the pool that have a
//               reference count of exactly 1; i.e. only those
//               TextureStages that are not being used outside of the pool.
//               Returns the number of TextureStages released.
////////////////////////////////////////////////////////////////////
INLINE int TextureStagePool::
garbage_collect() {
  return get_global_ptr()->ns_garbage_collect();
}

////////////////////////////////////////////////////////////////////
//     Function: TextureStagePool::list_contents
//       Access: Public, Static
//  Description: Lists the contents of the TextureStage pool to the
//               indicated output stream.
////////////////////////////////////////////////////////////////////
INLINE void TextureStagePool::
list_contents(ostream &out) {
  get_global_ptr()->ns_list_contents(out);
}
