// Filename: qppgTop.h
// Created by:  drose (13Mar02)
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

#ifndef qpPGTOP_H
#define qpPGTOP_H

#include "pandabase.h"

#include "pgMouseWatcherGroup.h"

#include "pandaNode.h"
#include "qpmouseWatcher.h"
#include "pointerTo.h"

class GraphicsStateGuardian;
class PGMouseWatcherGroup;

////////////////////////////////////////////////////////////////////
//       Class : PGTop
// Description : The "top" node of the new Panda GUI system.  This
//               node must be parented to the 2-d scene graph, and all
//               PG objects should be parented to this node or
//               somewhere below it.  PG objects not parented within
//               this hierarchy will not be clickable.
//
//               This node begins the special traversal of the PG
//               objects that registers each node within the
//               MouseWatcher and forces everything to render in a
//               depth-first, left-to-right order, appropriate for 2-d
//               objects.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA qpPGTop : public PandaNode {
PUBLISHED:
  qpPGTop(const string &name);
  virtual ~qpPGTop();

protected:
  INLINE qpPGTop(const qpPGTop &copy);

public:
  virtual PandaNode *make_copy() const;
  virtual bool has_cull_callback() const;
  virtual bool cull_callback(qpCullTraverser *trav, CullTraverserData &data);

PUBLISHED:
  void set_mouse_watcher(qpMouseWatcher *watcher);
  INLINE qpMouseWatcher *get_mouse_watcher() const;

public:
  // These methods duplicate the functionality of MouseWatcherGroup.
  INLINE bool add_region(MouseWatcherRegion *region);
  INLINE void clear_regions();

private:
  PT(qpMouseWatcher) _watcher;
  PGMouseWatcherGroup *_watcher_group;
  
public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    PandaNode::init_type();
    register_type(_type_handle, "qpPGTop",
                  PandaNode::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;

  friend class PGMouseWatcherGroup;
};

#include "qppgTop.I"

#endif
