// Filename: pgTop.h
// Created by:  drose (02Jul01)
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

#ifndef PGTOP_H
#define PGTOP_H

#include "pandabase.h"

#include "pgMouseWatcherGroup.h"

#include "namedNode.h"
#include "mouseWatcher.h"
#include "pointerTo.h"
#include "allAttributesWrapper.h"

class GraphicsStateGuardian;
class RenderTraverser;
class ArcChain;
class PGMouseWatcherGroup;

////////////////////////////////////////////////////////////////////
//       Class : PGTop
// Description : The "top" node of the new Panda GUI system.  This
//               node must be parented to the 2-d scene graph, and all
//               PG objects should be parented to this node or
//               somewhere below it.  PG objects not parented within
//               this hierarchy will not even be visible.
//
//               This node begins the special traversal of the PG
//               objects that registers each node within the
//               MouseWatcher and forces everything to render in a
//               depth-first, left-to-right order, appropriate for 2-d
//               objects.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGTop : public NamedNode {
PUBLISHED:
  PGTop(const string &name = "");
  virtual ~PGTop();

public:
  INLINE PGTop(const PGTop &copy);
  INLINE void operator = (const PGTop &copy);

  virtual Node *make_copy() const;

  virtual bool sub_render(const AllAttributesWrapper &attrib,
                          AllTransitionsWrapper &trans,
                          RenderTraverser *trav);
  virtual bool has_sub_render() const;

PUBLISHED:
  void set_mouse_watcher(MouseWatcher *watcher);
  INLINE MouseWatcher *get_mouse_watcher() const;

private:
  // These methods duplicate the functionality of MouseWatcherGroup.
  INLINE bool add_region(MouseWatcherRegion *region);
  INLINE void clear_regions();

private:
  void r_traverse(Node *node, const ArcChain &chain);

  PT(MouseWatcher) _watcher;
  PGMouseWatcherGroup *_watcher_group;
  GraphicsStateGuardian *_gsg;
  RenderTraverser *_trav;
  AllAttributesWrapper _attrib;
  int _sort_index;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    NamedNode::init_type();
    MouseWatcherGroup::init_type();
    register_type(_type_handle, "PGTop",
                  NamedNode::get_class_type(),
                  MouseWatcherGroup::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PGMouseWatcherGroup;
};

#include "pgTop.I"

#endif
