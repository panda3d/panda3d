// Filename: guiManager.h
// Created by:  cary (25Oct00)
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
#include <set>

#include "guiLabel.h"
#include "config_gui.h"

class EXPCL_PANDA GuiManager {
private:
  typedef map<GraphicsWindow*, GuiManager*> GuiMap;
  static GuiMap* _map;
  typedef set<MouseWatcherRegion*> RegionSet;
  RegionSet _regions;
  typedef set<GuiLabel*> LabelSet;
  LabelSet _labels;
  class SortComp {
  public:
    inline bool operator()(GuiLabel* a, GuiLabel* b) const {
      return (*a) < (*b);
    }
  };
  typedef set<GuiLabel*, SortComp> SortSet;
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
