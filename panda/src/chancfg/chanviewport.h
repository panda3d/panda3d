// Filename: chanviewport.h
// Created by:  cary (06Feb99)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://www.panda3d.org/license.txt .
//
// To contact the maintainers of this program write to
// panda3d@yahoogroups.com .
//
////////////////////////////////////////////////////////////////////

#ifndef __CHANVIEWPORT_H__
#define __CHANVIEWPORT_H__


class EXPCL_PANDA ChanViewport {
private:
  float _left, _right, _bottom, _top;

  INLINE ChanViewport(void);
public:
  INLINE ChanViewport(float, float, float, float);
  INLINE ChanViewport(const ChanViewport&);
  INLINE ~ChanViewport(void);
  INLINE ChanViewport& operator=(const ChanViewport&);

  INLINE float left(void) const;
  INLINE float right(void) const;
  INLINE float bottom(void) const;
  INLINE float top(void) const;
};

#include <pandabase.h>

#include "chanviewport.I"

#endif /* __CHANVIEWPORT_H__ */
