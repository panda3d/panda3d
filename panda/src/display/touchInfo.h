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
	TIF_UP = 0x0004,
  };

public:

  TouchInfo();

  void set_x(LONG x);
  void set_y(LONG y);
  void set_id(DWORD id);
  void set_flags(DWORD flags);

PUBLISHED:

  LONG get_x();
  LONG get_y();
  DWORD get_id();
  DWORD get_flags();

private:
  
	LONG _x;
	LONG _y;
	DWORD _id;
	DWORD _flags;
};

#endif
