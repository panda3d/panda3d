// Filename: mouseWatcherGroup.h
// Created by:  drose (02Jul01)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#ifndef MOUSEWATCHERGROUP_H
#define MOUSEWATCHERGROUP_H

#include "pandabase.h"
#include "mouseWatcherRegion.h"

#include "pointerTo.h"
#include "referenceCount.h"
#include "pvector.h"
#include "nodePath.h"
#include "lightMutex.h"

////////////////////////////////////////////////////////////////////
//       Class : MouseWatcherGroup
// Description : This represents a collection of MouseWatcherRegions
//               that may be managed as a group.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_TFORM MouseWatcherGroup : virtual public ReferenceCount {
public:
  MouseWatcherGroup();
  virtual ~MouseWatcherGroup();

PUBLISHED:
  void add_region(MouseWatcherRegion *region);
  bool has_region(MouseWatcherRegion *region) const;
  bool remove_region(MouseWatcherRegion *region);
  MouseWatcherRegion *find_region(const string &name) const;
  void clear_regions();

  void sort_regions();
  bool is_sorted() const;

  int get_num_regions() const;
  MouseWatcherRegion *get_region(int n) const;
  MAKE_SEQ(get_regions, get_num_regions, get_region);

  void output(ostream &out) const;
  void write(ostream &out, int indent_level = 0) const;

#ifndef NDEBUG
  void show_regions(const NodePath &render2d, 
                    const string &bin_name, int draw_order);
  void set_color(const LColor &color);
  void hide_regions();

  void update_regions();
#endif  // NDEBUG

protected:
  void do_sort_regions();
  bool do_remove_region(MouseWatcherRegion *region);

#ifndef NDEBUG
  virtual void do_show_regions(const NodePath &render2d, 
                               const string &bin_name, int draw_order);
  virtual void do_hide_regions();
  void do_update_regions();
#endif  // NDEBUG

protected:
  typedef pvector< PT(MouseWatcherRegion) > Regions;
  Regions _regions;
  bool _sorted;

  // This mutex protects the above list of regions, as well as the
  // below list of vizzes.  It is also referenced directly by
  // MouseWatcher, a derived class.
  LightMutex _lock;

private:
#ifndef NDEBUG
  PandaNode *make_viz_region(MouseWatcherRegion *region);

  typedef pvector< PT(PandaNode) > Vizzes;
  Vizzes _vizzes;

  bool _show_regions;
  NodePath _show_regions_root;
  LColor _color;
#endif  // NDEBUG

public:
  static TypeHandle get_class_type() {
    ReferenceCount::init_type();
    return _type_handle;
  }
  static void init_type() {
    register_type(_type_handle, "MouseWatcherGroup",
                  ReferenceCount::get_class_type());
  }

private:
  static TypeHandle _type_handle;

  friend class MouseWatcher;
  friend class BlobWatcher;
};

#endif
