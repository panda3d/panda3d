// Filename: chanlayout.I
// Created by:  cary (04Feb99)
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

INLINE LayoutItem::LayoutItem(void) {}

INLINE LayoutItem::LayoutItem(int x, int y) : _x(x), _y(y) {}

INLINE LayoutItem::LayoutItem(const LayoutItem& c) : _x(c._x), _y(c._y),
                                                     _regions(c._regions) {}

INLINE LayoutItem::~LayoutItem(void) {}

INLINE LayoutItem& LayoutItem::operator=(const LayoutItem& c) {
  _x = c._x;
  _y = c._y;
  _regions = c._regions;
  return *this;
}

INLINE void LayoutItem::AddRegion(const ChanViewport& r) {
  _regions.push_back(r);
}

INLINE int LayoutItem::GetNumRegions(void) {
  return _regions.size();
}

INLINE const ChanViewport& LayoutItem::operator[](int i) {
  return _regions[i];
}
