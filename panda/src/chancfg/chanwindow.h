// Filename: chanwindow.h
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

#ifndef __CHANWINDOW_H__
#define __CHANWINDOW_H__

#include "pandabase.h"

#include "pmap.h"
#include "pvector.h"
#include <string>
#include "chansetup.h"
#include <pta_int.h>

class WindowItem {
private:
  bool _hw_chans, _dvr, _border, _cursor;
  int _chan_offset, _sizeX, _sizeY;
  std::string _layout;
  SetupSyms _setups;
  PTA(int) _camera_group;
public:
  INLINE WindowItem();
  INLINE WindowItem(const bool, const bool, const int, const std::string&,
                    const SetupSyms&, const int, const int, const bool,
                    const bool, PTA(int) );
  INLINE WindowItem(const WindowItem&);
  INLINE ~WindowItem();

  INLINE WindowItem& operator=(const WindowItem&);

  INLINE bool getHWChans() const;
  INLINE bool getDVR() const;
  INLINE bool getBorder() const;
  INLINE bool getCursor() const;
  INLINE int getChanOffset() const;
  INLINE int getSizeX() const;
  INLINE int getSizeY() const;
  INLINE std::string getLayout() const;
  INLINE SetupSyms getSetups() const;
  INLINE int getCameraGroup(int display_region) const;
  INLINE int getNumCameraGroups() const;
};

typedef pmap<std::string, WindowItem> WindowType;

extern WindowType* WindowDB;

void ResetWindow();
void ParseWindow(istream&);

#include "chanwindow.I"

#endif /* __CHANWINDOW_H__ */
