// Filename: pgCullTraverser.h
// Created by:  drose (14Mar02)
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

#ifndef PGCULLTRAVERSER_H
#define PGCULLTRAVERSER_H

#include "pandabase.h"

#include "qppgTop.h"
#include "qpcullTraverser.h"

////////////////////////////////////////////////////////////////////
//       Class : PGCullTraverser
// Description : This is a specialization of CullTraverser for use
//               within the pgui system.  It is substituted in for the
//               normal CullTraverser by the PGTop node.  Its purpose
//               is to carry additional data through the traversal so
//               that PGItems can know how to register their regions
//               with the current MouseWatcherGroup.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA PGCullTraverser : public qpCullTraverser {
public:
  INLINE PGCullTraverser(qpPGTop *top, qpCullTraverser *trav);

  qpPGTop *_top;
  int _sort_index;

public:
  static TypeHandle get_class_type() {
    return _type_handle;
  }
  static void init_type() {
    qpCullTraverser::init_type();
    register_type(_type_handle, "PGCullTraverser",
                  qpCullTraverser::get_class_type());
  }
  virtual TypeHandle get_type() const {
    return get_class_type();
  }
  virtual TypeHandle force_init_type() {init_type(); return get_class_type();}

private:
  static TypeHandle _type_handle;
};

#include "pgCullTraverser.I"

#endif


  
