// Filename: displayRegion.I
// Created by:  frang (07Mar99)
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


////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::operator <
//       Access: Public
//  Description: Returns true if this DisplayRegion should be sorted
//               before the other one, false otherwise.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegion::
operator < (const DisplayRegion &other) const {
  return get_sort() < other.get_sort();
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::is_active
//       Access: Published
//  Description: Returns the active flag associated with the
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE bool DisplayRegion::
is_active() const {
  CDReader cdata(_cycler);
  return cdata->_active;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_sort
//       Access: Published
//  Description: Returns the sort value associated with the
//               DisplayRegion.
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_sort() const {
  CDReader cdata(_cycler);
  return cdata->_sort;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_stereo_channel
//       Access: Published
//  Description: Returns whether the DisplayRegion is specified as the
//               left or right channel of a stereo pair, or whether it
//               is a normal, monocular image.  See
//               set_stereo_channel().
////////////////////////////////////////////////////////////////////
INLINE Lens::StereoChannel DisplayRegion::
get_stereo_channel() {
  CDReader cdata(_cycler);
  return cdata->_stereo_channel;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_cube_map_index
//       Access: Published
//  Description: This is a special parameter that is only used when
//               rendering the faces of a cube map.  Normally you
//               should not need to set it directly.  This sets up the
//               DisplayRegion to render to the nth cube map face; the
//               value must be between 0 and 5, inclusive.  A normal
//               DisplayRegion that is not associated with any
//               particular cube map should be set to -1.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_cube_map_index(int cube_map_index) {
  int pipeline_stage = Thread::get_current_pipeline_stage();
  nassertv(pipeline_stage == 0);
  CDWriter cdata(_cycler);
  cdata->_cube_map_index = cube_map_index;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_cube_map_index
//       Access: Published
//  Description: Returns the cube map face index associated with this
//               particular DisplayRegion, or -1 if it is not
//               associated with a cube map.  See
//               set_cube_map_index().
////////////////////////////////////////////////////////////////////
INLINE int DisplayRegion::
get_cube_map_index() const {
  CDReader cdata(_cycler);
  return cdata->_cube_map_index;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::set_cull_result
//       Access: Public
//  Description: Stores the result of performing a cull operation on
//               this DisplayRegion.  Normally, this will only be
//               called by the GraphicsEngine; you should not call
//               this directly.
//
//               The stored result will automatically be applied back
//               to all upstream pipeline stages.
////////////////////////////////////////////////////////////////////
INLINE void DisplayRegion::
set_cull_result(CullResult *cull_result, SceneSetup *scene_setup) {
  CDCullWriter cdata(_cycler_cull, true);
  cdata->_cull_result = cull_result;
  cdata->_scene_setup = scene_setup;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_cull_result
//       Access: Public
//  Description: Returns the CullResult value that was stored on this
//               DisplayRegion, presumably by the last successful cull
//               operation.  This method is for the benefit of the
//               GraphicsEngine; normally you shouldn't call this
//               directly.
////////////////////////////////////////////////////////////////////
INLINE CullResult *DisplayRegion::
get_cull_result() const {
  CDCullReader cdata(_cycler_cull);
  return cdata->_cull_result;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::get_scene_setup
//       Access: Public
//  Description: Returns the SceneSetup value that was stored on this
//               DisplayRegion, presumably by the last successful cull
//               operation.  This method is for the benefit of the
//               GraphicsEngine; normally you shouldn't call this
//               directly.
////////////////////////////////////////////////////////////////////
INLINE SceneSetup *DisplayRegion::
get_scene_setup() const {
  CDCullReader cdata(_cycler_cull);
  return cdata->_scene_setup;
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::CDataCull::Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion::CDataCull::
CDataCull() {
}

////////////////////////////////////////////////////////////////////
//     Function: DisplayRegion::CDataCull::Copy Constructor
//       Access: Public
//  Description:
////////////////////////////////////////////////////////////////////
INLINE DisplayRegion::CDataCull::
CDataCull(const DisplayRegion::CDataCull &copy) :
  _cull_result(copy._cull_result),
  _scene_setup(copy._scene_setup)
{
}

INLINE ostream &operator << (ostream &out, const DisplayRegion &dr) {
  dr.output(out);
  return out;
}
