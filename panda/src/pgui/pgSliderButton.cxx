// Filename: pgSliderButton.cxx
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

#include "pgSliderButton.h"
#include "pgSliderBar.h"
#include "dcast.h"
#include "pgMouseWatcherParameter.h"

#include "throw_event.h"
#include "mouseButton.h"
#include "mouseWatcherParameter.h"
#include "colorAttrib.h"
#include "transformState.h"

TypeHandle PGSliderButton::_type_handle;

////////////////////////////////////////////////////////////////////
//     Function: PGSliderButton::Constructor
//       Access: Published
//  Description: 
////////////////////////////////////////////////////////////////////
PGSliderButton::
PGSliderButton(const string &name) : PGButton(name)
{
  _drag_n_drop = false;
  _slider_bar = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderButton::Destructor
//       Access: Public, Virtual
//  Description: 
////////////////////////////////////////////////////////////////////
PGSliderButton::
~PGSliderButton() {
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderButton::Copy Constructor
//       Access: Public
//  Description: 
////////////////////////////////////////////////////////////////////
PGSliderButton::
PGSliderButton(const PGSliderButton &copy) :
  PGButton(copy),
  _drag_n_drop(copy._drag_n_drop)
  //  _slider_bar(copy._slider_bar)
{
  _slider_bar = NULL;
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderButton::make_copy
//       Access: Public, Virtual
//  Description: Returns a newly-allocated Node that is a shallow copy
//               of this one.  It will be a different Node pointer,
//               but its internal data may or may not be shared with
//               that of the original Node.
////////////////////////////////////////////////////////////////////
PandaNode *PGSliderButton::
make_copy() const {
  return new PGSliderButton(*this);
}

////////////////////////////////////////////////////////////////////
//     Function: PGSliderButton::move
//       Access: Public, Virtual
//  Description: This is a callback hook function, called whenever the
//               button is dragged left or right  by the user normally.
////////////////////////////////////////////////////////////////////
void PGSliderButton::
move(const MouseWatcherParameter &param) {
  PGButton::move(param);
  if (_drag_n_drop && is_button_down()) {
    PGSliderBar *slider = DCAST(PGSliderBar, _slider_bar);
    slider->drag(param);
    //pgui_cat.info() << get_name() << "::move()" << endl;
  }
}
