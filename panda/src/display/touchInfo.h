// Filename: touchInfo.h
// Created by:  Walt Destler (May 25, 2010)
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

#ifndef TOUCHINFO_H
#define TOUCHINFO_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : TouchInfo
// Description : Stores information for a single touch event.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY TouchInfo {

PUBLISHED:
  enum TouchInfoFlags
  {
    TIF_move = 0x0001,
    TIF_down = 0x0002,
    TIF_up = 0x0004,
  };

public:

  TouchInfo();

  void set_x(int x);
  void set_y(int y);
  void set_id(int id);
  void set_flags(int flags);

PUBLISHED:

  int get_x();
  int get_y();
  int get_id();
  int get_flags();

private:
  
  int _x;
  int _y;
  int _id;
  int _flags;
};

#endif
