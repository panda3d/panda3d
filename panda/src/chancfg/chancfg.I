// Filename: chancfg.I
// Created by:  cary (07Feb99)
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

  INLINE PandaNode *ChanConfig::get_group_node(const int node_index) const {
    return _group_node[node_index];
  }
  INLINE int ChanConfig::get_group_membership(const int dr_index) const {
    return _group_membership[dr_index];
  }
  INLINE int ChanConfig::get_num_groups(void) const {
    return _group_node.size();
  }
  INLINE int ChanConfig::get_num_drs(void) const {
    return _display_region.size();
  }
  INLINE PT(DisplayRegion) ChanConfig::get_dr(const int dr_index) const {
    return _display_region[dr_index];
  }
  INLINE PT(GraphicsWindow) ChanConfig::get_win(void) const {
    return _graphics_window;
  }

