// Filename: pgSliderBarNotify.h
// Created by:  drose (18Aug05)
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

#ifndef PGSLIDERBARNOTIFY_H
#define PGSLIDERBARNOTIFY_H

#include "pandabase.h"
#include "pgItemNotify.h"

class PGSliderBar;

////////////////////////////////////////////////////////////////////
//       Class : PGSliderBarNotify
// Description : Objects that inherit from this class can receive
//               notify messages when a slider bar moves or otherwise
//               is reconfigured.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_PGUI PGSliderBarNotify : public PGItemNotify {
public:
  INLINE PGSliderBarNotify();

protected:
  virtual void slider_bar_adjust(PGSliderBar *slider_bar);
  virtual void slider_bar_set_range(PGSliderBar *slider_bar);

  friend class PGSliderBar;
};

#include "pgSliderBarNotify.I"

#endif
