// Filename: pgButton.I
// Created by:  masad (21Oct04)
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
//     Function: PGSliderButton::is_drag_n_drop
//       Access: Published
//  Description: Returns if this button is drag_n_drop kind
////////////////////////////////////////////////////////////////////
INLINE bool PGSliderButton::
is_drag_n_drop() {
  return _drag_n_drop;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderButton::set_drag_n_drop
//       Access: Published
//  Description: Makes this button a drag_n_drop kind
////////////////////////////////////////////////////////////////////
INLINE void PGSliderButton::
set_drag_n_drop(bool value) {
  _drag_n_drop = value;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderButton::set_slider_bar
//       Access: Published
//  Description: Store a copy of the controlling slider bar item
////////////////////////////////////////////////////////////////////
INLINE void PGSliderButton::
set_slider_bar(PGItem *item) {
  _slider_bar = item;
}
