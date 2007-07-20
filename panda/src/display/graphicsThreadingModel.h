// Filename: graphicsThreadingModel.h
// Created by:  drose (27Jan03)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) 2001 - 2004, Disney Enterprises, Inc.  All rights reserved
//
// All use of this software is subject to the terms of the Panda 3d
// Software license.  You should have received a copy of this license
// along with this source code; you will also find a current copy of
// the license at http://etc.cmu.edu/panda3d/docs/license/ .
//
// To contact the maintainers of this program write to
// panda3d-general@lists.sourceforge.net .
//
////////////////////////////////////////////////////////////////////

#ifndef GRAPHICSTHREADINGMODEL_H
#define GRAPHICSTHREADINGMODEL_H

#include "pandabase.h"

////////////////////////////////////////////////////////////////////
//       Class : GraphicsThreadingModel
// Description : This represents the user's specification of how a
//               particular frame is handled by the various threads.
////////////////////////////////////////////////////////////////////
class EXPCL_PANDA_DISPLAY GraphicsThreadingModel {
PUBLISHED:
  GraphicsThreadingModel(const string &model = string());
  INLINE GraphicsThreadingModel(const GraphicsThreadingModel &copy);
  INLINE void operator = (const GraphicsThreadingModel &copy);
  
  string get_model() const;
  INLINE const string &get_cull_name() const;
  INLINE void set_cull_name(const string &cull_name);
  INLINE int get_cull_stage() const;

  INLINE const string &get_draw_name() const;
  INLINE void set_draw_name(const string &cull_name);
  INLINE int get_draw_stage() const;

  INLINE bool get_cull_sorting() const;
  INLINE void set_cull_sorting(bool cull_sorting);
 
  INLINE bool is_single_threaded() const;
  INLINE bool is_default() const;
  INLINE void output(ostream &out) const;

private:
  void update_stages();

private:
  string _cull_name;
  int _cull_stage;
  string _draw_name;
  int _draw_stage;
  bool _cull_sorting;
};

INLINE ostream &operator << (ostream &out, const GraphicsThreadingModel &threading_model);

#include "graphicsThreadingModel.I"

#endif
