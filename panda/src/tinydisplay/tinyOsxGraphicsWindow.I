// Filename: tinyOsxGraphicsWindow.I
// Created by:  drose (12May08)
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



inline void TinyOsxGraphicsWindow::SendKeyEvent( ButtonHandle  key, bool down)
{
  if(down)
    _input_devices[0].button_down(key);
  else
    _input_devices[0].button_up(key);
}
