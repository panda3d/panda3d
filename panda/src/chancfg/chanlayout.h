// Filename: chanlayout.h
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

#ifndef __CHANLAYOUT_H__
#define __CHANLAYOUT_H__

#include "pandabase.h"

#include "pmap.h"
#include "pvector.h"
#include <algorithm>
#include <string>
#include "chanviewport.h"


typedef pvector<ChanViewport> LayoutRegionVec;

class LayoutItem {
private:
  int _x, _y;
  LayoutRegionVec _regions;
public:
  INLINE LayoutItem(void);
  INLINE LayoutItem(int, int);
  INLINE LayoutItem(const LayoutItem&);
  INLINE ~LayoutItem(void);

  INLINE LayoutItem& operator=(const LayoutItem&);
  INLINE void AddRegion(const ChanViewport&);
  INLINE int GetNumRegions(void);
  INLINE const ChanViewport& operator[](int);
};

typedef pmap<std::string, LayoutItem> LayoutType;

extern LayoutType* LayoutDB;

void ResetLayout();
void ParseLayout(istream&);

#include "chanlayout.I"

#endif /* __CHANLAYOUT_H__ */
