// Filename: chanwindow.I
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

INLINE WindowItem::WindowItem(void) {}

INLINE WindowItem::WindowItem(const bool h, const bool d, const int o,
                              const std::string& l, const SetupSyms& s,
                              const int x, const int y, const bool b,
                              const bool c, PTA(int) n) : 
  _hw_chans(h), _dvr(d),
  _border(b), _cursor(c),
  _chan_offset(o), _sizeX(x),
  _sizeY(y), _layout(l),
  _setups(s), 
  _camera_group(n) {}

INLINE WindowItem::WindowItem(const WindowItem& c) : 
  _hw_chans(c._hw_chans),
  _dvr(c._dvr),
  _border(c._border),
  _cursor(c._cursor),
  _chan_offset(c._chan_offset),
  _sizeX(c._sizeX),
  _sizeY(c._sizeY),
  _layout(c._layout),
  _setups(c._setups),
  _camera_group(c._camera_group) {}

INLINE WindowItem::~WindowItem(void) {}

INLINE WindowItem& WindowItem::operator=(const WindowItem& c) {
  _hw_chans = c._hw_chans;
  _dvr = c._dvr;
  _border = c._border;
  _cursor = c._cursor;
  _chan_offset = c._chan_offset;
  _sizeX = c._sizeX;
  _sizeY = c._sizeY;
  _layout = c._layout;
  _setups = c._setups;
  _camera_group = c._camera_group;
  return *this;
}

INLINE bool WindowItem::getHWChans(void) const {
  return _hw_chans;
}

INLINE bool WindowItem::getDVR(void) const {
  return _dvr;
}

INLINE bool WindowItem::getBorder(void) const {
  return _border;
}

INLINE bool WindowItem::getCursor(void) const {
  return _cursor;
}

INLINE int WindowItem::getChanOffset(void) const {
  return _chan_offset;
}

INLINE int WindowItem::getSizeX(void) const {
  return _sizeX;
}

INLINE int WindowItem::getSizeY(void) const {
  return _sizeY;
}

INLINE std::string WindowItem::getLayout(void) const {
  return _layout;
}

INLINE SetupSyms WindowItem::getSetups(void) const {
  return _setups;
}

INLINE int WindowItem::getCameraGroup(int display_region) const {
  return _camera_group[display_region];
}
INLINE int WindowItem::getNumCameraGroups(void) const {
 int largestIndex=0;
 for(int j=0;j<_camera_group.size();j++)
  if(_camera_group[j]>largestIndex)largestIndex=_camera_group[j];
 return largestIndex+1;
}
