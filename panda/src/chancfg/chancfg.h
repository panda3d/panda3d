// Filename: chancfg.h
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

#ifndef __CHANCFG_H__
#define __CHANCFG_H__

#include "pandabase.h"

#include "chanlayout.h"
#include "chansetup.h"
#include "chanwindow.h"
#include "chanshare.h"

#include "graphicsPipe.h"
#include "graphicsWindow.h"
#include "pandaNode.h"
#include "nodePath.h"

#include "pmap.h"

class EXPCL_PANDA ChanCfgOverrides {
public:
  enum Field { OrigX, OrigY, SizeX, SizeY, Title, Mask, Cameras };
private:
  class EXPCL_PANDA Types {
  private:
    bool         _b;
    int          _i;
    unsigned int _ui;
    float        _f;
    double       _d;
    std::string  _str;
  public:
    INLINE Types(void) {}
    INLINE Types(const Types& c) : _b(c._b), _i(c._i), _ui(c._ui), _f(c._f),
                                   _d(c._d), _str(c._str) {}
    INLINE ~Types(void) {}
    INLINE Types& operator=(const Types& c) {
      _b = c._b;
      _i = c._i;
      _ui = c._ui;
      _f = c._f;
      _d = c._d;
      _str = c._str;
      return *this;
    }
    INLINE void setBool(const bool v) { _b = v; }
    INLINE bool getBool(void) const { return _b; }
    INLINE void setInt(const int v) { _i = v; }
    INLINE int getInt(void) const { return _i; }
    INLINE void setUInt(const unsigned int v) { _ui = v; }
    INLINE unsigned int getUInt(void) const { return _ui; }
    INLINE void setFloat(const float v) { _f = v; }
    INLINE float getFloat(void) const { return _f; }
    INLINE void setDouble(const double v) { _d = v; }
    INLINE double getDouble(void) const { return _d; }
    INLINE void setString(const std::string& v) { _str = v; }
    INLINE std::string getString(void) const { return _str; }
  };
  typedef pmap<Field, Types> Fields;
  Fields _fields;
public:
  ChanCfgOverrides(void);
  ChanCfgOverrides(const ChanCfgOverrides&);
  ~ChanCfgOverrides(void);

  ChanCfgOverrides& operator=(const ChanCfgOverrides&);

  void setField(const Field, const bool);
  void setField(const Field, const int);
  void setField(const Field, const unsigned int);
  void setField(const Field, const float);
  void setField(const Field, const double);
  void setField(const Field, const std::string&);
  void setField(const Field, const char*);

  bool defined(const Field) const;

  bool getBool(const Field) const;
  int getInt(const Field) const;
  unsigned int getUInt(const Field) const;
  float getFloat(const Field) const;
  double getDouble(const Field) const;
  std::string getString(const Field) const;
};

extern ChanCfgOverrides ChanOverrideNone;

typedef pvector<SetupItem> SVec;

class EXPCL_PANDA ChanConfig
{
private:
  std::vector< PT(PandaNode) > _group_node;
  std::vector< PT(DisplayRegion) > _display_region;
  std::vector<int> _group_membership;
  PT(GraphicsWindow) _graphics_window;
  void chan_eval(GraphicsWindow* win, WindowItem& W, LayoutItem& L, 
         SVec& S, ChanViewport& V, int hw_offset, 
         int xsize, int ysize, const NodePath &render, bool want_cameras);
PUBLISHED:
  ChanConfig(GraphicsPipe*, std::string, const NodePath &render,
    ChanCfgOverrides& = ChanOverrideNone);
  INLINE PandaNode *get_group_node(const int node_index) const;
  INLINE int get_group_membership(const int dr_index) const;
  INLINE int get_num_groups(void) const;
  INLINE int get_num_drs(void) const;
  INLINE PT(DisplayRegion) get_dr(const int dr_index) const;
  INLINE PT(GraphicsWindow) get_win(void) const;
};

#include "chancfg.I"

#endif /* __CHANCFG_H__ */
