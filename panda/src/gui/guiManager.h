// Filename: guiManager.h
// Created by:  cary (25Oct00)
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

#ifndef __GUIMANAGER_H__
#define __GUIMANAGER_H__

#include <pandabase.h>
#include <graphicsWindow.h>
#include <mouse.h>
#include <mouseWatcher.h>
#include <mouseWatcherRegion.h>
#include <node.h>
#include "pset.h"

#include "guiLabel.h"
#include "config_gui.h"

class EXPCL_PANDA GuiManager {
private:
  typedef pmap<GraphicsWindow*, GuiManager*> GuiMap;
  static GuiMap* _map;
  typedef pset<MouseWatcherRegion*> RegionSet;
  RegionSet _regions;
  typedef pset<GuiLabel*> LabelSet;
  LabelSet _labels;
  class SortComp {
  public:
    inline bool operator()(GuiLabel* a, GuiLabel* b) const {
      return (*a) < (*b);
    }
  };
  typedef pset<GuiLabel*, SortComp> SortSet;
  SortSet _sorts;

  int _start_draw_order;
  int _next_draw_order;

  Node* _root;
  MouseWatcher* _watcher;
  EventHandler* _eh;

  INLINE GuiManager(MouseWatcher*, Node*, EventHandler*);
PUBLISHED:
  static GuiManager* get_ptr(GraphicsWindow*, MouseAndKeyboard*, Node *root2d);

  void add_region(MouseWatcherRegion*);
  void add_label(GuiLabel*);
  void add_label(GuiLabel*, Node*);

  void remove_region(MouseWatcherRegion*);
  void remove_label(GuiLabel*);

  bool has_region(MouseWatcherRegion*);
  bool has_label(GuiLabel*);

  void recompute_priorities(void);

  INLINE int get_next_draw_order(void) const;
  INLINE void set_next_draw_order(int);
  INLINE int get_start_draw_order(void) const;
  INLINE void set_start_draw_order(int);

  bool is_sane(void) const;
  void sanity_check(void) const;

  INLINE EventHandler* get_private_handler(void) const;
};

#include "guiManager.I"

#endif /* __GUIMANAGER_H__ */
