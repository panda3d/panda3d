// Filename: webcamVideoNull.cxx
// Created by: jyelon (01Nov2007)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2007, Disney Enterprises, Inc.  All rights reserved
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
//     Function: WebcamVideo::get_num_devices
//       Access: Static, Published
//  Description: 
////////////////////////////////////////////////////////////////////
int WebcamVideo::
get_num_devices() {
  return 0;
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::get_device_name
//       Access: Static, Published
//  Description: 
////////////////////////////////////////////////////////////////////
string WebcamVideo::
get_device_name(int n) {
  return "";
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::open
//       Access: Published, Virtual
//  Description: Open this video, returning a MovieVideoCursor.
////////////////////////////////////////////////////////////////////
PT(MovieVideoCursor) WebcamVideo::
open() {
  skel_cat.error() << "WebcamVideo support is not compiled in.\n";
  return NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: WebcamVideo::init_cursor_type
//       Access: Static, Public
//  Description: Calls WebcamVideoCursor::init_type
////////////////////////////////////////////////////////////////////
void WebcamVideo::
init_cursor_type() {
}
