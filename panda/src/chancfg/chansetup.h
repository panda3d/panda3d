// Filename: chansetup.h
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

#ifndef __CHANSETUP_H__
#define __CHANSETUP_H__

#include "pandabase.h"

#include <vector_string.h>

#include "pmap.h"
#include "chanviewport.h"

typedef vector_string SetupSyms;

class SetupFOV {
public:
  enum FOVType { Invalid, Default, Horizontal, Both };
private:
  FOVType _type;
  float _horiz, _vert;
public:
  INLINE SetupFOV(void);
  INLINE SetupFOV(const SetupFOV&);
  INLINE ~SetupFOV(void);

  INLINE SetupFOV& operator=(const SetupFOV&);

  INLINE void setFOV(void);
  INLINE void setFOV(const float);
  INLINE void setFOV(const float, const float);
  INLINE FOVType getType(void) const;
  INLINE float getHoriz(void) const;
  INLINE float getVert(void) const;
};

class SetupItem {
public:
  enum Orientation { Up, Down, Left, Right };
private:
  SetupSyms _layouts;
  SetupSyms _setups;

  bool _stereo;
  bool _hw_chan;
  int _chan;
  ChanViewport _viewport;
  SetupFOV _fov;
  Orientation _orientation;
public:
  INLINE SetupItem(void);
  INLINE SetupItem(const SetupItem&);
  INLINE ~SetupItem(void);

  INLINE SetupItem& operator=(const SetupItem&);

  INLINE void setState(const bool, const bool, const int, const ChanViewport&,
                       const SetupFOV&, const Orientation&);

  INLINE SetupSyms getLayouts(void) const;
  INLINE SetupSyms getSetups(void) const;
  INLINE bool getStereo(void) const;
  INLINE bool getHWChan(void) const;
  INLINE int getChan(void) const;
  INLINE ChanViewport getViewport(void) const;
  INLINE SetupFOV getFOV(void) const;
  INLINE Orientation getOrientation(void) const;
};

typedef pmap<std::string, SetupItem> SetupType;

extern SetupType* SetupDB;

void ResetSetup();
void ParseSetup(istream&);

#include "chansetup.I"

#endif /* __CHANSETUP_H__ */
