// Filename: chansetup.I
// Created by:  cary (05Feb99)
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

INLINE SetupFOV::SetupFOV(void) : _type(Invalid) {}

INLINE SetupFOV::SetupFOV(const SetupFOV& c) : _type(c._type),
                                               _horiz(c._horiz),
                                               _vert(c._vert) {}

INLINE SetupFOV::~SetupFOV(void) {}

INLINE SetupFOV& SetupFOV::operator=(const SetupFOV& c) {
  _type = c._type;
  _horiz = c._horiz;
  _vert = c._vert;
  return *this;
}

INLINE void SetupFOV::setFOV(void) {
  _type = Default;
}

INLINE void SetupFOV::setFOV(const float h) {
  _type = Horizontal;
  _horiz = h;
}

INLINE void SetupFOV::setFOV(const float h, const float v) {
  _type = Both;
  _horiz = h;
  _vert = v;
}

INLINE SetupFOV::FOVType SetupFOV::getType(void) const {
  return _type;
}

INLINE float SetupFOV::getHoriz(void) const {
  return _horiz;
}

INLINE float SetupFOV::getVert(void) const {
  return _vert;
}

INLINE SetupItem::SetupItem(void) : _viewport(0., 1., 0., 1.) {}

INLINE SetupItem::SetupItem(const SetupItem& c) : _layouts(c._layouts),
                                                  _setups(c._setups),
                                                  _stereo(c._stereo),
                                                  _hw_chan(c._hw_chan),
                                                  _chan(c._chan),
                                                  _viewport(c._viewport),
                                                  _fov(c._fov),
                                                  _orientation(c._orientation)
                                                  {}

INLINE SetupItem::~SetupItem(void) {}

INLINE SetupItem& SetupItem::operator=(const SetupItem& c) {
  _layouts = c._layouts;
  _setups = c._setups;
  _stereo = c._stereo;
  _hw_chan = c._hw_chan;
  _chan = c._chan;
  _viewport = c._viewport;
  _fov = c._fov;
  _orientation = c._orientation;
  return *this;
}

INLINE void SetupItem::setState(const bool st, const bool hw, const int pvc,
                                const ChanViewport& v, const SetupFOV& f,
                                const Orientation& o) {
  _stereo = st;
  _hw_chan = hw;
  _chan = pvc;
  _viewport = v;
  _fov = f;
  _orientation = o;
}

INLINE SetupSyms SetupItem::getLayouts(void) const {
  return _layouts;
}

INLINE SetupSyms SetupItem::getSetups(void) const {
  return _setups;
}

INLINE bool SetupItem::getStereo(void) const {
  return _stereo;
}

INLINE bool SetupItem::getHWChan(void) const {
  return _hw_chan;
}

INLINE int SetupItem::getChan(void) const {
  return _chan;
}

INLINE ChanViewport SetupItem::getViewport(void) const {
  return _viewport;
}

INLINE SetupFOV SetupItem::getFOV(void) const {
  return _fov;
}

INLINE SetupItem::Orientation SetupItem::getOrientation(void) const {
  return _orientation;
}
